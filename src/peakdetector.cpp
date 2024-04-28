#include "peakdetector.h"


PeakDetector::PeakDetector()
	: Extension(),
	lostBuffersProcessed(0),
	form(new PeakDetectorForm()),
	peakFinder(new PeakFinder()),
	bufferCounter(0),
	copyBufferId(-1),
	bytesPerFrameProcessed(0),
	bytesPerFrameRaw(0),
	isCalculating(false),
	active(false),
	bufferNr(0),
	nthBuffer(10),
	frameNr(0)
{
	qRegisterMetaType<PeakDetectorParameters>("PeakDetectorParameters");

	this->setType(EXTENSION);
	this->displayStyle = SEPARATE_WINDOW;
	this->name = "Peak Detector";
	this->toolTip = "Finds the peak within a ROI in a B-scan";

	this->setupGuiConnections();
	this->setupPeakFinder();
	this->initializeFrameBuffers();
}

PeakDetector::~PeakDetector() {
	peakFinderThread.quit();
	peakFinderThread.wait();

	delete this->form;

	this->releaseFrameBuffers(this->frameBuffersProcessed);
	this->releaseFrameBuffers(this->frameBuffersRaw);
}

QWidget* PeakDetector::getWidget() {
	return this->form;
}

void PeakDetector::activateExtension() {
	//this method is called by OCTproZ as soon as user activates the extension. If the extension controls hardware components, they can be prepared, activated, initialized or started here.
	this->active = true;
}

void PeakDetector::deactivateExtension() {
	//this method is called by OCTproZ as soon as user deactivates the extension. If the extension controls hardware components, they can be deactivated, resetted or stopped here.
	this->active = false;
}

void PeakDetector::settingsLoaded(QVariantMap settings) {
	//this method is called by OCTproZ and provides a QVariantMap with stored settings/parameters.
	this->form->setSettings(settings); //update gui with stored settings
}

void PeakDetector::setupGuiConnections() {
	connect(this->form, &PeakDetectorForm::info, this, &PeakDetector::info);
	connect(this->form, &PeakDetectorForm::error, this, &PeakDetector::error);
	connect(this, &PeakDetector::maxBuffers, this->form, &PeakDetectorForm::setMaximumBufferNr);
	connect(this, &PeakDetector::maxFrames, this->form, &PeakDetectorForm::setMaximumFrameNr);

	//store settings
	connect(this->form, &PeakDetectorForm::paramsChanged, this, &PeakDetector::storeParameters);

	//image display connections
	ImageDisplay* imageDisplay = this->form->getImageDisplay();
	connect(this, &PeakDetector::newFrame, imageDisplay, &ImageDisplay::receiveFrame);
	connect(imageDisplay, &ImageDisplay::roiChanged, this, [this](const QRect& rect) {
		QString rectString = QString("ROI: %1, %2, %3, %4").arg(rect.x()).arg(rect.y()).arg(rect.width()).arg(rect.height());
		emit this->info(rectString);
	});

	//data acquisition settings inputs from the GUI
	connect(this->form, &PeakDetectorForm::frameNrChanged, this, [this](int frameNr) {
		this->frameNr = frameNr;
	});
	connect(this->form, &PeakDetectorForm::bufferNrChanged, this, [this](int bufferNr) {
		this->bufferNr = bufferNr;
	});
	connect(this->form, &PeakDetectorForm::bufferSourceChanged, this, [this](BUFFER_SOURCE source) {
		this->bufferSource = source;
	});
}

void PeakDetector::setupPeakFinder() {
	this->peakFinder = new PeakFinder();
	this->peakFinder->moveToThread(&peakFinderThread);
	ImageDisplay* imageDisplay = this->form->getImageDisplay();
	connect(this, &PeakDetector::newFrame, this->peakFinder, &PeakFinder::findPeak);
	connect(imageDisplay, &ImageDisplay::roiChanged, this->peakFinder, &PeakFinder::setRoi);
	connect(this->form, &PeakDetectorForm::paramsChanged, this->peakFinder, &PeakFinder::setParams);
	connect(this->peakFinder, &PeakFinder::info, this, &PeakDetector::info);
	connect(this->peakFinder, &PeakFinder::error, this, &PeakDetector::error);
	connect(&peakFinderThread, &QThread::finished, this->peakFinder, &QObject::deleteLater);
	connect(this->peakFinder, &PeakFinder::averagedLineCalculated, this->form, &PeakDetectorForm::plotLine);
	connect(this->peakFinder, &PeakFinder::peakPositionFound, this->form, &PeakDetectorForm::plotPeakPositionIndicator);
	connect(this->peakFinder, &PeakFinder::peakPositionFound, this->form, &PeakDetectorForm::displayPeakPositionValue);
	peakFinderThread.start();
}

