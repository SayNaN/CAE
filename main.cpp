/*                 _ooOoo_ 
                  o8888888o 
                  88" . "88 
                  (| -_- |) 
                   O\ = /O 
               ____/`---*\____ 
                .  \\| |// `. 
              / \\||| : |||// \ 
            / _||||| -:- |||||- \ 
              | | \\\ - /// | | 
            | \_| .\\---//. | | 
            \  .-\__ `-` ___/-. / 
          ___`. .* /--.--\ `. . __ 
       ."" *< `.___\_<|>_/___.* >*"". 
      | | : `- \`.;`\ _ /`;.`/ - ` : | | 
        \ \ `-. \_ __\ /__ _/ .-` / / 
======`-.____`-.___\_____/___.-`____.-====== 
............................................. 
*/               

#include<QtWidgets>
#include"mainwindow.h"
#include"ZTservice.h"
#include"consolewidget.h"

static void msgHandlerFunction(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context);
    QMetaObject::invokeMethod(consoleWidget::instance(), "message", Q_ARG(QtMsgType,type),Q_ARG(QString, msg));
}

int main(int argc,char * argv[])
{
  QApplication app(argc,argv);
  qRegisterMetaType<QtMsgType>("QtMsgType");
  qInstallMessageHandler(msgHandlerFunction);
  MainWindow main;
  main.setWindowState(Qt::WindowMaximized);
  main.show();
  return app.exec();
}
