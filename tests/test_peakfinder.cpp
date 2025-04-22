#include "test_peakfinder.h"

void TestPeakFinder::testFindMaxValuePosition()
{
	PeakFinder peakFinder;
	
	PeakDetectorParameters params;
	params.feature = MAXVALUE;
	params.minThreshold = 0.0;
	params.roi = QRect(0, 0, 5, 1);
	params.showMinThreshold = false;
	params.autoScalingEnabled = true;
	peakFinder.setParams(params);
	
	//connect to signal to catch the result
	QSignalSpy spy(&peakFinder, &PeakFinder::peakPositionFound);
	
	unsigned char frame[5] = {1, 3, 5, 2, 4};
	
	peakFinder.findPeak(frame, 8, 5, 1);
	
	qDebug() << "Signal count:" << spy.count();
	if (spy.count() > 0) {
		qDebug() << "Peak position:" << spy.at(0).at(0).toInt();
	}
	
	//check if signal was emitted
	QCOMPARE(spy.count(), 1);
	
	//verify the peak position (should be 2, where value is 5)
	int peakPos = spy.at(0).at(0).toInt();
	QCOMPARE(peakPos, 2);
}

void TestPeakFinder::testEmptyInput()
{
	PeakFinder peakFinder;
	
	PeakDetectorParameters params;
	params.feature = MAXVALUE;
	params.minThreshold = 0.0;
	params.roi = QRect(10, 10, 5, 5); //ROI outside of frame
	params.showMinThreshold = false;
	params.autoScalingEnabled = true;
	peakFinder.setParams(params);
	
	//connect to signal to catch the result
	QSignalSpy spy(&peakFinder, &PeakFinder::peakPositionFound);
	
	//create a simple "frame" - 8-bit test data
	unsigned char frame[5] = {1, 3, 5, 2, 4};
	
	qDebug() << "Testing empty ROI with frame size: 5x1";
	qDebug() << "ROI set to:" << params.roi;
	
	peakFinder.findPeak(frame, 8, 5, 1);
	
	qDebug() << "Signal count:" << spy.count();
	if (spy.count() > 0) {
		qDebug() << "Peak position:" << spy.at(0).at(0).toInt();
	}
	
	QCOMPARE(spy.count(), 1);
	
	//verify no peak was found (-1)
	int peakPos = spy.at(0).at(0).toInt();
	QCOMPARE(peakPos, -1);
}

void TestPeakFinder::testThreshold()
{
	PeakFinder peakFinder;
	
	PeakDetectorParameters params;
	params.feature = MAXVALUE;
	params.minThreshold = 4;
	params.roi = QRect(0, 0, 5, 1);
	params.showMinThreshold = false;
	params.autoScalingEnabled = true;
	peakFinder.setParams(params);
	
	//connect to signal to catch the result
	QSignalSpy spy(&peakFinder, &PeakFinder::peakPositionFound);
	
	//create a simple "frame" - 8-bit test data
	unsigned char frame[5] = {1, 3, 5, 2, 4};
	
	qDebug() << "Testing threshold with values:" << frame[0] << frame[1] << frame[2] << frame[3] << frame[4];
	qDebug() << "Threshold set to:" << params.minThreshold;
	
	peakFinder.findPeak(frame, 8, 5, 1);
	
	qDebug() << "Signal count:" << spy.count();
	if (spy.count() > 0) {
		qDebug() << "Peak position:" << spy.at(0).at(0).toInt();
	}
	
	//check if signal was emitted
	QCOMPARE(spy.count(), 1);
	
	//verify only the peak >= threshold was found (should be position 2, value 5)
	int peakPos = spy.at(0).at(0).toInt();
	QCOMPARE(peakPos, 2);
}