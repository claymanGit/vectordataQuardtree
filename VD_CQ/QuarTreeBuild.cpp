
#include"stdio.h"
//#include"VectorElements.h"
#include<sstream>
#include<cmath>
#include<fstream>
#include<string.h>
#include<map>
#include<set>
#include<iostream>
#include<vector>
#include<gdal.h>
#include<gdal_priv.h>
#include<ogrsf_frmts.h>
#include<ogr_feature.h>
#include"funtions.h"

 using namespace std;
 QuadMap _QuadMap;
 QuadMap QtreeMap;
 nodeIt it;
 ULLONG OvrfileLenght=0;
 int  LevelCount=0;
map<int ,set<int> >FIDset;
float heightMin=0;
float heightMax=0;
set<float> heightSet;//存储登高线的高程值，以便于用等高线来作为线抽取的工作，
map<int ,set<float> >FIDHeightset;

string getBaseName(string &filepath){
	int index1=filepath.find_last_of("\\");
	int index2=filepath.find_last_of(".");
	string basename=filepath.substr(index1+1,index2-index1-1);
	return basename;
}
string  combineNodeName(string &baseName,int level,int y,int x){
	ostringstream oss;
	oss<<baseName<<"_L"<<level<<"_Y"<<y<<"_X"<<x;
	return oss.str();
}
string  getSubPath(string &filepath){
	int index=filepath.find_last_of("\\");
	string str= filepath.substr(0,index-1);
	return str;
}
void strToChar(char nodenameC[],string & nodenameS){
	int strlen=nodenameS.length();
	if(strlen>NODENAELENGHT){

	}
	for(int i=0;i<strlen;i++){
		nodenameC[i]=nodenameS[i];
	}
}
bool isPinNode(tileNode & node,OGRPoint p){
	double x=p.getX();
	double y=p.getY();
	double z=p.getZ();
	if((int)node.dbXMin<=x&&node.dbXMax>=(int)x&&(int)node.dbYMin<=y&&node.dbYMax>=(int)y)
		return true;
	else
		return false;
}
void setFileEnvelope(const std::string & filename,OGREnvelope* envelope){
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8","NO");
	OGRRegisterAll();
	OGRDataSource * poDS=(OGRDataSource*)OGRSFDriverRegistrar::Open(filename.c_str(),FALSE);     //gdal1.9
	//OGRSFDriverH * ogrsf=new OGRSFDriverH();
	//OGRDataSource * poDS=(OGRDataSource*)OGROpen(filename.c_str(),0,ogrsf);	
	if(!poDS){
		return ;
	}
	int layerCount=poDS->GetLayerCount();
	OGRLayer *poLayer=poDS->GetLayer(0);
	poLayer->GetExtent(envelope);
	GDALClose(poDS);
}

void isfailed(ifstream & fcin){
	bool good=fcin.good();
	bool badbit=fcin.bad();
	bool fail=fcin.fail();
	if(badbit)
		cout<<"badbit"<<endl;
	if(fail)
	{	
		cout<<"fail"<<endl;
		fcin.clear();
	}
	if(good)
		cout<<"good"<<endl;
}
/*vector<SVOtoLevel> */ 
int getQuardLeves(const std::string & filename){
	//return NULL;
	int PNum=getShpFilePointsNum(filename);
	int tempPNum=PNum;
	int levelNum=1;
	while(tempPNum>TOPLEVELPNUMS){//大于最顶层的点的话就继续增加层级，每层点数按照2的比例减小
		tempPNum/=4;//  层级的控制方法和do the resample对应吧应该
		levelNum++;
	}
	return levelNum;
}

