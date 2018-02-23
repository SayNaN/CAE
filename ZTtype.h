#ifndef ZTTYPE_H
#define ZTTYPE_H

#include"vector.h"

enum NodeType{
  CELLCENTER,            // 节点在节点内部
  MESHNODE,           // 节点在网格界面上
};

enum SimuType{
  Diffuse_1D,
  Convective_Diffusion_1D,
  
  Diffuse_2D,
  Convective_Diffusion_2D,
  
  Diffuse_3D,
  Convective_Diffusion_3D,
};

struct SimuInfo{
  double coorX;
  double xw;
  double xe;
  double cellLenght;
  double labdw;
  double labde;
  double hpw;
  double hpe;
  double areap;
  double areaw;
  double areae;
  double heatCap;
  double density;
};

#typedef std::vector<simuInfo> SimuInfo;
#typedef std::vector< std::vector<double> > SimuRes;

struct LocalParam
{
  double heatCap;
  double density;
};

#typedef std::vector<LocalParam> ZTLocalParam; 

struct ZTGlobalParam
{
  NodeType type;
  double f;
  double Sc;
  double Sp;
  double deltaT;
};

#endif
