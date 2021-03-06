#include<windows.h>
#include<math.h>
#include<QtWidgets>
#include<GL/GL.H>
#include<GL/GLU.H>
#include<vector>
#include"myglwidget.h"
#include"ZTservice.h"

MyGLWidget::MyGLWidget(ZTService *pService, QWidget *parent):
  QOpenGLWidget(parent),
  m_pService(pService),
  m_dRangeLeft(-1),
  m_dRangeRight(1),
  m_dRangeBottom(-1),
  m_dRangeTop(1),
  m_dRangeNear(-1000),
  m_dRangeFar(1000),
  m_bDrawMesh(true),
  m_bDrawGeo(true)
{
  m_sMaxCoor.x = -1e30;
  m_sMaxCoor.y = -1e30;
  m_sMaxCoor.z = -1e30;
  
  m_sMinCoor.x =  1e30;
  m_sMinCoor.y =  1e30;
  m_sMinCoor.z =  1e30;
}

void MyGLWidget::cleardisplay()
{
  /*
  m_bDrawGeo=false;
  */
  m_bDrawMesh=false;
  resetMyGL();
  update();
}

void MyGLWidget::initShowRange()
{
  if((m_sMaxCoor.x - m_sMinCoor.x)/(m_sMaxCoor.y - m_sMinCoor.y) < m_dWinWidth/m_dWinHeight)
    {
      m_dShowTop    = 1.1*m_sMaxCoor.y - 0.1*m_sMinCoor.y;
      m_dShowBottom = 1.1*m_sMinCoor.y - 0.1*m_sMaxCoor.y;
      m_dShowLeft   = (m_sMaxCoor.x + m_sMinCoor.x)/2.0 - m_dWinWidth/m_dWinHeight*(m_dShowTop-m_dShowBottom)/2.0;
      m_dShowRight  = (m_sMaxCoor.x + m_sMinCoor.x)/2.0 + m_dWinWidth/m_dWinHeight*(m_dShowTop-m_dShowBottom)/2.0;
    }
  else
    {
      m_dShowLeft   = 1.1*m_sMinCoor.x - 0.1*m_sMaxCoor.x;
      m_dShowRight  = 1.1*m_sMaxCoor.x - 0.1*m_sMinCoor.x;
      m_dShowTop    = (m_sMaxCoor.y + m_sMinCoor.y)/2.0 + m_dWinHeight/m_dWinWidth*(m_dShowRight-m_dShowLeft)/2.0;
      m_dShowBottom = (m_sMaxCoor.y + m_sMinCoor.y)/2.0 - m_dWinHeight/m_dWinWidth*(m_dShowRight-m_dShowLeft)/2.0;
    }
}

void MyGLWidget::wheelEvent(QWheelEvent *event)
{
  makeCurrent();
  QPoint eventscale=event->angleDelta();
  double positionx,positiony;
  double newtop,newbottom,newleft,newright;
  double localRatio;
  positionx=(double)event->x() / m_dWinWidth;
  positiony=(double)event->y() / m_dWinHeight;
  if(eventscale.y()>0)
    localRatio=1.25;
  else
    localRatio=0.8;
  if((m_dShowRight - m_dShowLeft)<(m_dShowTop - m_dShowBottom))
    {
      newtop    = m_dShowTop - (m_dShowTop-m_dShowBottom)*positiony + (m_dShowTop-m_dShowBottom)*localRatio* positiony;
      newbottom = m_dShowTop - (m_dShowTop-m_dShowBottom)*positiony - (m_dShowTop-m_dShowBottom)*localRatio*(1-positiony);
      newleft   = m_dShowLeft+ (m_dShowRight-m_dShowLeft)*positionx - (m_dShowTop-m_dShowBottom)*localRatio*positionx    * m_dWinWidth/m_dWinHeight;
      newright  = m_dShowLeft+ (m_dShowRight-m_dShowLeft)*positionx + (m_dShowTop-m_dShowBottom)*localRatio*(1-positionx)* m_dWinWidth/m_dWinHeight;
    }
  else
    {
      newleft   = m_dShowLeft+ (m_dShowRight-m_dShowLeft)*positionx - (m_dShowRight-m_dShowLeft)*localRatio* positionx;
      newright  = m_dShowLeft+ (m_dShowRight-m_dShowLeft)*positionx + (m_dShowRight-m_dShowLeft)*localRatio*(1-positionx);
      newtop    = m_dShowTop - (m_dShowTop-m_dShowBottom)*positiony + (m_dShowRight-m_dShowLeft)*localRatio* positiony   * m_dWinHeight/m_dWinWidth;
      newbottom = m_dShowTop - (m_dShowTop-m_dShowBottom)*positiony - (m_dShowRight-m_dShowLeft)*localRatio*(1-positiony)* m_dWinHeight/m_dWinWidth;
    }
  m_dShowLeft   = newleft;
  m_dShowRight  = newright;
  m_dShowTop    = newtop;
  m_dShowBottom = newbottom;
  qDebug()<<"111"<<m_dShowLeft<<m_dShowRight<<m_dShowTop<<m_dShowBottom;
  update();
}

