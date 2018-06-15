#include "mypainterwidget.h"
#include <QString>
#include <QMessageBox>
#include <QPainter>
#include <QPen>
#include <QMouseEvent>
#include<QTime>

MyPainterWidget::MyPainterWidget(QWidget* parent)
	:QWidget(parent){
		//setMinimumSize(240,120);
		//setMaximumSize(480,240);
		m_width=0;
		m_height=0;
		//m_width=QWidget::width();
		//m_height=QWidget::height();
		//this->setMouseTracking(true);
		this->isPressed = false;
		//this->setWindowTitle("open the file");
		this->m_envelope=NULL;
		flag=false;
		m_viewWidth=m_width;
		m_viewHeight=m_height;
		m_wheelScale=0.5;
		isViewOrigin=false;
}
void MyPainterWidget::Setenvelope(OGREnvelope* envelope){
	m_envelope=envelope;
	QRectF qf(envelope->MinX,envelope->MinY,envelope->MaxX-envelope->MinX,envelope->MaxY-envelope->MinY);
	m_shpEnvelop=qf;
	m_viewEnvelop=m_shpEnvelop;
	m_levelcount=QtreeMap.size()-1;
	m_currentLevel=0;
	m_RealLevel=0;

}
void MyPainterWidget::SetFileNames(string filename,string & ovrFileName,string &  dataFileName){
	m_filename=filename;
	m_ovrFileName=ovrFileName;
	m_dataFileName=dataFileName;
}
MyPainterWidget::~MyPainterWidget(){

}
void MyPainterWidget::clearDataSaved(){
	AprIt aprItBeg= m_dataSaved.begin();
	AprIt aprItEnd= m_dataSaved.end();
	for(;aprItBeg!=aprItEnd;aprItBeg++){
		Ppoints Pp=aprItBeg->second;
		PpointsIt pIt=Pp.begin();
		for(;pIt!=Pp.end();pIt++)
		{
			pIt->second.clear();
		}
		aprItBeg->second.clear();

	}
	m_dataSaved.clear();
}


//void MyPainterWidget::paintEvent(QPaintEvent*p){
//
//	if(!m_envelope)
//		return ;
//	
//	int width=QWidget::width();
//	int height=QWidget::height();
//	//m_width=width;
//	//m_height=height;
//	QPainter painter(this);
//	QPen pen;                                 //创建一个画笔
//	pen.setColor(Qt::darkCyan);
//	pen.setWidth(1);
//	painter.setPen(pen);
//	double xscal=(m_envelope->MaxX-m_envelope->MinX)/width;
//	double yscal=(m_envelope->MaxY-m_envelope->MinY)/height;
//	AprIt aprItBeg= m_dataSaved.begin();
//	AprIt aprItEnd= m_dataSaved.end();
//	for(;aprItBeg!=aprItEnd;aprItBeg++){
//		Ppoints Pp=aprItBeg->second;
//		PpointsIt pIt=Pp.begin();
//		for(;pIt!=Pp.end();)
//		{
//			QVector<QPointF>& dl=pIt->second;
//
//			int pcount=dl.size();
//			if(pcount==0){
//				pIt++;
//				continue;
//			}
//			QPointF * ptDraw=new QPointF[pcount];
//			if(!(m_width==width&&m_height==height)){
//				for(int i=0;i<pcount;i++){
//					dl[i].setX((dl[i].x()-m_envelope->MinX)/xscal);
//					dl[i].setY((dl[i].y()-m_envelope->MinY)/yscal);
//					ptDraw[i]=dl[i];
//				}
//				//m_width=width;
//				//m_height=height;
//			}
//			//painter.drawLines(pIt->second);
//			painter.drawPolyline(ptDraw,pcount-1);
//			int befF=pIt->first;
//			QPointF lp=pIt->second.last();
//			pIt++;
//			//if(pIt!=Pp.end()&&pIt->first==(befF+1)){
//			//	painter.drawLine(lp,pIt->second.first());
//			//}
//		}
//		
//		
//	}
//}


