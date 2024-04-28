#include "peakdetectorform.h"
#include "ui_peakdetectorform.h"

PeakDetectorForm::PeakDetectorForm(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::PeakDetectorForm) {
	ui->setupUi(this);

	this->firstRun = true;

	this->imageDisplay = this->ui->widget_imageDisplay;
	connect(this->imageDisplay, &ImageDisplay::info, this, &PeakDetectorForm::info);
	connect(this->imageDisplay, &ImageDisplay::error, this, &PeakDetectorForm::error);
	connect(this->imageDisplay, QOverload<QRect>::of(&ImageDisplay::roiChanged), this, [this](QRect roiRect) {
		this->parameters.roi = roiRect;
		emit roiChanged(roiRect);
		emit paramsChanged(this->parameters);
	});

	this->linePlot = this->ui->widget_linePlot;
	connect(this->linePlot, &LinePlot::info, this, &PeakDetectorForm::info);
	connect(this->linePlot, &LinePlot::error, this, &PeakDetectorForm::error);


	//SpinBox Buffer
	this->ui->spinBox_buffer->setMaximum(2);
	this->ui->spinBox_buffer->setMinimum(-1);
	this->ui->spinBox_buffer->setSpecialValueText(tr("All"));
	connect(this->ui->spinBox_buffer, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int bufferNr) {
		this->parameters.bufferNr = bufferNr;
		emit bufferNrChanged(bufferNr);
		emit paramsChanged(this->parameters);
	});

	//Frame slider and spinBox
	connect(this->ui->horizontalSlider_frame, &QSlider::valueChanged, this->ui->spinBox_frame, &QSpinBox::setValue);
	connect(this->ui->spinBox_frame, QOverload<int>::of(&QSpinBox::valueChanged), this->ui->horizontalSlider_frame, &QSlider::setValue);
	connect(this->ui->horizontalSlider_frame, &QSlider::valueChanged, this, [this](int frameNr) {
		this->parameters.frameNr = frameNr;
		emit frameNrChanged(frameNr);
		emit paramsChanged(this->parameters);
	});
	this->setMaximumFrameNr(512);	

	//DoubleSpinBox minimal threshold
	this->ui->doubleSpinBox_minThreshold->setMaximum(qPow(2, 32));
	this->ui->doubleSpinBox_minThreshold->setMinimum(0);

	connect(this->ui->doubleSpinBox_minThreshold, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double minThreshold) {
		this->parameters.minThreshold = minThreshold;
		emit minThresholdChanged(minThreshold);
		emit paramsChanged(this->parameters);
	});

	connect(this->ui->doubleSpinBox_minThreshold, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double minThreshold) {
		this->parameters.minThreshold = minThreshold;
		this->displayMinThreshold(this->parameters.minThreshold);
		emit minThresholdChanged(minThreshold);
		emit paramsChanged(this->parameters);
	});

	connect(this->ui->checkBox_showMinThreshold, &QCheckBox::stateChanged, this, [this](int state) {
		if (state == Qt::Checked) {
			this->parameters.showMinThreshold = true;
		} else {
			this->parameters.showMinThreshold = false;
		}
		this->displayMinThreshold(this->parameters.minThreshold);
		emit paramsChanged(this->parameters);
	});

	connect(this->ui->checkBox_autoscaling, &QCheckBox::stateChanged, this, [this](int state) {
		if (state == Qt::Checked) {
			this->parameters.autoScalingEnabled = true;
		} else {
			this->parameters.autoScalingEnabled = false;
		}
		this->enableAutoScalingLinePlot(this->parameters.autoScalingEnabled);
		emit paramsChanged(this->parameters);
	});


	//default values
	this->parameters.bufferNr= -1;
	this->parameters.bufferSource = PROCESSED;
	this->parameters.frameNr = 0;
	this->parameters.feature = MAXVALUE;
	this->parameters.roi = QRect(50,50, 400, 800);
	this->parameters.minThreshold = 0;
	this->parameters.showMinThreshold = false;
	this->parameters.autoScalingEnabled = true;
}

PeakDetectorForm::~PeakDetectorForm() {
	delete ui;
}

