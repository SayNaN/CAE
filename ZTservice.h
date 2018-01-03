#ifndef ZTSERVICE_H
#define ZTSERVICE_H

#include"ZTtype.h"

class ZTService
{
 public:
  static ZTService* getInstance();

  ZTSimuInfo* info();
  ZTGlobalParam* globalParam();
  ZTLocalParam* localParam();
  ZTRes* res();

 private:
  static ZTService* p_instance;
  ZTMesh* m_Mesh;
  ZTGlobalParam* m_GlobalParam;
  ZTLocalParam* m_LocalParam;
  ZTRes* m_Res;

  ZTService(){};
  ~ZTService(){};

  class ZTServiceGarbo
  {
  public:
    ~ZTServiceGarbo()
      {
	if(NULL != p_instance)
	  {
	    delete p_instance;
	  }
      }
  };
};

#endif
