#include "vd_cq.h"
#include<QFileDialog>
#include<QString>
#include<QTime>
VD_CQ::VD_CQ(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	ui.setupUi(this);
	myPainterW=new MyPainterWidget(this);
	//openButton =new QPushButton("OPEN THE FILE",this);
	QMenu *m=new QMenu("File");
	fileOpen=new QAction("open file",this);
	QAction *showOriginShp=new QAction("view Origin File",this);
	m->addAction(fileOpen);
	m->addAction(showOriginShp);
	mb.addMenu(m);
	connect(fileOpen,SIGNAL(triggered()),this,SLOT(slotOpenFileDialog()));
	connect(showOriginShp,SIGNAL(triggered()),this,SLOT(slotViewOShpFile()));

	this->setCentralWidget(myPainterW);
	this->setMenuBar(&mb);
	
}
VD_CQ::~VD_CQ()
{

}
void VD_CQ::slotOpenFileDialog(){
	QString *filter=new QString("*.shp");
	QString filename0=QFileDialog::getOpenFileName(this,QString("open the file"),NULL,NULL,filter);
	string filename=filename0.toStdString();
	if(filename=="")
		return ;
	OGREnvelope* envelope=new OGREnvelope;
	setFileEnvelope(filename0.toStdString(),envelope);
	int index=filename0.indexOf('.');
	filename0.truncate(index);
	QString exs=QString("ovr.bin");
	QString exs2=QString("data.bin");
	QString ovrfilename0=filename0+exs;
	QString datafilename0=filename0+exs2;
	bool isExist= QFile::exists(ovrfilename0);
	string ovrfilename=ovrfilename0.toStdString();
	string datafilename=datafilename0.toStdString();
	if(!isExist){
		buildDestinationFile2(filename,ovrfilename); 
		writeShptoDestination(filename,datafilename,ovrfilename);
	}


	BuildQTree(ovrfilename);
	AllPointsRead dataSaved;
	QTime time;
	time.start();
	readData(ovrfilename,datafilename,dataSaved);
	int timedata=time.elapsed();
	myPainterW->SetDataSaved(dataSaved);
	myPainterW->Setenvelope(envelope);
	myPainterW->SetFileNames(filename,ovrfilename,datafilename);
	myPainterW->isViewOrigin=false;
	myPainterW->update();
}
void VD_CQ::slotViewOShpFile(){

	QString *filter=new QString("*.shp");
	QString filename0=QFileDialog::getOpenFileName(this,QString("open the file"),NULL,NULL,filter);
	string filename=filename0.toStdString();
	if(filename=="")
		return ;
	OGREnvelope* envelope=new OGREnvelope;
	setFileEnvelope(filename0.toStdString(),envelope);

	myPainterW->SetFileNames(filename,string(""),string(""));
	QTime time;
	time.start();
	AllPointsRead dataSaved;
	ReadShpFileToMem(filename,dataSaved);
	int timedata=time.elapsed();
	myPainterW->SetDataSaved(dataSaved);
	myPainterW->Setenvelope(envelope);
	myPainterW->isViewOrigin=true;
	myPainterW->update();
}