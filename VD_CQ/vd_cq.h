#ifndef VD_CQ_H
#define VD_CQ_H

#include <QtGui/QMainWindow>
#include "ui_vd_cq.h"
#include<QFileDialog>
#include<QPushButton>
#include "mypainterwidget.h"
#include<QGridLayout>
#include<QVBoxLayout>
#include<QHBoxLayout>
#include<QDockWidget>
#include<QMenuBar>
#include<QMenu>
#include<QAction>
#include"funtions.h"
class VD_CQ : public QMainWindow
{
	Q_OBJECT

public:
	VD_CQ(QWidget *parent = 0, Qt::WFlags flags = 0);
	~VD_CQ();
	public slots:
		void slotOpenFileDialog();
		void slotViewOShpFile();
private:
	Ui::VD_CQClass ui;
	MyPainterWidget *myPainterW;
	QPushButton *openButton;
	QMenuBar mb;
	QAction *fileOpen;
};

#endif // VD_CQ_H