void PeakDetector::initializeFrameBuffers() {
	this->frameBuffersRaw.resize(NUMBER_OF_BUFFERS);
	this->frameBuffersProcessed.resize(NUMBER_OF_BUFFERS);
	for(int i = 0; i < NUMBER_OF_BUFFERS; i++){
		this->frameBuffersRaw[i] = nullptr;
		this->frameBuffersProcessed[i] = nullptr;
	}
}

void PeakDetector::releaseFrameBuffers(QVector<void *> buffers) {
	for (int i = 0; i < buffers.size(); i++) {
		if (buffers[i] != nullptr) {
			free(buffers[i]);
			buffers[i] = nullptr;
		}
	}
}

void PeakDetector::storeParameters() {
	//update settingsMap, so parameters can be reloaded into gui at next start of application
	this->form->getSettings(&this->settingsMap);
	emit storeSettings(this->name, this->settingsMap);
}

void PeakDetector::rawDataReceived(void* buffer, unsigned bitDepth, unsigned int samplesPerLine, unsigned int linesPerFrame, unsigned int framesPerBuffer, unsigned int buffersPerVolume, unsigned int currentBufferNr) {
	Q_UNUSED(buffer)
	Q_UNUSED(bitDepth)
	Q_UNUSED(samplesPerLine)
	Q_UNUSED(linesPerFrame)
	Q_UNUSED(framesPerBuffer)
	Q_UNUSED(buffersPerVolume)
	Q_UNUSED(currentBufferNr)
}

void PeakDetector::processedDataReceived(void* buffer, unsigned int bitDepth, unsigned int samplesPerLine, unsigned int linesPerFrame, unsigned int framesPerBuffer, unsigned int buffersPerVolume, unsigned int currentBufferNr) {
	if(this->active){
		if(!this->isCalculating && this->processedGrabbingAllowed){
			//check if current buffer is selected. If it is not selected discard it and do nothing (just return).
			if(this->bufferNr>static_cast<int>(buffersPerVolume-1)){this->bufferNr = static_cast<int>(buffersPerVolume-1);}
			if(!(this->bufferNr == -1 || this->bufferNr == static_cast<int>(currentBufferNr))){
				return;
			}

			//check if this is the nthBuffer
			this->bufferCounter++;
			if(this->bufferCounter < this->nthBuffer){
				return;
			}
			this->bufferCounter = 0;

			this->isCalculating = true;

			//calculate size of single frame
			size_t bytesPerSample = static_cast<size_t>(ceil(static_cast<double>(bitDepth)/8.0));
			size_t bytesPerFrame = samplesPerLine*linesPerFrame*bytesPerSample;

			//check if number of frames per buffer has changed and emit maxFrames to update gui
			if(this->framesPerBuffer != framesPerBuffer){
				emit maxFrames(framesPerBuffer-1);
				this->framesPerBuffer = framesPerBuffer;
			}
			//check if number of buffers per volume has changed and emit maxBuffers to update gui
			if(this->buffersPerVolume != buffersPerVolume){
				emit maxBuffers(buffersPerVolume-1);
				this->buffersPerVolume = buffersPerVolume;
			}

			//check if buffer size changed and allocate buffer memory
			if(this->frameBuffersProcessed[0] == nullptr || this->bytesPerFrameProcessed != bytesPerFrame){
				if(bitDepth == 0 || samplesPerLine == 0 || linesPerFrame == 0 || framesPerBuffer == 0){
					emit error(this->name + ":  " + tr("Invalid data dimensions!"));
					return;
				}
				//(re)create copy buffers
				if(this->frameBuffersProcessed[0] != nullptr){
					this->releaseFrameBuffers(this->frameBuffersProcessed);
				}
				for (int i = 0; i < this->frameBuffersProcessed.size(); i++) {
					this->frameBuffersProcessed[i] = static_cast<void*>(malloc(bytesPerFrame));
				}
				this->bytesPerFrameProcessed = bytesPerFrame;
			}

			//copy single frame of received data and emit it for further processing
			this->copyBufferId = (this->copyBufferId+1)%NUMBER_OF_BUFFERS;
			char* frameInBuffer = static_cast<char*>(buffer);
			if(this->frameNr>static_cast<int>(framesPerBuffer-1)){this->frameNr = static_cast<int>(framesPerBuffer-1);}
			memcpy(this->frameBuffersProcessed[this->copyBufferId], &(frameInBuffer[bytesPerFrame*this->frameNr]), bytesPerFrame);
			emit newFrame(this->frameBuffersProcessed[this->copyBufferId], bitDepth, samplesPerLine, linesPerFrame);

			this->isCalculating = false;
		}
		else{
			this->lostBuffersProcessed++;
			emit info(this->name + ": " + tr("Processed buffer lost. Total lost buffers: ") + QString::number(lostBuffersProcessed));
			if(this->lostBuffersProcessed>= INT_MAX){
				this->lostBuffersProcessed = 0;
				emit info(this->name + ": " + tr("Lost processed buffer counter overflow. Counter set to zero."));
			}
		}
	}
}
