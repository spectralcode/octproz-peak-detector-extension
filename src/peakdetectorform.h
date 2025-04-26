#ifndef DPEAKDETECTORFORM_H
#define DPEAKDETECTORFORM_H

#include <QWidget>
#include <QRect>
#include "peakdetectorparameters.h"
#include "lineplot.h"
#include "imagedisplay.h"

namespace Ui {
class PeakDetectorForm;
}

class PeakDetectorForm : public QWidget
{
	Q_OBJECT

public:
	explicit PeakDetectorForm(QWidget *parent = 0);
	~PeakDetectorForm();

	void setSettings(QVariantMap settings);
	void getSettings(QVariantMap* settings);

	ImageDisplay* getImageDisplay(){return this->imageDisplay;}
	LinePlot* getLinePlot(){return this->linePlot;}

	Ui::PeakDetectorForm* ui;

protected:
	bool eventFilter(QObject* watched, QEvent* event) override;

public slots:
	void setMaximumFrameNr(int maximum);
	void setMaximumBufferNr(int maximum);
	void plotLine(QVector<qreal> line);
	void plotPeakPositionIndicator(int pos);
	void displayPeakPositionValue(int pos);
	void displayMinThreshold(double value);
	void enableAutoScalingLinePlot(bool autoScaleEnabled);

private:
	ImageDisplay* imageDisplay;
	LinePlot* linePlot;
	PeakDetectorParameters parameters;
	bool firstRun;

signals:
	void paramsChanged(PeakDetectorParameters);
	void frameNrChanged(int);
	void bufferNrChanged(int);
	void featureChanged(int);
	void bufferSourceChanged(BUFFER_SOURCE);
	void roiChanged(QRect);
	void minThresholdChanged(double);
	void info(QString);
	void error(QString);
};

#endif //DPEAKDETECTORFORM_H