void MyGLWidget::mousePressEvent(QMouseEvent *e)
{
  switch(e->button())
    {
    case Qt::LeftButton:
      button_type=1;
      break;
    case Qt::MidButton:
      button_type=2;
      break;
    case Qt::RightButton:
      button_type=3;
      break;
    default:
      button_type=0;
      break;
    }
  if(button_type==3)
    {
      makeCurrent();
      initShowRange();
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      update();
    }
  else
    {
      m_nMouseX = e->x();
      m_nMouseY = e->y();
    }
}

void MyGLWidget::mouseMoveEvent(QMouseEvent *event)
{
  if(button_type==1)
    {
      makeCurrent();
      double s=0.5;
      glMatrixMode(GL_MODELVIEW);
      glGetDoublev(GL_MODELVIEW_MATRIX,m_aComat);
      glLoadIdentity();
      glRotatef(s*(event->y() - m_nMouseY), 1., 0., 0.);
      glRotatef(s*(event->x() - m_nMouseX), 0., 1., 0.);
      glMultMatrixd(m_aComat);
      update();
      m_nMouseX = event->x();
      m_nMouseY = event->y();
    }
  if(button_type==2)
    {
      makeCurrent();
      double matP[16];
      double matM[16];
      int portV[16];
      double win[3];
      double obj[3];
      double scale;
      glGetDoublev(GL_PROJECTION_MATRIX,matP);
      glGetDoublev(GL_MODELVIEW_MATRIX,matM);
      glGetIntegerv(GL_VIEWPORT,portV);
      
      gluProject(0,0,0,matM,matP,portV,&win[0],&win[1],&win[2]);
      gluUnProject(win[0],win[1]+1,win[2],matM,matP,portV,&obj[0],&obj[1],&obj[2]);
      
      scale=sqrt(obj[0]*obj[0]+obj[1]*obj[1]+obj[2]*obj[2]);
      glMatrixMode(GL_MODELVIEW);
      glGetDoublev(GL_MODELVIEW_MATRIX, m_aComat);
      glLoadIdentity();
      glTranslatef(scale*(event->x()- m_nMouseX),scale*(m_nMouseY-event->y()), 0.0);
      glMultMatrixd(m_aComat);
      update();
      m_nMouseX = event->x();
      m_nMouseY = event->y();
    }
}