void setFIDset(int FIDCount,int qtlevel){
	set<int> fidset;
	for(int i=0;i<FIDCount;i++)
		fidset.insert(i);
	FIDset.insert(make_pair<int,set<int> >(qtlevel-1,fidset));
	for(int i=qtlevel-2;i>=0;i--){
		int fsize=fidset.size();
		set<int>::iterator itbeg=fidset.begin();
		set<int>::iterator itend=fidset.end();
		if(i%2!=0){
			for(;itbeg!=itend;itbeg++){
				if(itbeg==itend)
					break;
				itbeg=fidset.erase(itbeg);
				if(itbeg==itend)
					break;
			}
		}
		/*else{
		do 
		{		
		itbeg++;
		if(itbeg==itend)
		break;
		itbeg=fidset.erase(itbeg);
		} while (itbeg!=itend);

		}*/

		FIDset.insert(make_pair<int,set<int> >(i,fidset));
		
	}
}
//层级采样，两种，一层线采样，一层点采样，
//每层线采样和点采样
void setFIDHeightset(int FIDCount,int qtlevel){

	FIDHeightset.insert(make_pair<int,set<float> >(qtlevel-1,heightSet));
	for(int i=qtlevel-2;i>=0;i--){
		int fsize=heightSet.size();
		set<float>::iterator itbeg=heightSet.begin();
		set<float>::iterator itend=heightSet.end();
		if(i%2!=0){
			for(;itbeg!=itend;itbeg++){
				if(itbeg==itend)
					break;
				itbeg=heightSet.erase(itbeg);
				if(itbeg==itend)
					break;
			}
		}
		else{
		  do {		
		      itbeg++;
		      if(itbeg==itend)
		        break;
		      itbeg=heightSet.erase(itbeg);
		  } while (itbeg!=itend);

		}

		FIDHeightset.insert(make_pair<int,set<float> >(i,heightSet));
		
	}
}
vector<OGRPoint> doTheResample(vector<OGRPoint> points){
	int pNums=points.size();
	vector<OGRPoint> pointsSampled;

	for(int i=1;i<=pNums;i++){
		if(i==pNums&&i!=1){
			pointsSampled.push_back(points[i-1]);//保存最后一个点
			continue;
		}
		if(i%2==0)
			pointsSampled.push_back(points[i-1]);
	}
	return pointsSampled;
}
//http://blog.sina.com.cn/s/blog_67f99fc40100r8h1.html
//  圆算法  ：阈值R =线段长度/(节点个数-1)/2
bool isInCircle(double threadhold,OGRPoint center,OGRPoint p){
	double xsqure=pow(p.getX()-center.getX(),2);
	double ysqure=pow(p.getY()-center.getY(),2);
	double distance=sqrt(xsqure+ysqure);
	if(distance>threadhold)
		return false;
	else
		return true;
}
vector<OGRPoint> doTheResample_circle(vector<OGRPoint> points,OGREnvelope *envelopF){
	
	int pNums=points.size();
	vector<OGRPoint> pointsSampled;
	if(pNums==1)
		return pointsSampled;
	double xsqure=pow(envelopF->MaxX-envelopF->MinX,2);
	double ysqure=pow(envelopF->MaxY-envelopF->MinY,2);
	double lenght=sqrt(xsqure+ysqure);
	double threadhold=lenght/((pNums-1));
	OGRPoint centerP;
	centerP=points[0];
	//pointsSampled.push_back(points[0]);
	for(int i=0;i<pNums;i++){
		if(isInCircle(threadhold,centerP,points[i]))
			continue;
		pointsSampled.push_back(points[i]);
		centerP=points[i];
	}
	//pointsSampled.push_back(points[pNums-1]);
	return pointsSampled; 
}
int getShpFilePointsNum(const std::string & filename){
	int numPoints=0;
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8","NO");
	OGRRegisterAll();
	OGRDataSource * poDS=(OGRDataSource*)OGRSFDriverRegistrar::Open(filename.c_str(),FALSE);     //gdal1.9
	//OGRSFDriverH * ogrsf=new OGRSFDriverH();
	//OGRDataSource * poDS=(OGRDataSource*)OGROpen(filename.c_str(),0,ogrsf);	
	if(!poDS){

	}
	int layerCount=poDS->GetLayerCount();
	OGRLayer *poLayer=poDS->GetLayer(0);
	OGREnvelope *envelope=new OGREnvelope();
	poLayer->GetExtent(envelope);
	int refcount=poLayer->GetRefCount();

	//readDBFByGDAL(filename);

	poLayer->ResetReading();
	OGRFeature * poFeature;
	unsigned int num=0;
	GIntBig fecount=poLayer->GetFeatureCount();
	vector<double> vd;
	while((poFeature=poLayer->GetNextFeature())!=NULL){
		OGRGeometry * poGeometry=poFeature->GetGeometryRef();
		int fid=poFeature->GetFID();
		if(poGeometry!=NULL){
			switch(wkbFlatten(poGeometry->getGeometryType())){
			case wkbLineString:{


				float pHight=(float)poFeature->GetFieldAsDouble("ELEVATION");
				if(pHight>heightMax)
					heightMax=pHight;
				if(pHight<heightMin)
					heightMin=pHight;
				heightSet.insert(pHight);
				OGREnvelope *evlop=new OGREnvelope();

				num++;
				OGRLineString *poLine=(OGRLineString *)poGeometry;
				poLine->getEnvelope(evlop);

				int pointCount=poLine->getNumPoints();
				numPoints+=pointCount;
				break;
							   }

			default:
				printf("another type of the vector element");

			}
		}
	}
	GDALClose(poDS);
	return numPoints;
}


