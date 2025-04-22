#ifndef TEST_PEAKFINDER_H
#define TEST_PEAKFINDER_H

#include <QtTest>
#include "peakfinder.h"

class TestPeakFinder : public QObject
{
	Q_OBJECT

private slots:
	void testFindMaxValuePosition();
	void testEmptyInput();
	void testThreshold();
};

#endif // TEST_PEAKFINDER_H