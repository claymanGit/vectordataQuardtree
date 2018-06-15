
#include"stdio.h"
#include<math.h>
#include"funtions.h"
bool isInCir(const shpPoint ptc,const double Radius,const shpPoint pt2){
	double rad2=sqrt(pow(pt2.dbX-ptc.dbX,2)+pow(pt2.dbY-ptc.dbY,2));
	return rad2<Radius?true:false;
}
bool isInCir(const OGRPoint ptc,const double Radius,const OGRPoint pt2){
	double rad2=sqrt(pow(pt2.getX()-ptc.getX(),2)+pow(pt2.getY()-ptc.getY(),2));
	return rad2<Radius?true:false;
}
// 1 先读取数据点集，确定层级，
// 2 读取图像的envelope和显示图像的大小，确定图像的每层的分辨率
// 3 每层分辨率的确定SVO
// 4 按照SVO来进行图像的分辨率
// 5 猜想，每层交替按照线抽稀和点抽稀来进行。
double setSVO();
OGRPoint * getIntersection(const OGRPoint &ptc,const double &Radius,const OGRPoint &pt1,const OGRPoint & pt2){
	//double diffx=pt2.getX()-pt1.getX();
	//double diffy=pt2.getY()-pt1.getY();
	//double a=pow(diffx,2)+pow(diffy,2);
	//double b=-2*(diffx+diffy);
	//double c=pow(ptc.getX(),2)+pow(ptc.getY(),2)-pow(Radius,2);
	double a=pow(pt2.getX()-pt1.getX(),2)+pow(pt2.getY()-pt1.getY(),2);
	double b=2*((pt2.getX()-pt1.getX())*(pt1.getX()-ptc.getX())+(pt2.getY()-pt1.getY())*(pt1.getY()-ptc.getY()));
	double c=pow(pt1.getX()-ptc.getX(),2)+pow(pt1.getY()-ptc.getY(),2)-pow(Radius,2);
	if((pow(b,2)-4*a*c)>0){
		double k1=(-b+sqrt(pow(b,2)-4*a*c))/(2*a);
		double k2=(-b-sqrt(pow(b,2)-4*a*c))/(2*a);
		double k;
		if(k1>0&&k1<1)
			k=k1;
		else
			k=k2;
		OGRPoint *p1=new OGRPoint;
		p1->setX(k*(pt2.getX()-pt1.getX())+pt1.getX());
		p1->setY(k*(pt2.getY()-pt1.getY())+pt1.getY());

		//p1->getX()=k*pt2.getX()+(1-k)*pt1.getX();   //二次差值，效果可能会更好点
		//p1->getY()=k*pt2.getY()+(1-k)*pt1.getY();
		return p1;
	}
	else
		return NULL;
}
//shpPoint *  getIntersection(const shpPoint &ptc,const double &Radius,const shpPoint &pt1,const shpPoint & pt2){
//	//double diffx=pt2.dbX-pt1.dbX;
//	//double diffy=pt2.dbY-pt1.dbY;
//	//double a=pow(diffx,2)+pow(diffy,2);
//	//double b=-2*(diffx+diffy);
//	//double c=pow(ptc.dbX,2)+pow(ptc.dbY,2)-pow(Radius,2);
//	double a=pow(pt2.dbX-pt1.dbX,2)+pow(pt2.dbY-pt1.dbY,2);
//	double b=2*((pt2.dbX-pt1.dbX)*(pt1.dbX-ptc.dbX)+(pt2.dbY-pt1.dbY)*(pt1.dbY-ptc.dbY));
//	double c=pow(pt1.dbX-ptc.dbX,2)+pow(pt1.dbY-ptc.dbY,2)-pow(Radius,2);
//	if((pow(b,2)-4*a*c)>0){
//		double k1=(-b+sqrt(pow(b,2)-4*a*c))/(2*a);
//		double k2=(-b-sqrt(pow(b,2)-4*a*c))/(2*a);
//		double k;
//		if(k1>0&&k1<1)
//			k=k1;
//		else
//			k=k2;
//		shpPoint *p1=new shpPoint();
//		p1->dbX=k*(pt2.dbX-pt1.dbX)+pt1.dbX;
//		p1->dbY=k*(pt2.dbY-pt1.dbY)+pt1.dbY;
//
//		//p1->dbX=k*pt2.dbX+(1-k)*pt1.dbX;   //二次差值，效果可能会更好点
//		//p1->dbY=k*pt2.dbY+(1-k)*pt1.dbY;
//		return p1;
//	}
//	else
//		return NULL;
//}