void writeLineDataToD(ofstream & fcout,vector<OGRPoint>  &pTw,ULLONG position){
	int psize=pTw.size();
	int OGRGsize=sizeof(OGRPoint);
	int sizep=sizeof(shpPoint);
	int bufs=sizeof(shpPoint)*psize;
	shpPoint *ps=(shpPoint *)malloc(bufs);
	ULLONG pp1=fcout.tellp();
	if(position!=pp1)
		cout<<"传入的fcout的流可能不正确"<<endl;
	for(int i=0;i<psize;i++){
		shpPoint p;
		double x=pTw[i].getX();
		double y=pTw[i].getY();
		double z=pTw[i].getZ();
		p.dbX=x;
		p.dbY=y;
		p.dbZ=z;
		ps[i]=p;
	}

	fcout.write((char *)ps,bufs);
	free(ps);
	ULLONG pp2=fcout.tellp();
	cout<<"写入点的位置是："<<pp2<<endl;
}
void writeLineDataToD2(ofstream & fcout1,vector<OGRPoint>  &pTw,int position){
	int psize=pTw.size();
	int OGRGsize=sizeof(OGRPoint);
	int befP=fcout1.tellp();
	if(position!=befP)
		cout<<"传入的fcout的流可能不正确"<<endl;
	int flag=false;

	std::string filenamenew2="C:\\Users\\chenq\\Desktop\\shpfiles\\chinaShp\\1\\bou2_4lovr2.txt";
	ofstream fcout;
	getFileHandleWrite(filenamenew2,fcout);
	fcout.seekp(0,std::ios::beg);
	befP=0;

	for(int i=0;i<psize;i++){
		double x=pTw[i].getX();
		double y=pTw[i].getY();
		double z=pTw[i].getZ();
		fcout.write((char*)&x,sizeof(double));
		fcout.write((char*)&y,sizeof(double));
		fcout.write((char*)&z,sizeof(double));
		bool failed=fcout.fail();
		int curP=fcout.tellp();
		fcout.flush();
		if(curP-befP!=24){
			//fcout.seekp(-1,std::ios::cur);
			//curP=fcout.tellp();
			flag=true;
		}
		befP=curP;
		//cout<<"写入点的位置是："<<pp2<<endl;
	}
	if(flag)
	{
		int pp2=fcout.tellp();
		fcout.flush();
		fcout.close();
		ifstream fcin;
		getFileHandleRead("C:\\Users\\chenq\\Desktop\\shpfiles\\chinaShp\\1\\bou2_4lovr2.txt",fcin);
		fcin.seekg(0,std::ios::beg);fstream fs;
		shpPoint *sp=(shpPoint *)malloc(sizeof(shpPoint)*psize);
		for(int i=0;i<psize;i++){

		}
		fcin.read((char *)sp,sizeof(shpPoint)*psize);
		int po=fcin.tellg();
		if(po==pp2)
			cout<<"well done"<<endl;
		free(sp);
	}

	int pp2=fcout.tellp();
	cout<<"写入点的位置是："<<pp2<<endl;
}
void writeLineToDestination(ofstream & fcout,OGRLineString *poLine,int iShpType,int FID,float FIDHeight)
{
	OGREnvelope *envelopF=new OGREnvelope();
	poLine->getEnvelope(envelopF);
	int pointCount=poLine->getNumPoints();

	//保存所有的点到pointsL中，后面直接简化：
	OGRPoint pTosave;
	vector<OGRPoint> pointsL;
	for(int i=0;i<pointCount;i++){
		poLine->getPoint(i,&pTosave);
		pointsL.push_back(pTosave);
	}
	int levelc;
	levelc=_QuadMap.size();
	int j,k;
	for( int i=levelc-1;i>=0;i--)  //金字塔层级从上到下为1-n，所以n越大的话越接近于底层，
	{		                      //所以写入的时候就应该从底层开始写入，所以这个for循环写成这样		
		//set<int> levelFIdset=FIDset[i];
		//if(levelFIdset.find(FID)==levelFIdset.end())
		//{
		//	break;
		//}
		//set<float> levelFIdHeightset=FIDHeightset[i];
		////此线不在所在的层级中，就跳过
		//if(levelFIdHeightset.find(FIDHeight)==levelFIdHeightset.end())
		//{
		//	break;
		//}
		int pcount=pointsL.size();//点集合数和写入数据点的pTos点；以及FIDP
		int pTos=0;   
		int FIDP=0;
		int ysize=_QuadMap[i].size();
		for( j=0;j<ysize;j++)
		{
			if(pTos>=pcount)
				break;
			//int xsize=_QuadMap[i][j].size();
			for( k=0;k<ysize;k++)
			{
				if(pTos>=pcount)
					break;
				nodeIt &ite=getTileNode(i,j,k);
				tileNode &Node=it->second;
				if(pTos<pcount&&!isPinNode(Node,pointsL[pTos]))//如果起点不在此范围内，则跳过：
					continue;
				vector<OGRPoint> pInNode;
				for( ;pTos<pcount&&isPinNode(Node,pointsL[pTos]);pTos++){//存入节点到缓存中
					OGRPoint pt=pointsL[pTos];
					pInNode.push_back(pt);
				}

				bool flag=false;
				if(pTos<pcount){//每遍历一遍所有的节点是否有节
					flag=true;
				}

				LContentHead contentToWrite;
				FIDP++;
				int psize=pInNode.size();
				//存入后一个节点中
				if(pTos<pcount-1){
					psize=psize+1;
					pInNode.push_back(pointsL[pTos+1]);
					//pInNode.push_back(pointsL[pTos+2]);
				}

				contentToWrite.iNumPoints=psize;
				contentToWrite.iNumParts=1;
				contentToWrite.FID=FID;
				contentToWrite.FIDP=FIDP;
				contentToWrite.iShpType=iShpType;


				ULLONG pTowrite=/*node*/Node.currentP;//写入的位置读取
				ULLONG nodeEnd=/*node*/Node.endP;

				//判断是否超出了node的边界
				ULLONG spaceNeed=(psize*sizeof(double)*SAVExyz)+sizeof(LContentHead);//存储的是点是x,y,z的数据
				if(pTowrite+spaceNeed>nodeEnd)
				{
					Node.isexceed=1;
					tileNode tempNode;
					tempNode=Node;
					tempNode.begP=OvrfileLenght;
					tempNode.currentP=OvrfileLenght;
					if(spaceNeed>TILESIZE)//有的线段太过长，导致超过了一个节点所能承受的长度
					{
						int ksize=spaceNeed/TILESIZE+1;
						tempNode.endP=OvrfileLenght=OvrfileLenght+TILESIZE*ksize;
					}
					else
						tempNode.endP=OvrfileLenght=OvrfileLenght+TILESIZE;
					tempNode.lineCount=0;
					_QuadMap[i][j].insert(make_pair<int,tileNode>(k,tempNode));
				}
				nodeIt &ite2=getTileNode(i,j,k);
				tileNode &theNode=it->second;


				pTowrite=theNode.currentP;
				//char toseek='a';
				//fcout.seekp(0,std::ios::beg);
				//fcout.write((char*)&toseek,pTowrite);
				//ULLONG positien=fcout.tellp();
				fcout.seekp(pTowrite);//写入记录头的位置
				if(fcout.fail())
					cout<<"too long to seek the file position"<<endl;
				ULLONG pp=fcout.tellp();

				ULLONG datapistion=pTowrite+sizeof(LContentHead);
				//contentToWrite.dataP=pTowrite+sizeof(LContentHead);//数据读取位置

				fcout.write((char*)&contentToWrite,sizeof(LContentHead));
				if(fcout.fail())
					int kk=4;
				ULLONG pp2=fcout.tellp();//返回记录头后的位置要写入的位置

				if(datapistion!=pp2)
					cout<<"记录头写入的位置不准确"<<endl;
				//if(pp2%2!=0)
				//	pp2++;

				contentToWrite.dataP=pp2;
				fcout.seekp(pp2);
				writeLineDataToD(fcout,pInNode,pp2);
				if(fcout.fail())
					int kk=4;
				ULLONG fpt=fcout.tellp();
				//if(fpt%2!=0)
				//	fpt++;

				int pointc=(fpt-pp2)/24;
				if(pointc!=pInNode.size())
					cout<<"写入数据点后返回的位置不准确"<<endl;

				contentToWrite.endP=fpt;
				fcout.seekp(pTowrite);
				fcout.write((char*)&contentToWrite,sizeof(LContentHead));
				if(fcout.fail())
					int kk=4;
				ULLONG pp3=fcout.tellp();


				theNode.currentP=fpt;
				theNode.lineCount=theNode.lineCount+1;//记录每个node节点所有的线数
				//这里应该判断是否超过了node的范围，而需要单独开辟空间
				//cout<<"当前的位置为："<<currentp<<endl;

				//如果flag为true，那么久要重新遍历此层级的全部节点
				if(flag){
					j=0;
					k=-1;
				}
			}

		}
		//TODO resample;d
		//********//
		pointsL=doTheResample(pointsL);
		//pointsL=doTheResample_circle(pointsL,envelopF);
		if(pointsL.size()==0||pointsL.size()==1)
			break;
		pointsL=doTheResample(pointsL);
		//pointsL=doTheResample_circle(pointsL,envelopF);
		if(pointsL.size()==0||pointsL.size()==1)
			break;
		//int psize=pointsL.size();
		//int rsize=0;
		//do{
		//	pointsL=doTheResample(pointsL);
		//	rsize=pointsL.size();
		//}while(rsize>(psize/4));
		//if(pointsL.size()==0||pointsL.size()==1)
		//	break;
	}
}
void writeNodesToDestination(fstream & fcout,int ppt){
	fcout.seekp(ppt,std::ios::beg);
	int pp=fcout.tellp();

	if(pp!=ppt)
		cout<<"error position"<<endl;
	int levelc;
	levelc=_QuadMap.size();
	for(int i=0;i<levelc;i++)
	{
		int ysize=_QuadMap[i].size();
		for(int j=0;j<ysize;j++)
		{
			int xsize=_QuadMap[i][j].size();
			for(int k=0;k<xsize;k++)
			{
				int count=_QuadMap[i][j].count(k);
				nodeIt it=_QuadMap[i][j].find(k);

				for(int m=0;m<count;m++){
					tileNode &node=it->second;
					fcout.write((char*)&node,sizeof(tileNode));
					if(fcout.fail())
						cout<<"write node to file error"<<endl;
					int currentp=fcout.tellp();
					cout<<"当前的位置为："<<currentp<<endl;
					it++;
				}
				//tileNode node=_QuadMap[i][j][k];
				//fcout.write((char*)&_QuadMap[i][j][k],sizeof(tileNode));
				//int currentp=fcout.tellp();
				//cout<<"当前的位置为："<<currentp<<endl;
			}
		}
	}
}
void writeShptoDestination(string & originfile,string datafilename,string Ovrfilename){
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8","NO");
	OGRRegisterAll();
	OGRDataSource * poDS=(OGRDataSource*)OGRSFDriverRegistrar::Open(originfile.c_str(),FALSE);     //gdal1.9
	//OGRSFDriverH * ogrsf=new OGRSFDriverH();
	//OGRDataSource * poDS=(OGRDataSource*)OGROpen(originfile.c_str(),0,ogrsf);	
	if(!poDS){

	}
	ifstream fcin;
	getFileHandleRead(Ovrfilename,fcin);
	//fcout.open(newfilename.c_str());//创立金字塔文件：
	//if(!fcout.is_open()){
	//	fcout.open(newfilename.c_str(),std::ios::out);
	//	fcout.close();
	//	fcout.open(newfilename.c_str(),std::ios::in|std::ios::out|std::ios::ate);
	//}
	HeadInfo headInf,headInf2;
	fcin.read((char *)&headInf,sizeof(HeadInfo));
	fcin.seekg(0,std::ios::beg);
	fcin.read((char *)&headInf2,sizeof(HeadInfo));	
	int headsize=headInf.headsize;
	int LC=headInf.levelCount;
	fcin.close();

	//ofstream fcout0;
	//getFileHandleWrite(Ovrfilename,fcout0);
	//fcout0.seekp(0,std::ios::beg);
	//fcout0.write((char *)&headInf,sizeof(HeadInfo));
	////fcout0.seekp(headsize+sizeof(HeadInfo),std::ios::beg);
	////fcout.write((char *)&headsize,sizeof(int));
	//int curp=fcout0.tellp();
	//fcout0.close();
	ofstream fcout;
	getFileHandleWrite(datafilename,fcout);

	int layerCount=poDS->GetLayerCount();
	OGRLayer *poLayer=poDS->GetLayer(0);
	OGREnvelope *envelopeLayer=new OGREnvelope();
	poLayer->GetExtent(envelopeLayer);
	poLayer->ResetReading();
	OGRFeature * poFeature;
	unsigned int num=0;
	GIntBig fecount=poLayer->GetFeatureCount();
	setFIDset(fecount,LevelCount);
	setFIDHeightset(fecount,LevelCount);
	vector<double> vd;
	int ishpType;
	OGREnvelope *envelopFbef;
	bool bef=false;
	int allcountfid=0;
	while((poFeature=poLayer->GetNextFeature())!=NULL){
		OGRGeometry * poGeometry=poFeature->GetGeometryRef();
		int fid=poFeature->GetFID();
		if(poGeometry!=NULL){
			switch(wkbFlatten(poGeometry->getGeometryType()))
			{
			case wkbLineString:
				{
					float pHight=(float)poFeature->GetFieldAsDouble("ELEVATION");
					OGREnvelope *envelopF=new OGREnvelope();
					OGRLineString *poLine=(OGRLineString *)poGeometry;
					poLine->getEnvelope(envelopF);
					/*if(!bef)
					{envelopFbef=envelopF;
					bef=true;
					}
					if(envelopF->Contains(*envelopFbef))
						int lck=4;
					else
						int jbc=5;
					envelopFbef=envelopF;*/
					OGRBoolean isRing=poLine-> IsRing();
					ishpType=3;
					allcountfid++;
					writeLineToDestination(fcout,poLine,ishpType,fid,pHight);
					delete envelopF;
					break;
				}
			case wkbPolygon:
				{
					//float pHight=(float)poFeature->GetFieldAsDouble("elev");
					OGREnvelope *envelopF=new OGREnvelope();
					num++;
					OGRLineString *poLine=(OGRLineString *)poGeometry;
					poLine->getEnvelope(envelopF);

					ishpType=5;
					int pointCount=poLine->getNumPoints();
					if(pointCount==1)
						ishpType=1;
					writeLineToDestination(fcout,poLine,ishpType,fid,0);

					break;
				}
			}
		}
	}


	fstream fcout2;
	getFileHandle(Ovrfilename,fcout2);
	fcout2.seekp(0,std::ios::beg);
	fcout2.write((char*)&headInf,sizeof(HeadInfo));
	int curpt0=fcout2.tellp();
	fcout2.seekp(sizeof(HeadInfo),std::ios::beg);
	int curpt=fcout2.tellp();

	writeNodesToDestination(fcout2,curpt);
	int fptop2=fcout.tellp();
	fcout2.flush();
	//fcout2.close();


	HeadInfo headInf3;
	fcout2.seekg(0,std::ios::beg);
	fcout2.read((char *)&headInf3,sizeof(HeadInfo));
	int fpt=fcout2.tellg();
	tileNode nodes;
	fcout2.read((char *)&nodes,sizeof(tileNode));
	fcout2.close();
	int pppp2=0;
}

