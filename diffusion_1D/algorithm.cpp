#include<stdio.h>
#include<stdlib.h>
#include<QtWidgets>
#include<vector>
#include<QVector3D>
#include<stdio.h>
#include<stdlib.h>

#include"ZTtype.h"
#include"ZTservice.h"
#include"algorithm.h"
#include"mesh/mesh_1D.h"
#include"linear_equation/numericalanalysis.h"

diffusion1D::diffusion1D(ZTService *pService, QObject* parent)
  :QObject(parent),
  m_pService(pService),
  m_aW(NULL),
  m_aE(NULL),
  m_aP0(NULL),
  m_aP(NULL),
  m_aP0A(NULL),
  m_aL(NULL),
  m_aArray(NULL),
  m_bArray(NULL),
  m_cArray(NULL),
  m_xArray(NULL),
  m_fArray(NULL)
{
  m_pMesher = new Mesh1D(pService);
  m_pData = m_pService->meshRes();
  m_pLocalParam = m_pService->localParam();
  m_pGlobalParam = m_pService->globalParam();
}

diffusion1D::~diffusion1D()
{
  freeArray();
}

void diffusion1D::assignMesh()
{
  int nIndex = 0;
  int nSegCount = m_pLocalParam->size();
  int nSubCount = 0;
  
  m_pData->at(0).dAreap = 1;
  m_pData->at(0).dAreaw = 1;
  m_pData->at(0).dAreae = 1;
  m_pData->at(0).dLabdw = 0;
  m_pData->at(0).dLabde = m_pLocalParam->at(0).dThermalConductivity;
  m_pData->at(0).dHeatCapMulDensity = m_pLocalParam->at(0).dDensity
                                    * m_pLocalParam->at(0).dHeatCap;

  for(int i=0; i<nSegCount; i++)
    {
      nSubCount = m_pLocalParam->at(i).nSubMeshNum;
      for(int j=0; j<nSubCount; j++)
	{
	  nIndex++;
	  m_pData->at(nIndex).dAreap = 1;
	  m_pData->at(nIndex).dAreaw = 1;
	  m_pData->at(nIndex).dAreae = 1;
	  m_pData->at(nIndex).dLabdw = m_pLocalParam->at(i).dThermalConductivity;
	  m_pData->at(nIndex).dLabde = m_pLocalParam->at(i).dThermalConductivity;
	  m_pData->at(nIndex).dHeatCapMulDensity = m_pLocalParam->at(i).dDensity
	                                         * m_pLocalParam->at(i).dHeatCap;
	}
    }
  nIndex++;
  if(CELLCENTER == m_pGlobalParam->nType)
    {
      m_pData->at(nIndex).dAreap = 1;
      m_pData->at(nIndex).dAreaw = 1;
      m_pData->at(nIndex).dAreae = 1;
      m_pData->at(nIndex).dLabdw = m_pLocalParam->at(nSegCount-1).dThermalConductivity;
      m_pData->at(nIndex).dLabde = 0;
      m_pData->at(nIndex).dHeatCapMulDensity = m_pLocalParam->at(nSegCount-1).dDensity
	                                     * m_pLocalParam->at(nSegCount-1).dHeatCap;
    }

  if(CELLCENTER == m_pGlobalParam->nType)
    {
      double dLabdp = 0;
      double dLabde = 0;
      double dXPlus = 0;
      double dXMinus = 0;
      nIndex = 0;
      for(int i=0; i<nSegCount-1; i++)
	{
	  nSubCount = m_pLocalParam->at(i).nSubMeshNum;
	  nIndex += nSubCount;

	  dLabdp  = m_pData->at(nIndex  ).dLabdw;
	  dLabde  = m_pData->at(nIndex+1).dLabdw;
	  dXPlus  = m_pData->at(nIndex  ).dCellLenght / 2.0;
	  dXMinus = m_pData->at(nIndex+1).dCellLenght / 2.0;
	  
	  m_pData->at(nIndex).dLabde = (dLabdp * dLabde)*(dXPlus + dXMinus)/
	    (dLabde * dXMinus + dLabdp * dXPlus);
	  m_pData->at(nIndex+1).dLabdw = m_pData->at(nIndex).dLabde;
	}
    }
  else
    {
      double dRouCw = 0;
      double dRouCe = 0;
      double dXw = 0;
      double dXe = 0;
      double dCellL = 0;
      nIndex = 0;
      for(int i=0; i<nSegCount-1; i++)
	{
	  nSubCount = m_pLocalParam->at(i).nSubMeshNum;
	  nIndex += nSubCount;
	  
	  dRouCw = m_pData->at(nIndex  ).dHeatCapMulDensity;
	  dRouCe = m_pData->at(nIndex+1).dHeatCapMulDensity;
	  dXw    = m_pData->at(nIndex  ).dXw;
	  dXe    = m_pData->at(nIndex  ).dXe;
	  dCellL = m_pData->at(nIndex  ).dCellLenght;
	  
	  m_pData->at(nIndex).dHeatCapMulDensity = (dRouCw * dXw + dRouCe * dXe) / dCellL;
	}
    }
  m_pService->info("网格节点赋值完成");
}

