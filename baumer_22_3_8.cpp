
#pragma execution_character_set("utf-8")
#include "baumer_22_3_8.h"
#include <QTimer>


using namespace cv;


baumer_22_3_8::baumer_22_3_8(QWidget* parent)
	: QMainWindow(parent)
	, ui(new Ui::baumer_22_3_8Class)
{
	ui->setupUi(this);
	thread = new QThread();
	fps = 0;
	frameCount = 0;
	setUpManager();
}


baumer_22_3_8::~baumer_22_3_8()
{
	thread->quit();
	while (!thread->isFinished());
	delete thread;
	delete ui;
}


void baumer_22_3_8::setUpManager()
{
	opencvManager* cvManager = new opencvManager();
	initDraw();
	QTimer* trigger = new QTimer();
	trigger->setInterval(20);


	connect(trigger, SIGNAL(timeout()), cvManager, SLOT(receiveGrabFrame()));
	connect(cvManager, SIGNAL(sendDisplacement(double)), this, SLOT(receiveDisplacement(double)));
	connect(ui->actionCam, SIGNAL(triggered()), this, SLOT(receiveCam()));
	connect(this, SIGNAL(sendSetup()), cvManager, SLOT(receiveSetup()));
	connect(cvManager, SIGNAL(sendSourceFrame(QImage)), this, SLOT(receiveSourceFrame(QImage)));
	connect(ui->playButton, SIGNAL(clicked(bool)), this, SLOT(receiveToggleStream()));
	connect(this, SIGNAL(sendToggleStream()), cvManager, SLOT(receiveToggleStream()));
	connect(thread, SIGNAL(finished()), cvManager, SLOT(deleteLater()));
	connect(thread, SIGNAL(finished()), trigger, SLOT(deleteLater()));

	trigger->start();
	cvManager->moveToThread(thread);
	trigger->moveToThread(thread);
	thread->start();
}




void baumer_22_3_8::receiveCam()
{
	emit sendSetup();
}



void baumer_22_3_8::receiveSourceFrame(QImage frame)
{
	ui->videoFieldSource->setPixmap(QPixmap::fromImage(frame));
	ui->videoFieldSource->setScaledContents(true);
	ui->videoFieldSource->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

	++frameCount;
	QTime curTime = QTime::currentTime();
	if (lastTime.msecsTo(curTime) > 1000) // 取固定时间间隔为1秒
	{
		fps = frameCount;
		frameCount = 0;
		lastTime = curTime;
		emit receiveFPS(fps);
	}
}


void baumer_22_3_8::receiveToggleStream()
{
	if (!ui->playButton->text().compare("开始采集")) ui->playButton->setText("停止采集");//Play的话，变Pause
	else ui->playButton->setText("开始采集");//否则，变Play
	lastTime = QTime::currentTime(); // ms
	emit sendToggleStream();
}


void baumer_22_3_8::receiveDisplacement(double value)
{
	dataReceived(value);   //该过程约10ms~14ms
	ui->julixianshi->setText(QString::number(value, 'f', 3));
}


void baumer_22_3_8::initDraw()
{
	splineSeries = new QSplineSeries();

	chart = new QChart();
	chart->addSeries(splineSeries);
	splineSeries->setUseOpenGL(true);
	chart->legend()->hide();
	chart->setTitle("实时曲线");
	chart->createDefaultAxes();
	chart->axisX()->setRange(0, maxX);
	chart->axisY()->setRange(-20, 20);

	ui->graphicsView->setChart(chart);
	ui->graphicsView->setRenderHint(QPainter::Antialiasing);  //设置抗锯齿
}


void baumer_22_3_8::dataReceived(double val)
{
	data << val;

	// 数据个数超过了最大数量，则删除最先接收到的数据，实现曲线向前移动
	while (data.size() > maxSize) {
		data.removeFirst();
	}

	// 界面被隐藏后就没有必要绘制数据的曲线了
	if (isVisible()) {
		splineSeries->clear();
		//scatterSeries->clear();
		int dx = maxX / (maxSize - 1);
		int less = maxSize - data.size();

		for (int i = 0; i < data.size(); ++i) {
			splineSeries->append(less * dx + i * dx, data.at(i));
			//scatterSeries->append(less*dx+i*dx, data.at(i));
		}
	}
}


void baumer_22_3_8::receiveFPS(int x)
{
	ui->FPS->setText(QString::number(x, 3));
}