void MyGLWidget::initializeGL()
{
  initializeOpenGLFunctions();

  glClearColor(1.0, 1.0, 1.0, 0.0);
  //glClearDepth(1.0);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glShadeModel(GL_SMOOTH);  
  glDepthFunc(GL_LEQUAL);

  GLfloat LightAmbient[]= { 0.2, 0.2, 0.2, 1.0 };
  GLfloat LightDiffuse[]= { 0.8, 0.8, 0.8, 1.0 };
  GLfloat lightSpecular[] = { 1.0, 1.0, 1.0, 1.0 };
  GLfloat LightPosition[]= { 1.0, 1.0, 1.0, 0.0 };
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);
  glLightfv(GL_LIGHT1, GL_SPECULAR, lightSpecular);
  glLightfv(GL_LIGHT1, GL_POSITION,LightPosition);
  glPopMatrix();
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT,LightAmbient);
  glEnable(GL_LIGHT1);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
  glEnable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
  //glDepthMask(GL_FALSE);
  //glEnable(GL_COLOR_MATERIAL);
}

void MyGLWidget::resizeGL(int w, int h)
{
  m_dWinWidth  = w;
  m_dWinHeight = h;
  glViewport(0, 0, w, h);
  double newtop,newbottom,newleft,newright;
  if((m_dRangeRight-m_dRangeLeft)/(m_dRangeTop-m_dRangeBottom)<m_dWinWidth/m_dWinHeight)
    {
      newtop    = m_dShowTop;
      newbottom = m_dShowBottom;
      newleft   = (m_dShowRight+m_dShowLeft)/2.0-(m_dShowTop-m_dShowBottom)/2.0*m_dWinWidth/m_dWinHeight;
      newright  = (m_dShowRight+m_dShowLeft)/2.0+(m_dShowTop-m_dShowBottom)/2.0*m_dWinWidth/m_dWinHeight;
    }
  else
    {
      newleft   = m_dShowLeft;
      newright  = m_dShowRight;
      newtop    = (m_dShowTop+m_dShowBottom)/2.0+(m_dShowRight-m_dShowLeft)/2.0*m_dWinHeight/m_dWinWidth;
      newbottom = (m_dShowTop+m_dShowBottom)/2.0-(m_dShowRight-m_dShowLeft)/2.0*m_dWinHeight/m_dWinWidth;
    }
  m_dShowLeft   = newleft;
  m_dShowRight  = newright;
  m_dShowTop    = newtop;
  m_dShowBottom = newbottom;
}