void MyPainterWidget::paintEvent(QPaintEvent*p){

	if(!m_envelope)
		return ;

	int width=QWidget::width();
	int height=QWidget::height();
	//m_width=width;
	//m_height=height;
	QPainter painter(this);
	QPen pen;                                 //创建一个画笔
	pen.setColor(Qt::darkCyan);
	pen.setWidth(1);
	painter.setPen(pen);
	//double xscal=(m_envelope->MaxX-m_envelope->MinX)/width;
	//double yscal=(m_envelope->MaxY-m_envelope->MinY)/height;

	double xscal=m_viewEnvelop.width()/width;
	double yscal=m_viewEnvelop.height()/height;
	AprIt aprItBeg= m_dataSaved.begin();
	AprIt aprItEnd= m_dataSaved.end();
	for(;aprItBeg!=aprItEnd;aprItBeg++){
		Ppoints Pp=aprItBeg->second;
		PpointsIt pIt=Pp.begin();
		for(;pIt!=Pp.end();)
		{
			QVector<QPointF> dl=pIt->second;

			int pcount=dl.size();
			if(pcount==0){
				pIt++;
				continue;
			}
			QPointF * ptDraw=new QPointF[pcount];
			//if(!(m_width==width&&m_height==height)){
			//int j=0;
				for(int i=0;i<pcount;i++){
					dl[i].setX((dl[i].x()-m_viewEnvelop.left())/xscal);
					dl[i].setY((dl[i].y()-m_viewEnvelop.top())/yscal);
					if(dl[i].x()<0||dl[i].x()>width||dl[i].y()<0||dl[i].y()>height)
						int chenqi=8;
					ptDraw[i]=dl[i];
					//int xx=dl[i].x();
					//int yy=dl[i].y();
				}
				//m_width=width;
				//m_height=height;
			//}
			//painter.drawLines(pIt->second);
			painter.drawPolyline(ptDraw,pcount);//pcout-1会出现画线不闭合的现象
			delete ptDraw;

			
			int befF=pIt->first;
			QPointF lp=pIt->second.last();
			pIt++;
			if(pIt!=Pp.end()&&pIt->first==(befF+1)){
				painter.drawLine(lp,pIt->second.first());
			}
		}


	}
}
void MyPainterWidget::mousePressEvent(QMouseEvent *e){
	setCursor(Qt::PointingHandCursor);
	startPnt = e->pos();
	endPnt = e->pos();
	this->isPressed = true;
	//QString msg ="("+QString::number(e->x())+","+QString::number(e->y())+")";
	//QMessageBox::warning(this,tr("Warning"),msg,QMessageBox::Ok);
}

void MyPainterWidget::mouseMoveEvent(QMouseEvent *e){
	if(this->isPressed){

		endPnt = e->pos();
		int width=QWidget::width();
		int height=QWidget::height();

		double xscale=m_viewEnvelop.width()/width;
		double yscale=m_viewEnvelop.height()/height;

		double difwidth=endPnt.x()-startPnt.x();
		double difheight=endPnt.y()-startPnt.y();

		double difviewwidth=difwidth*xscale;
		double difviewheight=difheight*yscale;

		QPointF diffqf(difviewwidth,difviewheight);
		QPointF topleftBef=m_viewEnvelop.topLeft();
		QPointF toplefAft=topleftBef-diffqf;
		m_viewEnvelop.moveTopLeft(toplefAft);
		//readLevelData(m_currentLevel,m_viewEnvelop);

		
		update();                                    //repainter，call paintEvent
		startPnt = endPnt;
	}
}

void MyPainterWidget::mouseReleaseEvent(QMouseEvent *e){
	setCursor(Qt::ArrowCursor);
	if(this->isPressed){

		endPnt = e->pos();
		int width=QWidget::width();
		int height=QWidget::height();

		double xscale=m_viewEnvelop.width()/width;
		double yscale=m_viewEnvelop.height()/height;

		double difwidth=endPnt.x()-startPnt.x();
		double difheight=endPnt.y()-startPnt.y();

		double difviewwidth=difwidth*xscale;
		double difviewheight=difheight*yscale;

		QPointF diffqf(difviewwidth,difviewheight);
		QPointF topleftBef=m_viewEnvelop.topLeft();
		QPointF toplefAft=topleftBef-diffqf;
		m_viewEnvelop.moveTopLeft(toplefAft);
		QTime t;
		t.start();
		if(isViewOrigin)
			int llll=0;//ReadShpFileToMem(m_filename,m_dataSaved);
		else
			readLevelData(m_currentLevel,m_viewEnvelop);
		int time=t.elapsed();

		update();                                    //repainter，call paintEvent
		this->isPressed = false;

	}
}


