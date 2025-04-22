#include <QtTest>
#include <QApplication>
#include "test_peakfinder.h"
#include "test_bitdepthconverter.h"

Q_DECLARE_METATYPE(uchar*)

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	qRegisterMetaType<uchar*>("uchar*");
	
	int status = 0;
	
	{
		TestPeakFinder tc;
		status |= QTest::qExec(&tc, argc, argv);
	}
	
	{
		TestBitDepthConverter tc;
		status |= QTest::qExec(&tc, argc, argv);
	}
	
	return status;
}