#include<QWidget>
#include"mainwindow.h"
#include"mesh/mesh_1D.h"
#include"ZTservice.h"
#include"leftwidget.h"
#include"myglwidget.h"
#include"operate.h"
#include"consolewidget.h"
#include"diffusion_1D/algorithm.h"

MainWindow::MainWindow(QWidget *parent):QMainWindow(parent)
{
  m_pService = new ZTService();
  setWindowTitle(tr("一维非稳态导热"));
  inimenu();
  inicentralwidget();
  startNewThread();
}

MainWindow::~MainWindow()
{
  m_oWorkThread.quit();
  m_oWorkThread.wait();
}

void MainWindow::inimenu()
{
  m_pOpenProject=new QAction(tr("打开工程"),this);
  m_pOpenProject->setShortcut(tr("Ctrl+O"));
  m_pOpenProject->setStatusTip(tr("打开工程"));
  m_pOpenProject->setToolTip(tr("打开工程"));
  connect(m_pOpenProject, &QAction::triggered, [this](){
      QString sFilePath;
      sFilePath=QFileDialog::getOpenFileName(this,tr("打开"),QString(),
					     tr("输入文件(*.dat)"));
      if(!sFilePath.isEmpty())
	{
	  m_pService->operate()->setProjectName(sFilePath);
	  m_pService->operate()->readProjectFile();
	  m_pText3D->cleardisplay();
	  m_pLeftWidget->resetGUI();
	  setWindowTitle(m_pService->operate()->getProjectName());
	}
    });

  m_pSaveProject=new QAction(tr("保存工程"),this);
  m_pSaveProject->setShortcut(tr("Ctrl+S"));
  m_pSaveProject->setStatusTip(tr("Save Project"));
  m_pSaveProject->setToolTip(tr("Save Project"));
  connect(m_pSaveProject,&QAction::triggered, [this](){
      if(!m_pService->operate()->canSave())
	{
	  saveAs();
	}
      else
	{
	  m_pService->operate()->writeProjectFile();
	}
    });

  m_pSaveProjectAs=new QAction(tr("另存为"),this);
  m_pSaveProjectAs->setStatusTip(tr("Save Project As"));
  m_pSaveProjectAs->setToolTip(tr("Save ProjectAs"));
  connect(m_pSaveProjectAs,&QAction::triggered, this, &MainWindow::saveAs);

  m_pExitProgram=new QAction(tr("&Exit"),this);
  m_pExitProgram->setShortcut(tr("Ctrl+E"));

  m_pStartRestart=new QAction(tr("开始计算"),this);
  m_pStartRestart->setStatusTip(tr("Start Simulation"));
  m_pStartRestart->setToolTip(tr("Start Simulation"));
  connect(m_pStartRestart, SIGNAL(triggered()), this, SLOT(startSimu()));

  m_pStopSimulation = new QAction(tr("终止计算"),this);

  //文件菜单
  m_pMenuFile=menuBar()->addMenu(tr("&File"));
  m_pMenuFile->addAction(m_pOpenProject);
  m_pMenuFile->addSeparator();            //分割线
  m_pMenuFile->addAction(m_pSaveProject);
  m_pMenuFile->addAction(m_pExitProgram);

  //模拟菜单
  m_pMenuSimulation=menuBar()->addMenu(tr("&Simulation"));
  m_pMenuSimulation->addAction(m_pStartRestart);
}

void MainWindow::initoolbar()
{
  m_pFileToolBar=addToolBar(tr("&File"));
  m_pFileToolBar->addAction(m_pOpenProject);
  m_pFileToolBar->addAction(m_pSaveProject);

  m_pEditToolBar=addToolBar(tr("&Edit"));
  m_pEditToolBar->addAction(m_pStartRestart);
  m_pEditToolBar->addAction(m_pStopSimulation);
}

void MainWindow::inicentralwidget()
{
  QSplitter *pSplitterMain=new QSplitter(Qt::Horizontal,this);
  m_pLeftWidget = new LeftWidget(m_pService, pSplitterMain);
  QSplitter *splitterright=new QSplitter(Qt::Vertical,pSplitterMain);
  m_pText3D = new MyGLWidget(m_pService, splitterright);
  m_pTextConsole = new QPlainTextEdit(tr("控制台\n"),splitterright);
  m_pTextConsole->setReadOnly(true);
  connect(m_pService->console(), &consoleWidget::message, [this](QtMsgType type, const QString &msg){
        QByteArray localMsg = msg.toLocal8Bit();
	switch (type)
	  {
	  case QtDebugMsg:
	    fprintf(stderr, "Debug: %s\n", localMsg.constData());
	    break;
	  case QtInfoMsg:
	    fprintf(stderr, "Info: %s\n", localMsg.constData());
	    break;
	  case QtWarningMsg:
	    fprintf(stderr, "Warning: %s\n", localMsg.constData());
	    break;
	  case QtCriticalMsg:
	    fprintf(stderr, "Critical: %s\n", localMsg.constData());
	    break;
	  case QtFatalMsg:
	    fprintf(stderr, "Fatal: %s\n", localMsg.constData());
	    abort();
	  }
	m_pTextConsole->appendPlainText(msg);
    });

  setCentralWidget(pSplitterMain);
  pSplitterMain->show();
}

void MainWindow::startNewThread()
{
  if(m_oWorkThread.isRunning())
    {
      return;
    }
  qDebug() << "main Thread : " << QThread::currentThreadId();
  diffusion1D *pWork = new diffusion1D(m_pService);
  pWork->moveToThread(&m_oWorkThread);
  connect(&m_oWorkThread, &QThread::finished, pWork, &QObject::deleteLater);
  connect(this, &MainWindow::startRun, pWork, &diffusion1D::doIt);
  connect(pWork, &diffusion1D::oneStepFinished, m_pText3D, &MyGLWidget::processRes, Qt::BlockingQueuedConnection);
  connect(pWork, &diffusion1D::calcFinished, [this](){
      m_pService->operate()->writeResFile("");
    });
  m_oWorkThread.start();
}

void MainWindow::startSimu()
{
  if(!m_pService->canStartSimu())
    {
      qDebug()<<tr("无法开始计算，请检查设置");
      return;
    }
  if(!m_pService->operate()->canSave())
    {
      saveAs();
    }
  m_pText3D->cleardisplay();
  emit startRun();
}

void MainWindow::saveAs()
{
  QString sFilePath;
  sFilePath=QFileDialog::getSaveFileName(this,tr("保存"),QString(),
					 tr("工程文件(*.dat)"));
  
  if(!sFilePath.isEmpty())
    {
      m_pService->operate()->setProjectName(sFilePath);
      m_pService->operate()->writeProjectFile();
      setWindowTitle(m_pService->operate()->getProjectName());
    }
}