void MyPainterWidget::SetDataSaved(AllPointsRead &dataSaved){
	m_dataSaved=dataSaved; 
	int j=89;
}
void MyPainterWidget::wheelEvent(QWheelEvent * event){
	 int numDegrees = event->delta() / 8;//滚动的角度，*8就是鼠标滚动的距离
     int numSteps = numDegrees / 15;//滚动的步数，*15就是鼠标滚动的角度
	 QPointF pos=event->pos();
	 int width=QWidget::width();
	 int height=QWidget::height();

	 double xscale=m_viewEnvelop.width()/width;
	 double yscale=m_viewEnvelop.height()/height;

	 double centerx=pos.x()*xscale+m_viewEnvelop.left();
	 double centery=pos.y()*yscale+m_viewEnvelop.top();
	 QPointF center(pos.x()*xscale+m_viewEnvelop.left(),pos.y()*yscale+m_viewEnvelop.top());
	
	 m_RealLevel=m_RealLevel+numSteps;
	 m_currentLevel=m_currentLevel+numSteps;

	 //if(m_currentLevel<0)
		// m_currentLevel=0;
	 //if(m_currentLevel>m_levelcount)
		// m_currentLevel=m_levelcount;


	 //int difd=m_levelcount-m_currentLevel;
	 double scales=pow(m_wheelScale,m_RealLevel);
	 m_viewEnvelop.setWidth(m_shpEnvelop.width()*scales);
	 m_viewEnvelop.setHeight(m_shpEnvelop.height()*scales);
	 m_viewEnvelop.moveCenter(center);
	 //if(0==m_currentLevel)
		// m_viewEnvelop=m_shpEnvelop;
	 QTime t;
	 t.start();
	 if(isViewOrigin)
		int llll=0; //ReadShpFileToMem(m_filename,m_dataSaved);
	 else
		readLevelData(m_currentLevel,m_viewEnvelop);
	 int time=t.elapsed();
	 update();

}
void MyPainterWidget::readLevelData(int level,QRectF viewEnvelope){
	if(level<0)
		level=0;
	if(level>m_levelcount)
		level=m_levelcount;
	string ovrFileName=m_ovrFileName;
	string FileNamedata=m_dataFileName;
	//m_dataSaved.clear();
	clearDataSaved();
	ifstream fcinOvr;
	getFileHandleRead(ovrFileName,fcinOvr);
	//HeadInfo headInf;
	//fcinOvr.read((char *)&headInf,sizeof(HeadInfo));
	//fcinOvr.close();

	ifstream fcinData;
	getFileHandleRead(FileNamedata,fcinData);
	//int headsize=headInf.headsize;
	//int LC=headInf.levelCount;
	//buildQTreeFromOvrFile(fcout,LC,headsize);
	int LevelCount=QtreeMap.size();
	//for(int i=0;i<LevelCount;i++){
		int i=level;
		int xsize=QtreeMap[i].size();
		for(int j=0;j<xsize;j++){
			//int ysize=QtreeMap[i][j].size();
			for(int k=0;k<xsize;k++){
				//tileNode node=QtreeMap[i][j][k];
				int count=QtreeMap[i][j].count(k);
				nodeIt it=QtreeMap[i][j].find(k);
				//判断当前的瓦片与图像是否有相交，
				QRectF qf(it->second.dbXMin,it->second.dbYMin,it->second.dbXMax-it->second.dbXMin,it->second.dbYMax-it->second.dbYMin);
				if(!m_viewEnvelop.intersects(qf))
					continue;
				for(int m=0;m<count;m++){
					readDataByNode(fcinData,it->second, m_dataSaved);
					it++;
				}
			}
			//break;
		}
		//break;
	//}

}