void MyGLWidget::paintGL()
{
  glMatrixMode(GL_MODELVIEW);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(m_dShowLeft, m_dShowRight, m_dShowBottom, m_dShowTop, m_dRangeNear, m_dRangeFar);
  glMatrixMode(GL_MODELVIEW);
  glEnable(GL_LIGHTING);

  drawAdditional();
  draw3D();
  
  glDisable(GL_LIGHTING);
  glGetDoublev(GL_MODELVIEW_MATRIX, m_aComat);
  m_aComat[3]=0;
  m_aComat[7]=0;
  m_aComat[11]=0;
  m_aComat[12]=0;
  m_aComat[13]=0;
  m_aComat[14]=0;
  m_aComat[15]=1;
  glPushMatrix();
  glClear(GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(-m_dWinWidth,m_dWinWidth,-m_dWinHeight,m_dWinHeight,-1000,1000);
  glMatrixMode(GL_MODELVIEW);
  glTranslatef(0.7*m_dWinWidth,-0.8*m_dWinHeight,0);
  glMultMatrixd(m_aComat);
  double ra;
  if(m_dWinWidth<m_dWinHeight)
    { ra=m_dWinWidth*0.1; }
  else
    { ra=m_dWinHeight*0.1; }

  glBegin(GL_QUAD_STRIP);
  glColor3f(0.0,0.0,1.0);
  glVertex3f(0.0,ra,0.0);
  glVertex3f(0.0,0.0,0.0);
  glVertex3f(ra,ra,0.0);
  glVertex3f(ra,0.0,0.0);
  glVertex3f(ra,ra,ra);
  glVertex3f(ra,0.0,ra);
  glVertex3f(0.0,ra,ra);
  glVertex3f(0.0,0.0,ra);
  glVertex3f(0.0,ra,0.0);
  glVertex3f(0.0,0.0,0.0);
  glEnd();
  glBegin(GL_QUADS);
  glVertex3f(0.0,ra,0.0);
  glVertex3f(ra,ra,0.0);
  glVertex3f(ra,ra,ra);
  glVertex3f(0.0,ra,ra);
  glEnd();
  glBegin(GL_QUADS);
  glVertex3f(ra,0.0,0.0);
  glVertex3f(0.0,0.0,0.0);
  glVertex3f(0.0,0.0,ra);
  glVertex3f(ra,0.0,ra);
  glEnd();
  glBegin(GL_LINES);
  glColor3f(0.0,0.0,0.0);
  glVertex3f(0.0,0.0,0.0);
  glVertex3f(1.5*ra,0.0,0.0);
  glVertex3f(0.0,0.0,0.0);
  glVertex3f(0.0,1.5*ra,0.0);
  glVertex3f(0.0,0.0,0.0);
  glVertex3f(0.0,0.0,1.5*ra);
  glVertex3f(ra,0.0,0.0);
  glVertex3f(ra,ra,0.0);
  glVertex3f(ra,0.0,0.0);
  glVertex3f(ra,0.0,ra);
  glVertex3f(0.0,ra,0.0);
  glVertex3f(ra,ra,0.0);
  glVertex3f(0.0,ra,0.0);
  glVertex3f(0.0,ra,ra);
  glVertex3f(0.0,0.0,ra);
  glVertex3f(ra,0.0,ra);
  glVertex3f(0.0,0.0,ra);
  glVertex3f(0.0,ra,ra);
  glVertex3f(ra,ra,ra);
  glVertex3f(0.0,ra,ra);
  glVertex3f(ra,ra,ra);
  glVertex3f(ra,0.0,ra);
  glVertex3f(ra,ra,ra);
  glVertex3f(ra,ra,0.0);
  glEnd();
  glRasterPos3f( 1.8*ra,    0.0, 0.0);
  drawstring("X");
  glRasterPos3f(    0.0, 1.8*ra, 0.0);
  drawstring("Y");
  glRasterPos3f(    0.0,    0.0, 1.8*ra);
  drawstring("Z");
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  //glDisable(GL_LIGHTING);
  glFlush();
}

void MyGLWidget::drawstring(const char *str)
{
  int len=0,i;
  wchar_t *wstring;
  HDC hdc=wglGetCurrentDC();
  GLuint list=glGenLists(1);
  for(i=0;str[i]!='\0';i++)
    {
      if(IsDBCSLeadByte(str[i]))
	i++;
      len++;
    }
  wstring=(wchar_t*)malloc((len+1)*sizeof(wchar_t));
  MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,str,-1,wstring,len);
  wstring[len]=L'\0';
  for(i=0;i<len;i++)
    {
      wglUseFontBitmapsW(hdc,wstring[i],1,list);
      glCallList(list);
    }
  free(wstring);
  glDeleteLists(list,1);
}

