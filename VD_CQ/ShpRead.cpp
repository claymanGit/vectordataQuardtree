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
#include"funtions.h"

void ReadShpFileToMem(string & originfile,AllPointsRead &dataSaved){
	dataSaved.clear();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8","NO");
	OGRRegisterAll();
	OGRDataSource * poDS=(OGRDataSource*)OGRSFDriverRegistrar::Open(originfile.c_str(),FALSE);     //gdal1.9
	//OGRSFDriverH * ogrsf=new OGRSFDriverH();
	//OGRDataSource * poDS=(OGRDataSource*)OGROpen(originfile.c_str(),0,ogrsf);	
	if(!poDS){

	}

	int layerCount=poDS->GetLayerCount();
	OGRLayer *poLayer=poDS->GetLayer(0);
	OGREnvelope *envelopeLayer=new OGREnvelope();
	poLayer->GetExtent(envelopeLayer);
	poLayer->ResetReading();
	OGRFeature * poFeature;
	unsigned int num=0;
	GIntBig fecount=poLayer->GetFeatureCount();
	vector<double> vd;
	int ishpType;
	while((poFeature=poLayer->GetNextFeature())!=NULL){
		OGRGeometry * poGeometry=poFeature->GetGeometryRef();
		int fid=poFeature->GetFID();
		if(poGeometry!=NULL){
			switch(wkbFlatten(poGeometry->getGeometryType()))
			{
			case wkbLineString:
				{
					//float pHight=(float)poFeature->GetFieldAsDouble("elev");
					OGREnvelope *envelopF=new OGREnvelope();
					OGRLineString *poLine=(OGRLineString *)poGeometry;
					poLine->getEnvelope(envelopF);
					OGRBoolean isRing=poLine-> IsRing();
					ishpType=3;
					QVector<QPointF> vQP;
					int pCount=poLine->getNumPoints();
					vQP.resize(pCount);
					OGRPoint pTosave;
					for(int j=0;j<pCount;j++){
						poLine->getPoint(j,&pTosave);
						vQP[j].setX(pTosave.getX());
						vQP[j].setY(pTosave.getY());
					}
					dataSaved[fid][1]=vQP;
					break;
				}
			case wkbPolygon:
				{
					//float pHight=(float)poFeature->GetFieldAsDouble("elev");
					OGREnvelope *envelopF=new OGREnvelope();
					num++;
					OGRLineString *polygon=(OGRLineString *)poGeometry;
					polygon->getEnvelope(envelopF);
					ishpType=5;
					int pointCount=polygon->getNumPoints();
					if(pointCount==1)
						ishpType=1;
		
					break;
				}
			}
		}
	}
}



