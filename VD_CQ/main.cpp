#include "vd_cq.h"
#include <QtGui/QApplication>
#include <QApplication>
#include "mypainterwidget.h"

int main(int argc,char** argv)
{
	QApplication a(argc,argv);
	//QRectF qf;
	//int sizeqp=sizeof(qf);

	VD_CQ w(0);
	w.show();
	return a.exec();
}