void MyGLWidget::draw3D()
{
  /*
  GLfloat mat_ambient[]  = {0.8f, 0.0f, 0.0f, 1.0f};
  GLfloat mat_diffuse[]  = {0.4f, 0.1f, 0.1f, 1.0f};
  GLfloat mat_specular[] = {0.3f, 0.1f, 0.1f, 1.0f};
  GLfloat mat_emission[] = {0.0f, 0.0f, 0.0f, 1.0f};
  GLfloat mat_shininess=100.0f;

  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_ambient);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,mat_specular);
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION,mat_emission);
  glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS,mat_shininess);
  */
  GLfloat mat_diffuse[] = {0.0f, 0.0f, 0.0f, 1.0f};
  GLfloat mat_Line[]  = {0.0f, 0.0f, 0.0f, 1.0f};
  
  int aIndex[3] = {0,0,0};
  if(m_bDrawGeo)
    {
      glBegin(GL_TRIANGLES);
      for(unsigned int i=0; i<m_vecIndex.size(); i++)
	{
	  aIndex[0] = m_vecIndex.at(i).fir;
	  aIndex[1] = m_vecIndex.at(i).sec;
	  aIndex[2] = m_vecIndex.at(i).thr;
	  for(int j=0; j<3; j++)
	    {
	      Vector3D *strucPos   = &(m_vecPoint.at(aIndex[j]).point);
	      Vector3D *strucNorm  = &(m_vecPoint.at(aIndex[j]).normalization);
	      Vector4D *strucColor = &(m_vecPoint.at(aIndex[j]).color);
	      mat_diffuse[0] = strucColor->x;
	      mat_diffuse[1] = strucColor->y;
	      mat_diffuse[2] = strucColor->z;
	      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_diffuse);
	      glNormal3f(strucNorm->x, strucNorm->y, strucNorm->z);
	      glVertex3f(strucPos->x, strucPos->y, strucPos->z);
	    }
	}
      glEnd();
    }
  if(m_bDrawMesh)
    {
      glBegin(GL_LINES);
      for(unsigned int i=0; i<m_vecIndex.size(); i++)
	{
	  aIndex[0] = m_vecIndex.at(i).fir;
	  aIndex[1] = m_vecIndex.at(i).sec;
	  aIndex[2] = m_vecIndex.at(i).thr;
	  for(int j=0; j<3; j++)
	    {
	      Vector3D *strucFir  = &(m_vecPoint.at(aIndex[j]).point);
	      Vector3D *strucSec  = &(m_vecPoint.at(aIndex[(j+1)%3]).point);
	      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_Line);
	      glVertex3f(strucFir->x, strucFir->y, strucFir->z);
	      glVertex3f(strucSec->x, strucSec->y, strucSec->z);
	    }
	}
      glEnd();
    }
}

void MyGLWidget::resetMyGL()
{
  m_sMaxCoor.x = -1e30;
  m_sMaxCoor.y = -1e30;
  m_sMaxCoor.z = -1e30;

  m_sMinCoor.x = 1e30;
  m_sMinCoor.y = 1e30;
  m_sMinCoor.z = 1e30;

  m_mapPointIndex.clear();
  m_vecPoint.clear();
  m_vecIndex.clear();
}

void MyGLWidget::drawAdditional()
{
  glBegin(GL_LINES);
  glColor3f(0.0, 0.0, 0.0);
  glVertex3f(m_sMinCoor.x,                  m_sMinCoor.y, m_sMinCoor.z);
  glVertex3f(m_sMaxCoor.x+0.03*m_dWinWidth, m_sMinCoor.y, m_sMinCoor.z);

  glVertex3f(m_sMinCoor.x, m_sMinCoor.y,                  m_sMinCoor.z);
  glVertex3f(m_sMinCoor.x, m_sMaxCoor.y+0.03*m_dWinWidth, m_sMinCoor.z);

  glVertex3f(m_sMinCoor.x, m_sMinCoor.y, m_sMinCoor.z);
  glVertex3f(m_sMinCoor.x, m_sMinCoor.y, m_sMaxCoor.z+0.03*m_dWinWidth);
  glEnd();

  glRasterPos3f(m_sMaxCoor.x+0.04*m_dWinWidth, m_sMinCoor.y,                  m_sMinCoor.z);
  drawstring("X(m)");
  glRasterPos3f(m_sMinCoor.x,                  m_sMaxCoor.y+0.03*m_dWinWidth, m_sMinCoor.z);
  drawstring("T(K)");
  glRasterPos3f(m_sMinCoor.x,                  m_sMinCoor.y,                  m_sMaxCoor.z+0.03*m_dWinWidth);
  drawstring("Time(s)");

}