nodeIt & getTileNode(int i,int j,int k){
	multimap<unsigned int,tileNode> &row=_QuadMap[i][j];

	int count=row.count(k);
	it=row.find(k);
	for(int m=0;m<count;m++){
		if(!it->second.isexceed)
			break;
		it++;
	}
	return it;
}
void readDataByNode(ifstream &fcout,tileNode node,AllPointsRead &dataSaved){
	//AllPointsRead dataSaved;
	ULLONG begP=node.begP;
	fcout.seekg(begP);

	int LCount=node.lineCount;
	for(int i=0;i<LCount;i++){
		LContentHead *lch=(LContentHead*)malloc(sizeof(LContentHead));
		isfailed(fcout);
		fcout.sync();
		fcout.read((char *)lch,sizeof(LContentHead));
		isfailed(fcout);

		fcout.seekg(lch->dataP);
		ULLONG datapp=fcout.tellg();


		int pCount=lch->iNumPoints;
		shpPoint * readPoints=(shpPoint *)malloc(sizeof(shpPoint)*pCount);
		fcout.read((char *)readPoints,sizeof(shpPoint)*pCount);
		//ReadPointsFromFile(fcout,pCount,datapp);
		isfailed(fcout);


		ULLONG position=fcout.tellg();

		fcout.seekg(lch->endP);
		position=fcout.tellg();
		QVector<QPointF> vQP;
		vQP.resize(pCount);
		for(int j=0;j<pCount;j++){
			vQP[j].setX(readPoints[j].dbX);
			vQP[j].setY(readPoints[j].dbY);
		}
		dataSaved[lch->FID][lch->FIDP]=vQP;
		free(lch);
		free(readPoints);

		//pair<unsigned int,shpPoint*> p(lch->FIDP,readPoints);
		//dataSaved.insert(make_pair<unsigned int,Ppoints>(lch->FID,p));
		//if(dataSaved[lch->FID][lch->FIDP]!=NULL){

		//}
	}


}
void ReadPointsFromFile(ifstream &fcin,int pCount,int position){
	int pBeg=fcin.tellg();
	shpPoint * readPoints=(shpPoint *)malloc(sizeof(shpPoint)*pCount);
	shpPoint sp;
	int befP=pBeg;
	int curp=pBeg;
	for(int i=0;i<pCount;i++){
		fcin.read((char*)&sp,sizeof(shpPoint));
		int curp=fcin.tellg();
		bool isfail=fcin.fail();
		if(isfail){
			fcin.clear();
			fcin.seekg(befP);
			i--;
			continue;
		}
		befP=curp;
		readPoints[i]=sp;
	}
	free(readPoints);
}