void diffusion1D::doIt()
{
  m_pMesher->run();
  initArray();
  assignMesh();
  calcaCoff();
  assignBeginningField();
  startRun();
  emit calcFinished();
  printf("diffusion1d end\n");
}

void diffusion1D::freeAndNil(double* array)
{
  if(NULL != array)
    {
      free(array);
    }
  array = NULL;
}

void diffusion1D::freeArray()
{
  freeAndNil(m_aW);
  freeAndNil(m_aE);
  freeAndNil(m_aP0);
  freeAndNil(m_aP);
  freeAndNil(m_aP0A);
  freeAndNil(m_aL);
  freeAndNil(m_aArray);
  freeAndNil(m_bArray);
  freeAndNil(m_cArray);
  freeAndNil(m_xArray);
  freeAndNil(m_fArray);
}

void diffusion1D::initArray()
{
  freeArray();
  m_meshNode = m_pData->size();
  char strTmp[80];
  sprintf(strTmp, "meshnode = %d", m_meshNode);
  m_pService->info(strTmp);
  m_aW       = (double*)malloc(sizeof(double)*m_meshNode);
  m_aE       = (double*)malloc(sizeof(double)*m_meshNode);
  m_aP0      = (double*)malloc(sizeof(double)*m_meshNode);
  m_aP       = (double*)malloc(sizeof(double)*m_meshNode);
  m_aP0A     = (double*)malloc(sizeof(double)*m_meshNode);
  m_aL       = (double*)malloc(sizeof(double)*m_meshNode);
  m_aArray   = (double*)malloc(sizeof(double)*m_meshNode);
  m_bArray   = (double*)malloc(sizeof(double)*m_meshNode);
  m_cArray   = (double*)malloc(sizeof(double)*m_meshNode);
  m_xArray   = (double*)malloc(sizeof(double)*m_meshNode);
  m_fArray   = (double*)malloc(sizeof(double)*m_meshNode);
}

void diffusion1D::calcaCoff()
{
  double deltaT = m_pGlobalParam->dDeltaT;
  double f      = m_pGlobalParam->dF;
  double Sp     = m_pGlobalParam->dSp;
  double Sc     = m_pGlobalParam->dSc;
  for(int i=1; i<m_meshNode-1; i++)
    {
      SimuInfoRes* pCache = &(m_pData->at(i));
      m_aE[i] = pCache->dLabde * pCache->dAreae / pCache->dXe;

      m_aW[i] = pCache->dLabdw * pCache->dAreaw / pCache->dXw;

      m_aP0[i] = pCache->dHeatCapMulDensity * pCache->dAreap * pCache->dCellLenght / deltaT;

      m_aP[i] = f*m_aE[i] + f*m_aW[i] + m_aP0[i] - f*Sp* pCache->dAreap * pCache->dCellLenght;

      m_aP0A[i] = -(1-f)*(m_aE[i] + m_aW[i] - Sp* pCache->dAreap * pCache->dCellLenght); 

      m_aL[i] = Sc* pCache->dAreap * pCache->dCellLenght;
    }
  m_pService->info("网格节点系数计算完成");
}

void diffusion1D::assignBeginningField()
{
  for(int i=0; i<m_meshNode; i++)
    {
      m_pData->at(i).vecTime_Temperature.clear();
      m_pData->at(i).vecTime_Temperature.push_back(std::pair<double,double>(0, m_pGlobalParam->dInitTemperature));
    }
  emit oneStepFinished(0);
}

