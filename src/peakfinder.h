#ifndef PEAKFINDER_H
#define PEAKFINDER_H

#include <QObject>
#include <QVector>
#include <QRect>
#include <QApplication>
#include <QtMath>
#include "peakdetectorparameters.h"


class PeakFinder : public QObject
{
	Q_OBJECT
public:
	explicit PeakFinder(QObject *parent = nullptr);

private:
	bool isFeatureExtracting;
	PeakDetectorParameters params;

	int findMaxValuePosition(const QVector<qreal>& line, double threshold);
	QRect clampRoi(QRect roi, unsigned int samplesPerLine, unsigned int linesPerFrame);
	QVector<qreal> calculateAveragedLine(void* frameBuffer, unsigned int bitDepth, unsigned int samplesPerLine, unsigned int linesPerFrame);
	template <typename T> QVector<qreal> calculateAveragedLine(QRect roi, T* frame, unsigned int samplesPerLine, unsigned int linesPerFrame);


signals:
	void averagedLineCalculated(QVector<qreal>);
	void peakPositionFound(int);
	void info(QString);
	void error(QString);

public slots:
	void findPeak(void* frameBuffer, unsigned int bitDepth, unsigned int samplesPerLine, unsigned int linesPerFrame);
	void setRoi(QRect roi);
	void setFeature(int featureOption);
	void setParams(PeakDetectorParameters params);
};

#endif //PEAKFINDER