void MyGLWidget::processMaxMin(int nIndex)
{
  MeshRes* pMeshRes = m_pService->meshRes();
  for(unsigned int i=0; i<pMeshRes->size()-1; i++)
    {
      if(m_sMaxCoor.x < pMeshRes->at(i).dCoorX)
	{
	  m_sMaxCoor.x = pMeshRes->at(i).dCoorX;
	}
      if(m_sMaxCoor.y < pMeshRes->at(i).vecTime_Temperature.at(nIndex).second)
	{
	  m_sMaxCoor.y = pMeshRes->at(i).vecTime_Temperature.at(nIndex).second;
	}
      if(m_sMaxCoor.z < pMeshRes->at(i).vecTime_Temperature.at(nIndex).first)
	{
	  m_sMaxCoor.z = pMeshRes->at(i).vecTime_Temperature.at(nIndex).first;
	}

      if(m_sMinCoor.x > pMeshRes->at(i).dCoorX)
	{
	  m_sMinCoor.x = pMeshRes->at(i).dCoorX;
	}
      if(m_sMinCoor.y > pMeshRes->at(i).vecTime_Temperature.at(nIndex).second)
	{
	  m_sMinCoor.y = pMeshRes->at(i).vecTime_Temperature.at(nIndex).second;
	}
      if(m_sMinCoor.z > pMeshRes->at(i).vecTime_Temperature.at(nIndex).first)
	{
	  m_sMinCoor.z = pMeshRes->at(i).vecTime_Temperature.at(nIndex).first;
	}
    }
}

void MyGLWidget::processRes(int nIndex)
{
  processMaxMin(nIndex);
  if(nIndex > 0)
    {
      MeshRes* pMeshRes = m_pService->meshRes();
      for(unsigned int i=0; i<pMeshRes->size()-1; i++)
	{
	  Vector3D strucVer[3];
	  strucVer[0].x = pMeshRes->at(i).dCoorX;
	  strucVer[0].y = pMeshRes->at(i).vecTime_Temperature.at(nIndex-1).second;
	  strucVer[0].z = pMeshRes->at(i).vecTime_Temperature.at(nIndex-1).first;

	  strucVer[1].x = pMeshRes->at(i).dCoorX;
	  strucVer[1].y = pMeshRes->at(i).vecTime_Temperature.at(nIndex).second;
	  strucVer[1].z = pMeshRes->at(i).vecTime_Temperature.at(nIndex).first;
	  
	  strucVer[2].x = pMeshRes->at(i+1).dCoorX;
	  strucVer[2].y = pMeshRes->at(i+1).vecTime_Temperature.at(nIndex-1).second;
	  strucVer[2].z = pMeshRes->at(i+1).vecTime_Temperature.at(nIndex-1).first;
	  
	  addTrangle(strucVer);
	  
	  strucVer[0].x = pMeshRes->at(i+1).dCoorX;
	  strucVer[0].y = pMeshRes->at(i+1).vecTime_Temperature.at(nIndex-1).second;
	  strucVer[0].z = pMeshRes->at(i+1).vecTime_Temperature.at(nIndex-1).first;

	  strucVer[1].x = pMeshRes->at(i).dCoorX;
	  strucVer[1].y = pMeshRes->at(i).vecTime_Temperature.at(nIndex).second;
	  strucVer[1].z = pMeshRes->at(i).vecTime_Temperature.at(nIndex).first;

	  strucVer[2].x = pMeshRes->at(i+1).dCoorX;
	  strucVer[2].y = pMeshRes->at(i+1).vecTime_Temperature.at(nIndex).second;
	  strucVer[2].z = pMeshRes->at(i+1).vecTime_Temperature.at(nIndex).first;

	  addTrangle(strucVer);
	}
    }
  reCalcColor();
  update();
}