void diffusion1D::startRun()
{
  double f = m_pGlobalParam->dF;
  switch(m_pGlobalParam->eInletType)
    {
    case FirstClass:
      m_bArray[0] = 1;
      m_cArray[0] = 0;
      m_fArray[0] = m_pGlobalParam->dInletTemp;
      break;
    case SecondClass:
      m_bArray[0] = 1;
      m_cArray[0] = -1;
      if(CELLCENTER == m_pGlobalParam->nType)
	{
	  m_fArray[0] = 2 * m_pGlobalParam->dInletHeatFlux * m_pData->at(0).dCellLenght / m_pData->at(0).dLabde;
	}
      else
	{
	  m_fArray[0] = 2 * ( pow(m_pData->at(0).dCellLenght,2) * m_pData->at(0).dAreap +
			      m_pGlobalParam->dInletHeatFlux * m_pData->at(0).dCellLenght)
	    / m_pData->at(0).dLabde;
	}
      break;
    case ThirdClass:
      m_bArray[0] = 1;
      m_cArray[0] = -1 - 2 * m_pGlobalParam->dInletConvectiveCoeff * m_pData->at(0).dCellLenght
	/ m_pData->at(0).dLabde;
      if(CELLCENTER == m_pGlobalParam->nType)
	{
	  m_fArray[0] =-2 * m_pGlobalParam->dInletConvectiveCoeff *
	    m_pData->at(0).dCellLenght * m_pGlobalParam->dInletInfinityTemp
	    / m_pData->at(0).dLabde / m_cArray[0];
	}
      else
	{
	  m_fArray[0] =-2 * ( pow(m_pData->at(0).dCellLenght,2) * m_pData->at(0).dAreap +
			      m_pGlobalParam->dInletConvectiveCoeff *
			      m_pData->at(0).dCellLenght * m_pGlobalParam->dInletInfinityTemp)
	    / m_pData->at(0).dLabde / m_cArray[0];
	}
      break;
    }
  m_pService->info("进口边界条件初始化完成");
  int z = m_meshNode - 1;
  qDebug()<<"in"<<m_pGlobalParam->eInletType;
  qDebug()<<"out"<<m_pGlobalParam->eOutletType;
  switch(m_pGlobalParam->eOutletType)
    {
    case FirstClass:
      m_aArray[z] = 0;
      m_bArray[z] = 1;
      m_fArray[z] = m_pGlobalParam->dOutletTemp;
      break;
    case SecondClass:
      m_aArray[z] = -1;
      m_bArray[z] = 1;
      if(CELLCENTER == m_pGlobalParam->nType)
	{
	  m_fArray[z] = 2 * m_pGlobalParam->dOutletHeatFlux * m_pData->at(z).dCellLenght / m_pData->at(z).dLabdw;
	}
      else
	{
	  m_fArray[z] = 2 * ( pow(m_pData->at(z).dCellLenght,2) * m_pData->at(z).dAreap +
			      m_pGlobalParam->dOutletHeatFlux * m_pData->at(z).dCellLenght)
	    / m_pData->at(z).dLabdw;
	}
      break;
    case ThirdClass:
      m_aArray[z] = -1 - 2 * m_pGlobalParam->dOutletConvectiveCoeff * m_pData->at(z).dCellLenght
	/ m_pData->at(z).dLabdw;
      m_bArray[z] = 1;
      if(CELLCENTER == m_pGlobalParam->nType)
	{
	  m_fArray[z] =-2 * m_pGlobalParam->dOutletConvectiveCoeff *
	    m_pData->at(z).dCellLenght * m_pGlobalParam->dOutletInfinityTemp
	    / m_pData->at(z).dLabdw / m_aArray[z];
	}
      else
	{
	  m_fArray[z] =-2 * ( pow(m_pData->at(z).dCellLenght,2) * m_pData->at(z).dAreap +
			      m_pGlobalParam->dOutletConvectiveCoeff *
			      m_pData->at(z).dCellLenght * m_pGlobalParam->dOutletInfinityTemp)
	    / m_pData->at(z).dLabdw / m_aArray[z];
	}
      break;
    }
  m_pService->info("出口边界条件初始化完成");
  for(unsigned int i=1; i<m_pGlobalParam->nTimeStep; i++)
    {
      for(int j=1; j<m_meshNode-1; j++)
	{
	  m_aArray[j] = -m_aW[j] * f;
	  m_bArray[j] = m_aP[j];
	  m_cArray[j] = -m_aE[j] * f;
	  m_fArray[j] = (1-f)*
	    (m_aE[j]*m_pData->at(j+1).vecTime_Temperature.at(i-1).second +
	     m_aW[j]*m_pData->at(j-1).vecTime_Temperature.at(i-1).second)+
	    m_pData->at(j).vecTime_Temperature.at(i-1).second * (m_aP0[j]+m_aP0A[j]) +
	    m_aL[j];
	}

      ZTChasingMethod(m_aArray, m_bArray, m_cArray, m_fArray, m_xArray, m_meshNode);

      for(int j=0; j<m_meshNode; j++)
	{
	  m_pData->at(j).
	    vecTime_Temperature.push_back(std::pair<double,double>(m_pGlobalParam->dDeltaT*i, m_xArray[j]));
	}
      //qDebug()<<"pso:-1"<<"time:"<<m_pGlobalParam->dDeltaT*i<<"temp:"<<m_xArray[m_meshNode-1];
      emit oneStepFinished(i);
    }
}

void diffusion1D::flushBufToConsole()
{
  m_pService->info(m_oBuf);
}
