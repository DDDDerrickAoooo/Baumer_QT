#include "opencvManager.h"
#include<opencv2/imgproc.hpp>
#include <QtCore/qmath.h>

using namespace cv;

opencvManager::opencvManager(QObject* parent) :QObject(parent), status(false), toggleStream(false)
{
	camera = NeoAPI::Cam();
}

opencvManager::~opencvManager()
{
	image.release();
	thr.release();
	camera.ClearImages();
}



void opencvManager::receiveGrabFrame()   //获取抓取的图像，并处理
{
	if (!toggleStream || !status)
	{
		return;
	}
	camera.f().TriggerSoftware.Execute();
	img = camera.GetImage();
	Mat image(cv::Size(width, height), CV_8UC1, img.GetImageData(), Mat::AUTO_STEP);
	/*minMaxIdx(image, &minv, &maxv, idx_min, idx_max);
	if (idx_max[1] < 150 || idx_max[0] < 150 || idx_max[1]>1350 || idx_max[0]>1100) return;
	Mat roi(image, Rect(idx_max[1] - 150, idx_max[0] - 150, 400, 400));*/

	process(image);//处理图像

	//Mat转QImage
	QImage outputSource((const unsigned char*)image.data, image.cols, image.rows, QImage::Format_Grayscale8);
	/*outputSource.setColorCount(256);
	for (int i = 0; i < 256; i++) {
		outputSource.setColor(i, qRgb(i, i, i));
	}
	uchar* pSrc = image.data;
	for (int row = 0; row < image.rows; row++) {
		uchar* pDest = outputSource.scanLine(row);
		memcpy(pDest, pSrc, image.cols);
		pSrc += image.step;
	}*/

	emit sendSourceFrame(outputSource);
}


void opencvManager::receiveSetup()
{
	try
	{
		camera.Connect();
		if (!camera.IsConnected())
		{
			status = false;
			return;
		}
		else
		{
			camera.f().TriggerMode = NeoAPI::TriggerMode::On;
			auto fs = NeoAPI::FeatureStack();
			fs.Add("ExposureTime", 1000);      // add features to the stack
			fs.Add("Width", 1920);
			fs.Add("Height", 304);
			fs.Add("OffsetX", 0);
			fs.Add("OffsetY", 0);
			camera.WriteFeatureStack(fs);
			width = static_cast<int>(camera.f().Width);
			height = static_cast<int>(camera.f().Height);
			/*camera.f().BinningVerticalMode = NeoAPI::BinningVerticalMode::Average;
			camera.f().BinningVertical = 2;*/
			camera.SetImageBufferCount(50);
			camera.SetImageBufferCycleCount(50);
			status = true;
		}

	}
	catch (NeoAPI::NeoException& exc) {
		qDebug() << "error: " << exc.GetDescription();
		return;
	}
	catch (...) {
		qDebug() << "oops, error";
		return;
	}
}

void opencvManager::process(Mat imag)
{
	//threshold(imag, thr, 100, 255, THRESH_BINARY);
	//adaptiveThreshold(imag, thr, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 25, 10);
	OTSU(imag, thr);
	//threshold(image, thr, yuzhi, 255, THRESH_BINARY);
	Moments m = moments(thr, true);
	//x = m.m10 / m.m00 + idx_max[1] - 151;
	x = m.m10 / m.m00;
	x = k1 * qPow(x, 4) + k2 * qPow(x, 3) + k3 * qPow(x, 2) + k4 * x + k5;
	emit sendDisplacement(x);
}


void opencvManager::receiveToggleStream()
{
	toggleStream = !toggleStream;
}



Mat opencvManager::calcgrayhist(const Mat& image)
{
	Mat histogram = Mat::zeros(Size(256, 1), CV_32SC1);
	//图像宽高
	int rows = image.rows;
	int cols = image.cols;
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < rows; j++)
		{
			int index = int(image.at<uchar>(i, j));
			histogram.at<int>(0, index) += 1;
		}
	}
	return histogram;
};


int opencvManager::OTSU(const Mat& image, Mat& OTSU_image)
{
	int rows = image.rows;
	int cols = image.cols;
	Mat histogram = calcgrayhist(image);
	//归一化直方图
	Mat normhist;
	histogram.convertTo(normhist, CV_32FC1, 1.0 / (rows * cols), 0.0);
	//计算累加直方图和一阶累积矩
	Mat zeroaccumulate = Mat::zeros(Size(256, 1), CV_32FC1);
	Mat oneaccumulate = Mat::zeros(Size(256, 1), CV_32FC1);
	for (int i = 0; i < 256; i++)
	{
		if (i == 0)
		{
			zeroaccumulate.at<float>(0, i) = normhist.at<float>(0, i);
			oneaccumulate.at<float>(0, i) = i * normhist.at<float>(0, i);
		}
		else
		{
			zeroaccumulate.at<float>(0, i) = zeroaccumulate.at<float>(0, i - 1)
				+ normhist.at<float>(0, i);
			oneaccumulate.at<float>(0, i) = oneaccumulate.at<float>(0, i - 1)
				+ i * normhist.at<float>(0, i);
		}
	}
	//计算间类方差
	Mat variance = Mat::zeros(Size(256, 1), CV_32FC1);
	float mean = oneaccumulate.at<float>(0, 255);
	for (int i = 0; i < 255; i++)
	{
		if (zeroaccumulate.at<float>(0, i) == 0 || zeroaccumulate.at<float>(0, i) == 1)
			variance.at<float>(0, i) = 0;
		else
		{
			float cofficient = zeroaccumulate.at<float>(0, i) * (1.0 -
				zeroaccumulate.at<float>(0, i));
			variance.at<float>(0, i) = pow(mean * zeroaccumulate.at<float>(0, i)
				- oneaccumulate.at<float>(0, i), 2.0) / cofficient;
		}
	}
	//找到阈值；
	Point maxloc;
	//计算矩阵中最大值
	minMaxLoc(variance, NULL, NULL, NULL, &maxloc);
	int otsuthreshold = maxloc.x;
	threshold(image, OTSU_image, otsuthreshold, 255, THRESH_BINARY);
	return otsuthreshold;
};