void PeakDetectorForm::setSettings(QVariantMap settings) {
	// Update parameters struct
	if (!settings.isEmpty()) {
		this->parameters.bufferNr = settings.value(PEAKDETECTOR_BUFFER).toInt();
		this->parameters.bufferSource = static_cast<BUFFER_SOURCE>(settings.value(PEAKDETECTOR_SOURCE).toInt());
		this->parameters.feature = static_cast<PEAK_FEATURE>(settings.value(PEAKDETECTOR_FEATURE).toInt());
		this->parameters.frameNr = settings.value(PEAKDETECTOR_FRAME).toInt();
		int roiX = settings.value(PEAKDETECTOR_ROI_X).toInt();
		int roiY = settings.value(PEAKDETECTOR_ROI_Y).toInt();
		int roiWidth = settings.value(PEAKDETECTOR_ROI_WIDTH).toInt();
		int roiHeight = settings.value(PEAKDETECTOR_ROI_HEIGHT).toInt();
		this->parameters.roi = QRect(roiX, roiY, roiWidth, roiHeight);
		this->parameters.minThreshold = settings.value(PEAKDETECTOR_MIN_THRESHOLD).toDouble();
		this->parameters.showMinThreshold = settings.value(PEAKDETECTOR_SHOW_MIN_THRESHOLD).toBool();
		this->parameters.autoScalingEnabled = settings.value(PEAKDETECTOR_AUTOSCALING_ENABLED).toBool();
	}

	// Update GUI elements
	this->ui->spinBox_buffer->setValue(this->parameters.bufferNr);
	//this->ui->comboBox_imageFeature->setCurrentIndex(static_cast<int>(this->parameters.imageFeature));
	this->ui->horizontalSlider_frame->setValue(this->parameters.frameNr);
	this->ui->widget_imageDisplay->setRoi(this->parameters.roi);
	this->ui->doubleSpinBox_minThreshold->setValue(this->parameters.minThreshold);
	this->ui->checkBox_showMinThreshold->setChecked(this->parameters.showMinThreshold);
	this->ui->checkBox_autoscaling->setChecked(this->parameters.autoScalingEnabled);
	this->enableAutoScalingLinePlot(this->parameters.autoScalingEnabled);
}

void PeakDetectorForm::getSettings(QVariantMap* settings) {
	if (!settings) {
		return;
	}

	settings->insert(PEAKDETECTOR_BUFFER, this->parameters.bufferNr);
	settings->insert(PEAKDETECTOR_SOURCE, static_cast<int>(this->parameters.bufferSource));
	settings->insert(PEAKDETECTOR_FEATURE, static_cast<int>(this->parameters.feature));
	settings->insert(PEAKDETECTOR_FRAME, this->parameters.frameNr);
	settings->insert(PEAKDETECTOR_ROI_X, this->parameters.roi.x());
	settings->insert(PEAKDETECTOR_ROI_Y, this->parameters.roi.y());
	settings->insert(PEAKDETECTOR_ROI_WIDTH, this->parameters.roi.width());
	settings->insert(PEAKDETECTOR_ROI_HEIGHT, this->parameters.roi.height());
	settings->insert(PEAKDETECTOR_MIN_THRESHOLD, this->parameters.minThreshold);
	settings->insert(PEAKDETECTOR_SHOW_MIN_THRESHOLD, this->parameters.showMinThreshold);
	settings->insert(PEAKDETECTOR_AUTOSCALING_ENABLED, this->parameters.autoScalingEnabled);
}

void PeakDetectorForm::setMaximumFrameNr(int maximum) {
	this->ui->horizontalSlider_frame->setMaximum(maximum);
	this->ui->spinBox_frame->setMaximum(maximum);
}

void PeakDetectorForm::setMaximumBufferNr(int maximum) {
	this->ui->spinBox_buffer->setMaximum(maximum);
}

void PeakDetectorForm::plotLine(QVector<qreal> line) {
	if(this->firstRun){
		this->enableAutoScalingLinePlot(true);
		this->linePlot->plotLine(line);
		this->enableAutoScalingLinePlot(this->parameters.autoScalingEnabled);
		this->firstRun = false;
		return;
	}
	this->linePlot->plotLine(line);
}

void PeakDetectorForm::plotPeakPositionIndicator(int pos){
	if(pos < 0){
		this->linePlot->setVerticalLineVisible(false);
	} else {
		this->linePlot->setVerticalLineVisible(true);
		this->linePlot->setVerticalLine(pos);
	}
}

void PeakDetectorForm::displayPeakPositionValue(int pos) {
	if(pos < 0){
		this->ui->lineEdit_peakPosition->setText(tr("No peak detected"));
	} else {
		this->ui->lineEdit_peakPosition->setText(QString::number(pos));
	}
}

void PeakDetectorForm::displayMinThreshold(double value) {
	if(this->parameters.showMinThreshold){
		this->linePlot->setHorizontalLineVisible(true);
		this->linePlot->setHorizontalLine(value);
	} else {
		this->linePlot->setHorizontalLineVisible(false);
	}
}

void PeakDetectorForm::enableAutoScalingLinePlot(bool autoScaleEnabled) {
	this->linePlot->enableAutoScaling(autoScaleEnabled);
}

