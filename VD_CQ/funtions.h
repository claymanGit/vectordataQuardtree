#pragma once
#ifndef FUNCTIONS_H
#define FUNCTIONS_H

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
#include<QPointF>
#include<QVector>
#include<set>
using namespace std;
#define NULLSHP 0
#define SHAPE_POINT 1
#define POLYLINE 3
#define SHAPE_POLYGON 5
#define MULTIPOINT 8
#define POINTZ 11
#define POLYLINEZ 13
#define POLYGONZ 15
#define POINTM 21
#define POLYLINEM 23
#define POLGONM 25
#define MULTIPOINTM 28
#define MULTIPATCH 31


#define TILESIZE /*8388608*/(1024*1024)   //1M reserved
#define NODENAELENGHT 64//节点文件长度
#define TOPLEVELPNUMS (1<<16) //2的16次方个点
#define SAVExy 2
#define SAVExyz 3

#define ULLONG unsigned long long

//#pragma pack(4)
struct HeadInfo{//金字塔文件固定大小12字节：用来表示文件头的大小和金字塔的的层级
	int levelCount;//金字塔层级数（可以用来重建金字塔）
	double headsize;//文件头的大小，指示读取的位置
};
typedef struct tileNode{           //用于金字塔文件的文件头的节点
	char tileName[NODENAELENGHT];
	int NameLenght;                //记录名字的长度
	int level;
	int levelx;
	int levely;
	int iShpType;
	double dbXMin;
	double dbYMin;
	double dbXMax;
	double dbYMax;
	//double dbZMin;
	//double dbZMax;
	//double dbMMin;
	//double dbMMax;
	int lineCount;                  //记录node节点存的线数
	int isexceed;                  //当前的节点不够存储数据后的节点节点数据
	ULLONG begP;
	ULLONG endP;
	ULLONG currentP;


	//int exceedbegP;
	//int exceedendP;
	//int exceedcurrentP;
	//char reserve[80];
	tileNode(){
		level=0;
		levelx=0;
		levely=0;
		iShpType=0;
		dbXMin=0;
		dbYMin=0;
		dbXMax=0;
		dbYMax=0;
		//dbZMin=0;
		//dbZMax=0;
		//dbMMin=0;
		//dbMMax=0;
		begP=0;
		endP=0;
		currentP=0;
		lineCount=0;
		isexceed=0;
		//exceedbegP=0;
		//exceedendP=0;
		//exceedcurrentP=0;
	}
	tileNode & operator =(tileNode node){
		int lenght=node.NameLenght;
		for(int i=0;i<lenght;i++){
			tileName[i]=node.tileName[i];
		}
		NameLenght=node.NameLenght;
		level=node.level;
		levelx=node.levelx;
		levely=node.levely;
		dbXMin=node.dbXMin;
		dbYMin=node.dbYMin;
		dbXMax=node.dbXMax;
		dbYMax=node.dbYMax;
		return *this;
	}
}NODEHEAD;
typedef struct shpPoint{
	double dbX;
	double dbY;
	double dbZ;
}SHPPOINT;
//struct Point{
//public:
//	int m_uIndex;
//	double dbX;
//	double m_dby;
//};
typedef struct PtContent{
	int iShpType;
	double dbX;
	double dbY;
}PTCONTENT;

typedef struct LContentHead{
	int iShpType;
	int iNumParts;
	int iNumPoints;
	int FID;
	int FIDP;
	ULLONG dataP;
	ULLONG endP;

}LCONTENTHEAD;

typedef struct LContent{
	int iShpType;
	//shpPoint Box[2];
	int iNumParts;
	int iNumPoints;
	//LContentHead head;
	int FID;
	int FIDP;
	int * partOff;
	shpPoint *	points;
}SHPINFO;
typedef struct PContentHead{  //不管点线面，都用这个记录头
	int iShpType;
	//shpPoint Box[2];
	int iNumParts;
	int iNumPoints;
	//LContentHead head;
	int FID;
	int FIDP;
	int isExceed;
}PCONTENTHEAD;
typedef struct PContent{
	int iShpType;
	shpPoint Box[2];
	int iNumParts;
	int iNumPoints;
	//LContentHead head;
	int FID;
	int FIDP;
	int * partOff;
	shpPoint *	points;
}PCONTENT;


typedef std::multimap<unsigned int,tileNode> Row;
typedef std::map<unsigned int,Row> Level;
typedef std::map<unsigned int,Level> QuadMap;

typedef std::multimap<unsigned int,tileNode>::iterator nodeIt;

typedef std::map<unsigned int,QVector<QPointF>/*shpPoint **/> Ppoints;//fidp data
typedef std::map<unsigned int,Ppoints> AllPointsRead;//fid fidpextern QuadMap _QuadMap;
typedef std::map<unsigned int,QVector<QPointF>/*shpPoint **/>::iterator PpointsIt;
typedef std::map<unsigned int,Ppoints>::iterator AprIt;

//金字塔层级从上到下 0~n，最底层为第n级
extern QuadMap _QuadMap;
extern QuadMap QtreeMap;
//extern AllPointsRead dataToSave;
extern ULLONG OvrfileLenght;//现在整个文件占据的大小
extern int  LevelCount;
extern int RowCount;
extern int LineCount;
extern map<int ,set<int> >FIDset;
extern float heightMin;
extern float heightMax;
extern set<float> heightSet;
extern map<int ,set<float> >FIDHeightset;

string getBaseName(string &filepath);
string  combineNodeName(string &baseName,int level,int y,int x);
string  getSubPath(string &filepath);

void strToChar(char nodenameC[],string & nodenameS);

bool isPinNode(tileNode & node,OGRPoint p);
vector<OGRPoint> doTheResample(vector<OGRPoint> points);
void writeLineDataToD(ofstream & fcout,vector<OGRPoint>  &pTw,ULLONG position);
void writeLineDataToD2(ofstream & fcout1,vector<OGRPoint>  &pTw,int position);
extern nodeIt it;
nodeIt & getTileNode(int i,int j,int k);
void writeLineToDestination(ofstream & fcout,OGRLineString *poLine,int iShpType,int FID,float FIDHeight);

void writeNodesToDestination(fstream & fcout,int ppt);
void writeShptoDestination(string & originfile,string datafilename,string Ovrfilename);
//void insertTileToQuardMap(tileNode tile){
//	_QuadMap[tile.level][tile.levely][tile.levelx]=tile;
//}

int getShpFilePointsNum(const std::string & filename);
void setFileEnvelope(const std::string & filename,OGREnvelope* envelope);
void ReadPointsFromFile(ifstream &fcin,int pCount,int position);
void isfailed(ifstream & fcin);
/*vector<SVOtoLevel> */ 
int getQuardLeves(const std::string & filename);


void readDataByNode(ifstream &fcout,tileNode node,AllPointsRead & dataSaved);
void buildDestinationFile(string & originfile,string newfilename);
void buildDestinationFile2(string & originfile,string ovrfilename);
void buildQTreeFromOvrFile(fstream &fcout,int LevelCount,int headSize);
void BuildQTree(const string & ovrFileName);
void readData(const string & ovrFileName,const string & FileNamedata,AllPointsRead &dataSaved);
void readDBFByGDAL(string filename);

void getFileHandleRead(const string & ovrFileName,ifstream & fcout);
void getFileHandleWrite(const string & ovrFileName,ofstream & fcout);
void getFileHandle(const string & ovrFileName,fstream &fhandle);


void ReadShpFileToMem(string & originfile,AllPointsRead &dataSaved);
#endif 