vector<OGRPoint> Li_OpenShaw_Line(vector<OGRPoint> &points){
	double SVO=1;
	int pcount=points.size();
	vector<OGRPoint> sampleLine;
			int z=0;
			OGRPoint center=points[z];
			for(int k=1;k<pcount;k++){
				if(isInCir(center,SVO,sampleLine[k]))
					continue;
				OGRPoint p1=points[k-1];
				OGRPoint p2=points[k];
				OGRPoint * p=getIntersection(center,SVO,p1,p2);
				OGRPoint Rp;
				Rp.setX((center.getX()+p->getX())/2);
				Rp.setY((center.getY()+p->getY())/2);
				sampleLine.push_back(Rp);
				center=*p;
			}
	return sampleLine;
}
//
//Polygon * Li_OpenShaw_Polygon(Polygon polygon){
//	double SVO=1;
//	int psize=polygon.m_Polygon.size();
//	Polygon *samplePolygon=new Polygon();
//	for(int i=0;i<psize;i++){
//		Parts *sampleParts=new Parts();
//		Parts *part=polygon.m_Polygon[i];
//		int plsize=part->m_parts.size();
//		for(int j=0;j<plsize;j++)
//		{
//			Points *samplePoints=new Points();
//			Points *pts=part->m_parts[j];
//			int pcount=pts->m_Points.size();
//			int z=0;
//			shpPoint *center=pts->m_Points[z];
//			for(int k=1;k<pcount;k++){
//				if(isInCir(*center,SVO,*pts->m_Points[k]))
//					continue;
//				shpPoint p1=*(pts->m_Points[k-1]);
//				shpPoint p2=*(pts->m_Points[k]);
//				shpPoint * p=getIntersection(*center,SVO,p1,p2);
//				shpPoint Rp;
//				Rp.dbX=(center->dbX+p->dbX)/2;
//				Rp.dbY=(center->dbY+p->dbY)/2;
//				samplePoints->m_Points.push_back(&Rp);
//				center=p;
//			}
//			sampleParts->m_parts.push_back(samplePoints);
//		}
//		samplePolygon->m_Polygon.push_back(sampleParts);
//	}
//	samplePolygon->m_extents=polygon.m_extents;
//	//samplePolygon->m_ID=polygon.m_ID;
//	samplePolygon->m_uIndex=polygon.m_uIndex;
//	return samplePolygon;
//} 
//void li_OpenShaw(ShpFile &fileData){
//	ShpFile sampleFile(fileData);
//
//
//	switch(fileData.m_shpType){
//	case NULLSHP:
//		break;
//	case SHAPE_POINT:
//		break;
//	case POLYLINE:
//		{
//			int linesize=fileData.m_listLines->size();
//			list<points*>::iterator lfineBegin=fileData.m_listLines->begin();
//			list<points*>::iterator lfileEnd=fileData.m_listLines->end();
//			for(;lfineBegin!=lfileEnd;lfineBegin++){
//				sampleFile.m_listLines->push_back(Li_OpenShaw_Line(**lfineBegin)) ;
//			}
//			break;
//		}
//	case SHAPE_POLYGON:
//		{
//			int polygonsize=fileData.m_listPolygons->size();
//			list<Polygon*>::iterator lfineBegin=fileData.m_listPolygons->begin();
//			list<Polygon*>::iterator lfileEnd=fileData.m_listPolygons->end();
//			for(;lfineBegin!=lfileEnd;lfineBegin++){
//				sampleFile.m_listPolygons->push_back(Li_OpenShaw_Polygon(**lfineBegin)) ;
//			}
//		}
//		break;
//
//	}
//}
//