void buildDestinationFile(string & originfile,string newfilename){

	//string newfilename;
	double originfileLenght;
	int levelcount=getQuardLeves(originfile);//depend the size of the datavolume//金字塔层级数
	//LevelCount=levelcount;//赋值全局变量
	OGREnvelope *fileenvelope=new OGREnvelope();
	setFileEnvelope(originfile,fileenvelope);
	tileNode hhead;
	tileNode node;
	int *eLevelTileCount=(int *)malloc(sizeof(int)*levelcount);//金字塔
	int *eLevelBoundryCount=(int *)malloc(sizeof(int)*levelcount);
	int basesacle=4;//面积的每层级缩略
	int baseboundrysacle=2;//边长没层级缩略
	int filenum=0;
	for(int i=0;i<levelcount;i++){//每个层级应该对应的节点数
		eLevelTileCount[i]=pow((double)basesacle,i);
		eLevelBoundryCount[i]=pow((double)baseboundrysacle,i);
		filenum+=eLevelTileCount[i];
	}

	double headsize=sizeof(tileNode)*filenum;//文件头所占据空间的大小：

	HeadInfo headInfo;
	headInfo.levelCount=levelcount;
	headInfo.headsize=headsize;

	double Headsize1A2=headsize+sizeof(HeadInfo);//整个文件头所存在的大小

	OvrfileLenght=/*Headsize1A2+*/TILESIZE*filenum;

	//hhead.tilecout=filenum;
	//hhead.headLenght=headsize;
	string subpath=getSubPath(originfile);
	string baseName=getBaseName(originfile);

	double width=fileenvelope->MaxX - fileenvelope->MinX;
	double height=fileenvelope->MaxY - fileenvelope->MinY;


	ULLONG numHasDeal=0;

	ofstream fcout;
	getFileHandleWrite(newfilename,fcout);
	//fcout.open(newfilename.c_str());//创立金字塔文件：
	//if(fcout.is_open())
	//	remove(newfilename.c_str());
	//if(!fcout.is_open()){
	//	fcout.open(newfilename.c_str(),std::ios::out);
	//	fcout.close();
	//	fcout.open(newfilename.c_str(),std::ios::in|std::ios::out|std::ios::ate);
	//}
	fcout.write((char*)&headInfo,sizeof(HeadInfo));
	int hsize=sizeof(HeadInfo);
	int fpp=fcout.tellp();

	if(fcout.fail())
		exit(1);
	//fcout.write((char*)&hhead,sizeof(hhead));


	for(int i=0;i<levelcount;i++){
		tileNode * nodesInlvel=(tileNode *)malloc(sizeof(tileNode)*eLevelTileCount[i]);

		double wstep=width/eLevelBoundryCount[i];
		double hstep=height/eLevelBoundryCount[i];

		double tileEnvelop[4];

		for(int j=0;j<eLevelBoundryCount[i];j++){//x

			for(int k=0;k<eLevelBoundryCount[i];k++){  //y

				tileEnvelop[1]= fileenvelope->MinX + (k+1) * wstep;//每个节点文件的四至范围//k,j,.....
				tileEnvelop[3]= fileenvelope->MinY + (j+1) * hstep;
				tileEnvelop[0]=tileEnvelop[1]-wstep;
				tileEnvelop[2]=tileEnvelop[3]-hstep;

				int pInLevel=j*eLevelBoundryCount[i]+k;//数组下标
				tileNode node;
				node.dbXMin=tileEnvelop[0];
				node.dbXMax=tileEnvelop[1];
				node.dbYMin=tileEnvelop[2];
				node.dbYMax=tileEnvelop[3];
				node.lineCount=0;
				node.level=i;
				node.levely=j;
				node.levelx=k;
				ULLONG dbegP=Headsize1A2+numHasDeal*8388608;
				node.begP=dbegP;
				node.currentP=dbegP;
				numHasDeal++;
				node.endP=node.begP+TILESIZE;

				string nodeFileName=combineNodeName(baseName,i,j,k);//......
				strToChar(node.tileName,nodeFileName);
				//writeNodeToDestination(fcout,nodesInlvel[pInLevel]);
				nodesInlvel[pInLevel]=node;
				//_QuadMap[i][j][k]= nodesInlvel[pInLevel];
				_QuadMap[i][j].insert(make_pair<int,tileNode>(k,nodesInlvel[pInLevel]));

			}
		}
		free(nodesInlvel);
	}

	fcout.flush();
	fcout.close();	
	free(eLevelTileCount);
	free(eLevelBoundryCount);
}
void buildDestinationFile2(string & originfile,string ovrfilename){

	//string newfilename;
	double originfileLenght;
	int levelcount=getQuardLeves(originfile);//depend the size of the datavolume//金字塔层级数
	//LevelCount=levelcount;//赋值全局变量
	LevelCount=levelcount;
	OGREnvelope *fileenvelope=new OGREnvelope();
	setFileEnvelope(originfile,fileenvelope);
	tileNode hhead;
	tileNode node;
	int *eLevelTileCount=(int *)malloc(sizeof(int)*levelcount);//金字塔
	int *eLevelBoundryCount=(int *)malloc(sizeof(int)*levelcount);
	int basesacle=4;//面积的每层级缩略
	int baseboundrysacle=2;//边长没层级缩略
	int filenum=0;
	for(int i=0;i<levelcount;i++){//每个层级应该对应的节点数
		eLevelTileCount[i]=pow((double)basesacle,i);
		eLevelBoundryCount[i]=pow((double)baseboundrysacle,i);
		filenum+=eLevelTileCount[i];
	}

	double headsize=sizeof(tileNode)*filenum;//文件头所占据空间的大小：

	HeadInfo headInfo;
	headInfo.levelCount=levelcount;
	headInfo.headsize=headsize;

	double Headsize1A2=headsize+sizeof(HeadInfo);//整个文件头所存在的大小

	OvrfileLenght=/*Headsize1A2+*/TILESIZE*filenum;//整个文件的大小

	//hhead.tilecout=filenum;
	//hhead.headLenght=headsize;
	string subpath=getSubPath(originfile);
	string baseName=getBaseName(originfile);

	double width=fileenvelope->MaxX - fileenvelope->MinX;
	double height=fileenvelope->MaxY - fileenvelope->MinY;


	ULLONG numHasDeal=0;

	ofstream fcout;
	getFileHandleWrite(ovrfilename,fcout);
	//fcout.open(newfilename.c_str());//创立金字塔文件：
	//if(fcout.is_open())
	//	remove(newfilename.c_str());
	//if(!fcout.is_open()){
	//	fcout.open(newfilename.c_str(),std::ios::out);
	//	fcout.close();
	//	fcout.open(newfilename.c_str(),std::ios::in|std::ios::out|std::ios::ate);
	//}
	fcout.write((char*)&headInfo,sizeof(HeadInfo));
	int hsize=sizeof(HeadInfo);
	int fpp=fcout.tellp();

	if(fcout.fail())
		exit(1);
	//fcout.write((char*)&hhead,sizeof(hhead));


	for(int i=0;i<levelcount;i++){
		tileNode * nodesInlvel=(tileNode *)malloc(sizeof(tileNode)*eLevelTileCount[i]);

		double wstep=width/eLevelBoundryCount[i];
		double hstep=height/eLevelBoundryCount[i];

		double tileEnvelop[4];
		Level level;
		for(int j=0;j<eLevelBoundryCount[i];j++){//y
			Row row;
			for(int k=0;k<eLevelBoundryCount[i];k++){  //x

				tileEnvelop[1]= fileenvelope->MinX + (k+1) * wstep;//每个节点文件的四至范围//k,j,.....
				tileEnvelop[3]= fileenvelope->MinY + (j+1) * hstep;
				tileEnvelop[0]=tileEnvelop[1]-wstep;
				tileEnvelop[2]=tileEnvelop[3]-hstep;

				int pInLevel=j*eLevelBoundryCount[i]+k;//数组下标
				tileNode node;
				node.dbXMin=tileEnvelop[0];
				node.dbXMax=tileEnvelop[1];
				node.dbYMin=tileEnvelop[2];
				node.dbYMax=tileEnvelop[3];
				node.lineCount=0;
				node.level=i;
				node.levely=j;
				node.levelx=k;
				ULLONG dbegP=/*Headsize1A2*/0+numHasDeal*TILESIZE/*8388608*/;
				node.begP=dbegP;
				node.currentP=dbegP;
				numHasDeal++;
				node.endP=node.begP+TILESIZE;
				node.isexceed=0;
				string nodeFileName=combineNodeName(baseName,i,j,k);//......
				strToChar(node.tileName,nodeFileName);
				node.NameLenght=nodeFileName.length();
				//writeNodeToDestination(fcout,nodesInlvel[pInLevel]);
				//row[k]=node;
				row.insert(pair<int,tileNode>(k,node));
				//tileNode nodet;
				//int n=row.count(k);
				//nodeIt it=row.find(k);
				//for(int m=0;m<n;m++){
				//	cout<<"multimap first "<<it->first<<endl;
				//}
			}
			level[j]=row;
			row.clear();
		}
		_QuadMap[i]=level;
		level.clear();
		free(nodesInlvel);
	}

	fcout.flush();
	fcout.close();	
	free(eLevelTileCount);
	free(eLevelBoundryCount);
}




