#include<QWidget>
#include<QFile>
#include<QDebug>
#include<QTextStream>
#include<QObject>

#include"operate.h"
#include"ZTservice.h"

Operate::Operate(ZTService *pService):
  QObject(),
  m_pService(pService),
  m_sFullProjectPath(""),
  m_sProjectPath(""),
  m_sProjectName(""),
  m_sResFileName("")
{
}

void Operate::resetProject()
{
  m_sFullProjectPath = "";
  m_sProjectPath = "";
  m_sProjectName = "";
  m_sResFileName = "";
}

bool Operate::canSave()
{
  if(m_sFullProjectPath == "")
    {
      return false;
    }
  writeProjectFile();
  return true;
}

void Operate::setProjectName(QString sText)
{
  m_sFullProjectPath = sText;
  m_sProjectPath = sText.section("/",0,-2)+tr("/");
  m_sProjectName = sText.section("/",-1,-1).section(".",0,0);
}

QString Operate::getProjectName()
{
  return m_sProjectName;
}

void Operate::writeProjectFile()
{
  GlobalParam* pGlobalTmp = m_pService->globalParam();
  LocalParam* pLocalTmp = m_pService->localParam();
  
  QFile file(m_sFullProjectPath);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
      qInfo()<<tr("无法打开工程文件");
      return;
    }

  QTextStream out(&file);
  out << pGlobalTmp->nType << endl;
  out << pGlobalTmp->dF << endl;
  out << pGlobalTmp->dSc << endl;
  out << pGlobalTmp->dSp << endl;
  out << pGlobalTmp->dDeltaT << endl;       // time step
  out << pGlobalTmp->dInitTemperature << endl;
  out << pGlobalTmp->nTimeStep << endl;
  out << pGlobalTmp->nSegNum << endl;
  out << pGlobalTmp->nMeshNum << endl;
  out << pGlobalTmp->eInletType << endl;
  out << pGlobalTmp->dInletTemp << endl;
  out << pGlobalTmp->dInletHeatFlux << endl;
  out << pGlobalTmp->dInletInfinityTemp << endl;
  out << pGlobalTmp->dInletConvectiveCoeff << endl;
  out << pGlobalTmp->eOutletType << endl;
  out << pGlobalTmp->dOutletTemp << endl;
  out << pGlobalTmp->dOutletHeatFlux << endl;
  out << pGlobalTmp->dOutletInfinityTemp << endl;
  out << pGlobalTmp->dOutletConvectiveCoeff << endl;

  out << pLocalTmp->size() << endl;

  for(unsigned int i=0; i<pLocalTmp->size(); i++)
    {
      out << pLocalTmp->at(i).dLength << endl;
      out << pLocalTmp->at(i).nSubMeshNum << endl;
      out << pLocalTmp->at(i).dStartP << endl;
      out << pLocalTmp->at(i).dHeatCap << endl;
      out << pLocalTmp->at(i).dDensity << endl;
      out << pLocalTmp->at(i).dThermalConductivity << endl;
      out << pLocalTmp->at(i).dArea << endl;
    }
  file.close();
  
  qInfo()<<tr("工程文件保存完毕!");
}


void Operate::readProjectFile()
{
  m_pService->resetService();
  GlobalParam* pGlobalTmp = m_pService->globalParam();
  LocalParam* pLocalTmp = m_pService->localParam();
  int nTmp;

  QFile file(m_sFullProjectPath);

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      qInfo()<<tr("无法打开工程文件");
      return;
    }

  QTextStream in(&file);
  in >> nTmp;
  pGlobalTmp->nType = (NodeType)nTmp;
  in >> pGlobalTmp->dF;
  in >> pGlobalTmp->dSc;
  in >> pGlobalTmp->dSp;
  in >> pGlobalTmp->dDeltaT;
  in >> pGlobalTmp->dInitTemperature;
  in >> pGlobalTmp->nTimeStep;
  in >> pGlobalTmp->nSegNum;
  in >> pGlobalTmp->nMeshNum;
  in >> nTmp;
  pGlobalTmp->eInletType = (BoundaryConditionType)nTmp;
  in >> pGlobalTmp->dInletTemp;
  in >> pGlobalTmp->dInletHeatFlux;
  in >> pGlobalTmp->dInletInfinityTemp;
  in >> pGlobalTmp->dInletConvectiveCoeff;
  in >> nTmp;
  pGlobalTmp->eOutletType = (BoundaryConditionType)nTmp;
  in >> pGlobalTmp->dOutletTemp;
  in >> pGlobalTmp->dOutletHeatFlux;
  in >> pGlobalTmp->dOutletInfinityTemp;
  in >> pGlobalTmp->dOutletConvectiveCoeff;

  int nCount;
  
  in >> nCount;

  pLocalTmp->clear();
  for(int i=0; i<nCount; i++)
    {
      TabCellContent oStructTemp;
      in >> oStructTemp.dLength;
      in >> oStructTemp.nSubMeshNum;
      in >> oStructTemp.dStartP;
      in >> oStructTemp.dHeatCap;
      in >> oStructTemp.dDensity;
      in >> oStructTemp.dThermalConductivity;
      in >> oStructTemp.dArea;

      pLocalTmp->push_back(oStructTemp);
    }

  file.close();
  qInfo()<<tr("工程文件读取完毕!");
  m_pService->modified(false);
}

void Operate::writeResFile(QString sText)
{
  sText = m_sProjectPath+"d1Dres.dat";
  qDebug()<<sText;

  MeshRes* pMRTmp = m_pService->meshRes();

  QFile file(sText);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
      qInfo()<<tr("无法打开结果文件");
      return;
    }

  QTextStream out(&file);
  out << "variables =\"x\",\"Temperature\",\"Time\"" << endl;
  out << "ZONE I=" <<pMRTmp->size()<<", J="<< pMRTmp->at(0).vecTime_Temperature.size()<<", F=POINT"<<endl;

  for(unsigned int i=0; i<pMRTmp->size(); i++)
    {
      for(unsigned int j=0; j<pMRTmp->at(i).vecTime_Temperature.size(); j++)
	{
	  out << pMRTmp->at(i).dCoorX << " "
	      << pMRTmp->at(i).vecTime_Temperature.at(j).second << " "
	      << pMRTmp->at(i).vecTime_Temperature.at(j).first  << endl;

	}
    }
  file.close();
  qInfo()<<tr("结果文件输出完毕!");
  m_pService->modified(false);
}

bool Operate::writeTecFile(QString sFileName, QString sHeaderText, int nRowCount, double *x, double *y)
{
  QFile file(sFileName);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
      qInfo()<<tr("无法打开结果文件");
      return false;
    }
  QTextStream out(&file);
  out << sHeaderText;
  for(int i=0; i<nRowCount; i++)
    {
      out << x[i] <<y[i];
      out << endl;
    }
  file.close();
}

// **x和**y在传入前应该置空
bool Operate::readTecFile(QString sFileName, double **x, double **y, int nCount)
{
  QFile file(sFileName);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
      qInfo()<<tr("无法打开结果文件");
      return false;
    }
  int nIndex = -1;
  nCount = -1;
  QTextStream in(&file);
  /*
  while(!in.atEnd())
    {
      QString sTmp = in.readLine().trimmed();
      QStringList oList = sTmp.split(" ");
      bool bIsDouble;
      oList[0].toDouble(bIsDouble);
      if(bIsDouble)
	{
	  if(-1 == nCount || (-1 != nCount && nCount <= nIndex))
	    {
	      return false;
	    }
	  else
	    {
	      nIndex++;
	      //(*x)[];
	    }
	}
    }
  */
}
