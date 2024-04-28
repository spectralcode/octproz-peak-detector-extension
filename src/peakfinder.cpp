#include "peakfinder.h"
#include <QtMath>

PeakFinder::PeakFinder(QObject *parent)
	: QObject(parent),
	isFeatureExtracting(false)
{

}

void PeakFinder::setParams(PeakDetectorParameters params) {
	this->params = params;
}

void PeakFinder::findPeak(void* frameBuffer, unsigned int bitDepth, unsigned int samplesPerLine, unsigned int linesPerFrame) {
	if (!this->isFeatureExtracting) {
		this->isFeatureExtracting = true;
		int peakPosition = -1;

		QVector<qreal> averagedLine = this->calculateAveragedLine(frameBuffer, bitDepth, samplesPerLine, linesPerFrame);
		emit averagedLineCalculated(averagedLine);

		//find peak in averaged A-scan based on selected method/feature
		switch (this->params.feature) {
			case MAXVALUE:
				peakPosition = this->findMaxValuePosition(averagedLine, this->params.minThreshold);
				break;
			//todo: additional cases for other features
		}

		emit peakPositionFound(peakPosition);
		this->isFeatureExtracting = false;
	}
}

void PeakFinder::setRoi(QRect roi) {
	this->params.roi = roi;
}

void PeakFinder::setFeature(int featureOption) {
	this->params.feature = static_cast<PEAK_FEATURE>(featureOption);
}

int PeakFinder::findMaxValuePosition(const QVector<qreal>& line, double threshold) {
	if (line.isEmpty()) {
		return -1;
	}
	qreal max = line[0];
	int maxPos = -1;
	for (int i = 1; i < line.size(); i++) {
		if (line[i] > threshold && line[i] > max) {
			max = line[i];
			maxPos = i;
		}
	}

	return maxPos;
}

QRect PeakFinder::clampRoi(QRect roi, unsigned int samplesPerLine, unsigned int linesPerFrame) {
	QRect clampedRoi(0, 0, 0, 0);
	QRect normalizedRoi = roi.normalized();

	int roiX = normalizedRoi.x();
	int roiWidth = normalizedRoi.width();
	int roiY = normalizedRoi.y();
	int roiHeight = normalizedRoi.height();

	int frameWidth = static_cast<int>(samplesPerLine);
	int frameHeight = static_cast<int>(linesPerFrame);

	//check if roi is fully outside of frame and return zero sized roi
	if(roiX >= frameWidth || roiY >= frameHeight || (roiX + roiWidth) < 0 || (roiY + roiHeight) < 0){
		return clampedRoi;
	}

	//clamp roi to ensure it is fully within the frame
	int endX = (qMin(roiX + roiWidth, frameWidth));
	int endY = (qMin(roiY + roiHeight, frameHeight));
	int clampedX = qMax(0, roiX);
	int clampedY = qMax(0, roiY);
	int clampedWidth = qMin(endX-clampedX, frameWidth);
	int clampedHeight = qMin(endY-clampedY, frameHeight);
	clampedRoi.setX(clampedX);
	clampedRoi.setY(clampedY);
	clampedRoi.setWidth(clampedWidth);
	clampedRoi.setHeight(clampedHeight);

	return clampedRoi;
}

QVector<qreal> PeakFinder::calculateAveragedLine(void *frameBuffer, unsigned int bitDepth, unsigned int samplesPerLine, unsigned int linesPerFrame) {
	QVector<qreal> averagedLine;

	if (bitDepth <= 8) {
		unsigned char* frame = static_cast<unsigned char*>(frameBuffer);
		averagedLine = this->calculateAveragedLine<unsigned char>(this->params.roi, frame, samplesPerLine, linesPerFrame);
	} else if (bitDepth > 8 && bitDepth <= 16) {
		unsigned short* frame = static_cast<unsigned short*>(frameBuffer);
		averagedLine = this->calculateAveragedLine<unsigned short>(this->params.roi, frame, samplesPerLine, linesPerFrame);
	} else if (bitDepth > 16 && bitDepth <= 32) {
		unsigned long* frame = static_cast<unsigned long*>(frameBuffer);
		averagedLine = this->calculateAveragedLine<unsigned long>(this->params.roi, frame, samplesPerLine, linesPerFrame);
	}

	return averagedLine;
}

template<typename T>
QVector<qreal> PeakFinder::calculateAveragedLine(QRect roi, T* frame, unsigned int samplesPerLine, unsigned int linesPerFrame) {
	QVector<qreal> averagedLine(samplesPerLine, 0);
	QRect clampedRoi = this->clampRoi(roi, samplesPerLine, linesPerFrame);
	int roiY = clampedRoi.y();
	int roiHeight = clampedRoi.height();
	int roiX = clampedRoi.x();
	int roiWidth = clampedRoi.width();

	//if roi is out of the frame clampedRoi(..) will return a QRect with 0 width and 0 height
	if (roiWidth <= 0 || roiHeight <= 0) {
		return averagedLine;
	}

	//loop through ROI and sum up the values
	int endY = roiY + roiHeight;
	int endX = roiX + roiWidth;
	QVector<qreal> sumLine(roiWidth, 0);

	for (int y = roiY; y < endY; ++y) {
		for (int x = roiX; x < endX; ++x) {
			sumLine[x - roiX] += frame[y * samplesPerLine + x];
		}
	}

	// compute average per column in ROI
	for (int i = 0; i < roiWidth; ++i) {
		averagedLine[roiX + i] = sumLine[i] / roiHeight;
	}

	return averagedLine;
}
