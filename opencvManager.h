#pragma once

#include<qobject.h>
#include<qimage.h>
#include<qdebug.h>
#include<neoapi/neoapi.hpp>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui.hpp>
class opencvManager :public QObject
{
	Q_OBJECT;

private:
	cv::Mat image, thr;
	NeoAPI::Cam camera;
	NeoAPI::Image img;


	//double minv, maxv;
	//int idx_max[2], idx_min[2];
	const double k1 = -9.036251264315815e-14;
	const double k2 = 1.023092989802062e-11;
	const double k3 = -4.704060799332894e-06;
	const double k4 = 0.071700640472450;
	const double k5 = -62.476247850723280;
	double x;
	int width;
	int height;
	bool status;
	bool toggleStream;

	void process(cv::Mat imag);
	cv::Mat calcgrayhist(const cv::Mat& image);
	int OTSU(const cv::Mat& image, cv::Mat& OTSU_image);

public:
	explicit opencvManager(QObject* parent = 0);
	~opencvManager();


signals:
	void sendSourceFrame(QImage ima);
	void sendDisplacement(double z);

public slots:
	void receiveGrabFrame();
	void receiveSetup();
	void receiveToggleStream();


};
