#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_baumer_22_3_8.h"
#include "opencvManager.h"
#include<qobject.h>

#include <QLabel>
#include <QThread>
#include <QList>
#include <QSplineSeries>
#include <QChart>
#include <QChartView>
#include <QTime>


//static int fps = 0;
//static QTime lastTime; // ms
//static int frameCount = 0;


class baumer_22_3_8 : public QMainWindow
{
	Q_OBJECT

public:
	baumer_22_3_8(QWidget* parent = Q_NULLPTR);
	~baumer_22_3_8();
	void initDraw();


private:
	Ui::baumer_22_3_8Class* ui;
	QThread* thread;

	//void dataReceived(double value);
	const qint32 maxSize = 51;  //data最多存储maxSize个元素
	const qint32 maxX = 50;
	const qint32 maxY = 100;
	qint32 pointCount;
	qint32 fps = 0;
	QTime lastTime; // ms
	qint32 frameCount = 0;

	QList<double> data;   //存储业务数据的LIST
	QChart* chart;
	QChartView* charView;
	QSplineSeries* splineSeries;

	void setUpManager();

	void dataReceived(double val);


signals:
	void sendSetup();
	void sendToggleStream();


private slots:
	void receiveCam();
	void receiveSourceFrame(QImage frame);
	void receiveToggleStream();
	void receiveDisplacement(double value);
	void receiveFPS(int x);


};
