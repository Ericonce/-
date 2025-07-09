#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QDebug>
#include<QTimer>

#include "QStackedWidget"
#include "QButtonGroup"
#include "QHBoxLayout"
#include "QVBoxLayout"
#include "QStringList"

#include "show_sql.h"
#include "usarthelper.h"
#include "dcm1.h"
#include "dcm2.h"
#include "dcm3.h"
#include "dcm4.h"
#include "dcm5.h"
#include "dcm6.h"
#include "dcm7.h"
#include "dcm8.h"
#include "foc1.h"

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void buttonClicked();//左侧导航按钮的槽函数

private:
    void initWidget();
    void initNav();//初始化按钮与退出功能
    void initMain();

    void Loop_Init();
    void Loop_Detection();

private://数据库
    QSqlDatabase getSharedDatabase();

private:
    QHBoxLayout* GreatLayout;

    QWidget* NavWidget;
    QVBoxLayout* NavLayout;
    QButtonGroup* BtnGroup;
    QStringList Button_names;

    int LoopIsOpen = 0;
    int index_widget = 0;
    QTimer *Timer_Loop = nullptr;
    QWidget* MainWidget;
    QVBoxLayout* MainLayout;
    QStackedWidget* StackedWidget;

private:
    show_sql *m_sql;

signals:
    // 定义发送串口指令的信号（参数为要发送的字符串）
    void sendSerialCommand(const QString &cmd);

};
#endif // WIDGET_H