void MyGLWidget::reCalcColor()
{
  int nCount = m_vecPoint.size();
  double temperatureDiff = m_sMaxCoor.y - m_sMinCoor.y;
  double colorDiffR = maxColor.x - minColor.x;
  double colorDiffG = maxColor.y - minColor.y;
  double colorDiffB = maxColor.z - minColor.z;
  for(int i=0; i<nCount; i++)
    {
      double currentTemperature = m_vecPoint.at(i).point.y;
	  
      m_vecPoint.at(i).color.x = (currentTemperature - m_sMinCoor.y)/temperatureDiff*colorDiffR + minColor.x;
      m_vecPoint.at(i).color.y = (currentTemperature - m_sMinCoor.y)/temperatureDiff*colorDiffG + minColor.y;
      m_vecPoint.at(i).color.z = (currentTemperature - m_sMinCoor.y)/temperatureDiff*colorDiffB + minColor.z;
      m_vecPoint.at(i).color.a = 1.0f;
    }
}

void MyGLWidget::getnormaldir(Vector3D oFir, Vector3D oSec, Vector3D oThr, Vector3D &oNormal)
{
  Vector3D oTmp1,oTmp2;
  oTmp1.x = oSec.x - oFir.x;
  oTmp1.y = oSec.y - oFir.y;
  oTmp1.z = oSec.z - oFir.z;

  oTmp2.x = oThr.x - oSec.x;
  oTmp2.y = oThr.y - oSec.y;
  oTmp2.z = oThr.z - oSec.z;

  corss(oTmp1, oTmp2, oNormal);
}

void MyGLWidget::corss(Vector3D oVec1, Vector3D oVec2, Vector3D &oNormal)
{
  oNormal.x = oVec1.y*oVec2.z - oVec1.z*oVec2.y;
  oNormal.y = oVec1.z*oVec2.x - oVec1.x*oVec2.z;
  oNormal.z = oVec1.x*oVec2.y - oVec1.y*oVec2.x;
}

void MyGLWidget::guiyi(Vector3D oSrc, Vector3D &oDes)
{
  double dLength = sqrt(oSrc.x*oSrc.x + oSrc.y*oSrc.y + oSrc.z*oSrc.z);

  oDes.x = oSrc.x / dLength;
  oDes.y = oSrc.y / dLength;
  oDes.z = oSrc.z / dLength;
}

void MyGLWidget::addTrangle(Vector3D strucVer[3])
{
    Vector3D strucNorm;
    QString strKey;
    int aIndex[3];

    getnormaldir(strucVer[0], strucVer[1], strucVer[2], strucNorm);
    
    for(int j=0; j<3; j++)
      {
	strKey = QString::number(strucVer[j].x) + " " + QString::number(strucVer[j].y) + " " + QString::number(strucVer[j].z);
	
	std::map<QString, unsigned int>::iterator itMap = m_mapPointIndex.find(strKey);
	
	if(itMap == m_mapPointIndex.end())
	  {
	    GLData oTmp;
	    oTmp.point.x = strucVer[j].x;
	    oTmp.point.y = strucVer[j].y;
	    oTmp.point.z = strucVer[j].z;
	    
	    oTmp.normal.x = strucNorm.x;
	    oTmp.normal.y = strucNorm.y;
	    oTmp.normal.z = strucNorm.z;
	    
	    guiyi(oTmp.normal, oTmp.normalization);

	    m_vecPoint.push_back(oTmp);
	    m_mapPointIndex[strKey] = m_vecPoint.size()-1;
	    aIndex[j] = m_vecPoint.size()-1;
	  }
	else
	  {
	    int nMapIndex = itMap->second;
	    
	    m_vecPoint.at(nMapIndex).normal.x += strucNorm.x;
	    m_vecPoint.at(nMapIndex).normal.y += strucNorm.y;
	    m_vecPoint.at(nMapIndex).normal.z += strucNorm.z;
	    
	    guiyi(m_vecPoint.at(nMapIndex).normal, m_vecPoint.at(nMapIndex).normalization);
	    aIndex[j] = nMapIndex;
	  }
      }
    Vector3N oVector3N;
    oVector3N.fir = aIndex[0];
    oVector3N.sec = aIndex[1];
    oVector3N.thr = aIndex[2];
    m_vecIndex.push_back(oVector3N);
}
