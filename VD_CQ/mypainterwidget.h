#ifndef MYPAINTERWIDGET_H
#define MYPAINTERWIDGET_H

#include <QWidget>
#include <QPoint>
#include<vector>
#include"funtions.h"
#include<QRectF>
using namespace std;


//�߶�
typedef struct myLine{
	QPoint startPnt;
	QPoint endPnt;
}myLine;

class MyPainterWidget: public QWidget
{
public:
	MyPainterWidget(QWidget* parent=0);
	~MyPainterWidget();

	//�̳�
	void paintEvent(QPaintEvent* p);
	void mousePressEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
	void wheelEvent(QWheelEvent * event);
	void readLevelData(int level,QRectF viewEnvelope);
	QPoint startPnt;   //���
	QPoint endPnt;     //�յ�
	bool isPressed;    //����Ƿ���

	vector<myLine*> lines; //������е��߶�
	void SetDataSaved(AllPointsRead &dataSaved);
	void Setenvelope(OGREnvelope* envelope);
	void SetFileNames(string filename,string & ovrFileName,string &  dataFileName);
	void clearDataSaved();
	bool flag;
	int m_width;//widget width
	int m_height;//widget height

	int m_levelcount;
	AllPointsRead m_dataSaved;
	OGREnvelope* m_envelope;
	QRectF m_shpEnvelop;//shp rect
	QRectF m_viewEnvelop;//widgt view
	int m_viewWidth;
	int m_viewHeight;
	int m_currentLevel;
	int m_RealLevel;
	double m_wheelScale;
	string m_filename;
	string m_dataFileName;
	string m_ovrFileName;
	bool isViewOrigin;
	
};

#endif // MYPAINTERWIDGET_H
