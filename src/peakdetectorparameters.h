#ifndef PEAKDETECTORPARAMETERS_H
#define PEAKDETECTORPARAMETERS_H

#include <QString>
#include <QtGlobal>
#include <QMetaType>
#include <QRect>

#define PEAKDETECTOR_SOURCE "image_source"
#define PEAKDETECTOR_FEATURE "feature"
#define PEAKDETECTOR_FRAME "frame_number"
#define PEAKDETECTOR_BUFFER "buffer_number"
#define PEAKDETECTOR_ROI_X "roi_x"
#define PEAKDETECTOR_ROI_Y "roi_y"
#define PEAKDETECTOR_ROI_WIDTH "roi_width"
#define PEAKDETECTOR_ROI_HEIGHT "roi_height"
#define PEAKDETECTOR_MIN_THRESHOLD "min_threshold"
#define PEAKDETECTOR_SHOW_MIN_THRESHOLD "show_min_threshold"
#define PEAKDETECTOR_AUTOSCALING_ENABLED "auto_scaling_enabled"


enum BUFFER_SOURCE{
	RAW,
	PROCESSED
};

enum PEAK_FEATURE{
	MAXVALUE
};

struct PeakDetectorParameters {
	BUFFER_SOURCE bufferSource;
	PEAK_FEATURE feature;
	QRect roi;
	int frameNr;
	int bufferNr;
	double minThreshold;
	bool showMinThreshold;
	bool autoScalingEnabled;
};
Q_DECLARE_METATYPE(PeakDetectorParameters)


#endif //PEAKDETECTORPARAMETERS_H


