QT += core gui widgets testlib
TARGET = peakdetector-tests
TEMPLATE = app

# Path to plugin source
SRCDIR = $$shell_path($$PWD/../src)

INCLUDEPATH += \
	$$SRCDIR

SOURCES += \
	main.cpp \
	test_peakfinder.cpp \
	test_bitdepthconverter.cpp \
	$$SRCDIR/peakfinder.cpp \
	$$SRCDIR/bitdepthconverter.cpp

HEADERS += \
	test_peakfinder.h \
	test_bitdepthconverter.h \
	$$SRCDIR/peakfinder.h \
	$$SRCDIR/bitdepthconverter.h \
	$$SRCDIR/peakdetectorparameters.h
