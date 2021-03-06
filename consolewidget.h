#ifndef CONSOLEWIDGET_H
#define CONSOLEWIDGET_H

#include<QObject>

class consoleWidget: public QObject
{
  Q_OBJECT
 public:
  static consoleWidget *instance();
  void msg(const char* msg);

 private:
  static consoleWidget *m_pInstance;
  //consoleWidget(QWidget *parent = NULL);
  //~consoleWidget(){};

  /*
 private slots:
  void handleMessage(QtMsgType type, const QString &msg);
  */
  
 signals:
  void message(QtMsgType type, const QString &msg);
};

#endif