void readData(const string & ovrFileName,const string & FileNamedata,AllPointsRead &dataSaved){
	//fstream fcout;
	//fcout.open(ovrFileName.c_str());//创立金字塔文件：
	//if(!fcout.is_open()){
	//	fcout.open(ovrFileName.c_str(),std::ios::out);
	//	fcout.close();
	//	fcout.open(ovrFileName.c_str(),std::ios::in|std::ios::out|std::ios::ate);
	//}
	ifstream fcinOvr;
	getFileHandleRead(ovrFileName,fcinOvr);
	HeadInfo headInf;
	fcinOvr.read((char *)&headInf,sizeof(HeadInfo));
	fcinOvr.close();

	ifstream fcinData;
	getFileHandleRead(FileNamedata,fcinData);
	int headsize=headInf.headsize;
	int LC=headInf.levelCount;
	//buildQTreeFromOvrFile(fcout,LC,headsize);
	int LevelCount=QtreeMap.size();
	for(int i=0;i<LevelCount;i++){
		int xsize=QtreeMap[i].size();
		for(int j=0;j<xsize;j++){
			//int ysize=QtreeMap[i][j].size();
			for(int k=0;k<xsize;k++){
				//tileNode node=QtreeMap[i][j][k];
				int count=QtreeMap[i][j].count(k);
				nodeIt it=QtreeMap[i][j].find(k);
				for(int m=0;m<count;m++){
					readDataByNode(fcinData,it->second, dataSaved);
					it++;
				}
			}
			break;
		}
		break;
	}
}
void buildQTreeFromOvrFile(fstream &fcout,int LevelCount,int headSize){
	int ppt=fcout.tellg();
	if(ppt!=16)
		cout<<"error position"<<endl;
	//fcout.seekg(16,std::ios::beg);
	int *eLevelTileCount=(int *)malloc(sizeof(int)*LevelCount);//金字塔
	int *eLevelBoundryCount=(int *)malloc(sizeof(int)*LevelCount);

	int basesacle=4;//面积的每层级缩略
	int baseboundrysacle=2;
	int filenum=0;
	for(int i=0;i<LevelCount;i++){//每个层级应该对应的节点数
		eLevelTileCount[i]=pow((double)basesacle,i);
		eLevelBoundryCount[i]=pow((double)baseboundrysacle,i);
		filenum+=eLevelTileCount[i];
	}
	tileNode node;
	LContentHead;
	int beflevel=-1,befx=-1,befy=-1;

	for(int i=0;i<LevelCount;i++)
	{
		for(int j=0;j<eLevelBoundryCount[i];j++)
		{
			for(int k=0;k<eLevelBoundryCount[i];k++)
			{
				int ppt2=fcout.tellg();
				fcout.read((char*)&node,sizeof(tileNode));
				if(fcout.fail())
					cout<<"read error"<<endl;
				//QtreeMap[i][j][k]=node;
				//readDataByNode(fcout,node);
				if(node.level==beflevel&&node.levelx==befx&&node.levely==befy){
					/*k--;*/
					i=beflevel;
					j=befy;
					k=befx;
				}
				QtreeMap[i][j].insert(make_pair<unsigned int,tileNode>(k,node));
				beflevel=node.level;
				befx=node.levelx;
				befy=node.levely;
			}
		}
	}
	//fcout.flush();
	fcout.close();
	free(eLevelTileCount);
	free(eLevelBoundryCount);
}
void BuildQTree(const string & ovrFileName){

	fstream fcin;
	//getFileHandleRead(ovrFileName,fcin);
	fcin.open(ovrFileName.c_str(),std::ios::in|std::ios::out|std::ios::binary);//创立金字塔文件：
	if(!fcin.is_open()){
		fcin.open(ovrFileName.c_str(),std::ios::out);
		fcin.close();
		fcin.open(ovrFileName.c_str(),std::ios::in|std::ios::out|std::ios::ate);
	}
	HeadInfo headInf;
	fcin.seekg(0,std::ios::beg);
	fcin.read((char *)&headInf,sizeof(HeadInfo));
	int ppt=fcin.tellg();
	if(fcin.fail())
		cout<<"read error"<<endl;
	fcin.seekg(ppt,std::ios::beg);
	int headsize=headInf.headsize;
	int LC=headInf.levelCount;
	buildQTreeFromOvrFile(fcin,LC,headsize);
	fcin.close();
}
void readDBFByGDAL(string filename){
	vector<string> m_proName,m_proName2;
	vector<string> pHightProVec,pHightProVec2;
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8","NO");
	OGRRegisterAll();
	OGRDataSource * poDS=(OGRDataSource*)OGRSFDriverRegistrar::Open(filename.c_str(),FALSE);     //gdal1.9

	//OGRSFDriverH * ogrsf=new OGRSFDriverH();
	//OGRDataSource * poDS=(OGRDataSource*)OGROpen(filename.c_str(),0,ogrsf);	
	if(poDS==NULL){
		return ;
	}
	int lc=poDS->GetLayerCount();
	OGRLayer * poLayer=poDS->GetLayer(0);
	if(poLayer!=NULL){

	}
	poLayer->ResetReading();
	OGRFeature *poFeature;
	while((poFeature=poLayer->GetNextFeature())!=NULL){
		OGRGeometry * poGeometry=poFeature->GetGeometryRef();
		if(poGeometry!=NULL){
			if(wkbFlatten(poGeometry->getGeometryType())==wkbLineString)
			{
				OGRFeatureDefn *posDef=poLayer->GetLayerDefn();
				std::string nameText;
				int fieldCount=posDef->GetFieldCount();
				for(int iField=0;iField<fieldCount;iField++)
				{
					OGRFieldDefn * oField=posDef->GetFieldDefn(iField);
					nameText=oField->GetNameRef();
					OGRFieldType pType=oField->GetType();
					if(pType==OFTInteger||pType==OFTReal){
						m_proName.push_back(nameText);
						float pHeight=(float)poFeature->GetFieldAsDouble(iField);
						stringstream ss;
						ss<<nameText<<"("<<pHeight<<",..."<<")";
						nameText=ss.str();
						pHightProVec.push_back(nameText);
					}
					if(pType==OFTString){
						m_proName.push_back(nameText);
						string propetryType=poFeature->GetFieldAsString(iField);
						stringstream ss;
						ss<<nameText<<"("<<propetryType<<",..."<<")";
						nameText=ss.str();
						pHightProVec.push_back(nameText);
					}
				}
				OGRFeature::DestroyFeature(poFeature);
				break;
			}
		}
	}
}