void getFileHandleRead(const string & ovrFileName,ifstream & fcout){
	fcout.open(ovrFileName.c_str(),std::ios::in|std::ios::binary);
	if(!fcout.is_open()){ 
		fcout.open(ovrFileName.c_str(),std::ios::in|std::ios::binary);
		fcout.close();
		fcout.open(ovrFileName.c_str(),std::ios::in|std::ios::trunc|std::ios::binary);
	}
}
void getFileHandleWrite(const string & ovrFileName,ofstream & fcout){
	fcout.open(ovrFileName.c_str(),std::ios::out|std::ios::binary/*|std::ios::ate*/);//创建文件的mode很重要
	if(!fcout.is_open()){ 
		fcout.open(ovrFileName.c_str(),std::ios::out|std::ios::binary);
		fcout.close();
		fcout.open(ovrFileName.c_str(),std::ios::out|std::ios::trunc|std::ios::binary);
	}
}
void getFileHandle(const string & ovrFileName,fstream &fhandle){
	fhandle.open(ovrFileName.c_str(),std::ios::out|std::ios::in|std::ios::binary/*|std::ios::ate*/);//创立金字塔文件：
	if(!fhandle.is_open()){ 
		fhandle.open(ovrFileName.c_str(),std::ios::out|std::ios::binary);
		fhandle.close();
		fhandle.open(ovrFileName.c_str(),std::ios::in|std::ios::out|std::ios::trunc|std::ios::binary);
	}
}