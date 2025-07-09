#include "widget.h"
#include <QDebug>
#include "QTimer"
#include <QCoreApplication>

#include "QHBoxLayout"
#include "QVBoxLayout"
#include "QPushButton"
#include "QLabel"
#include "diagnosiswindow.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    m_sql = new show_sql;
    // if (m_sql->Dataloadfromcsv("C:/Users/lengyan/Desktop/dcm1data.csv")) {
    //     qDebug() << "CSV导入成功";
    // } else {
    //     qDebug() << "CSV导入失败";
    // }

    // if (m_sql->Dataloadfromcsv("C:/Users/lengyan/Desktop/foc.csv")) {
    //     qDebug() << "CSV导入成功";
    // } else {
    //     qDebug() << "CSV导入失败";
    // }

    initWidget();
    initNav();
    initMain();
}

Widget::~Widget() {}

void Widget::initWidget()
{
    //this->setFixedSize(1920,1080);
    this->setStyleSheet("background-color: #FOF3F8;");  // 浅绿
    this->resize(1920,1080);
    this->setWindowTitle("电机状态检测与数据分析上位机系统");
    qDebug() << "主窗口固定尺寸:" << this->size();
    //1.整体采用水平布局
    GreatLayout = new QHBoxLayout(this);
    GreatLayout->setSpacing(0);//子控件之间的间距为0像素
    GreatLayout->setContentsMargins(0,0,0,0);//内容与父容器之间的间距
    qDebug() << "创建horizontalLayout后父窗口尺寸:" << this->size();

    //2.左侧导航窗体
    NavWidget = new QWidget(this);
    NavLayout = new QVBoxLayout(NavWidget);
    NavLayout->setSpacing(0);
    NavLayout->setContentsMargins(0,0,0,0);
    qDebug() << "NavLayout初始尺寸:" << NavLayout->sizeHint() << "最小尺寸:" << NavLayout->minimumSize();

    //3.右侧主体布局-子界面
    MainWidget = new QWidget(this);
    MainLayout = new QVBoxLayout(MainWidget);
    MainLayout->setSpacing(0);
    MainLayout->setContentsMargins(0,0,0,0);
    qDebug() << "MainLayout初始尺寸:" << MainLayout->sizeHint() << "最小尺寸:" << MainLayout->minimumSize();

    GreatLayout->addWidget(NavWidget);
    GreatLayout->addWidget(MainWidget, 3); // 右侧占3份宽度
    qDebug() << "添加左右组件后horizontalLayout尺寸约束:" << GreatLayout->sizeConstraint();


    StackedWidget = new QStackedWidget(MainWidget);
    qDebug() << "StackedWidget初始尺寸:" << StackedWidget->size()
             << "sizePolicy:" << StackedWidget->sizePolicy();
}

void Widget::initNav()
{
    Button_names<<"自动检测模式"
        <<"有刷电机1"
        <<"有刷电机2"
        <<"有刷电机3"
        <<"有刷电机4"
        <<"无刷电机1"
        <<"无刷电机2"
        <<"无刷电机3"
        <<"无刷电机4"
        <<"数据查看"
        <<"状态诊断"
        <<"串口助手"
        <<"退出";
    BtnGroup = new QButtonGroup(this);
    for(int i=0; i<Button_names.count(); i++)
    {
        QPushButton* btn = new QPushButton(this);
        connect(btn, &QPushButton::clicked, this, &Widget::buttonClicked);
        btn->setMinimumHeight(60);
        btn->setCheckable(true);
        btn->setText(QString("%1.%2").arg(i,2,10,QChar('0')).arg(Button_names.at(i)));

        // 基础样式（所有按钮共用）
        QString baseStyle = R"(
        QPushButton {
            font: 25px;
            text-align: left;
            background-color: #F5F5F5;
            border: 2px solid #D3D3D3;
            border-radius: 15px;
            padding-left: 20px;
            min-width: 200px;
        }
        QPushButton:hover {
            background-color: #E0E0E0;
            border: 2px solid #A8A8A8;
        }
        QPushButton:pressed {
            background-color: #D0D0D0;
            border: 2px solid #909090;
        }
        QPushButton:checked {
            background-color: #90EE90;
            border: 2px solid #70C070;
        }
    )";

        // 如果是最后一个按钮
        if(i == Button_names.count() - 1) {
            baseStyle += R"(
            QPushButton:hover {
                background-color: #FF8C00;  /* 深橙色 */
                border: 2px solid #CC7000;
                color: white;              /* 悬停时文字变白 */
            }
        )";
        }

        btn->setStyleSheet(baseStyle);
        BtnGroup->addButton(btn,i);
        NavWidget->layout()->addWidget(btn);
    }
    BtnGroup->button(1)->click();
}

void Widget::initMain()
{
    MainLayout->addWidget(StackedWidget);

    // 创建各界面并保存指针
    dcm1* dcm1Widget = new dcm1(m_sql, "dcm_1data",this);
    StackedWidget->addWidget(dcm1Widget);

    dcm2* dcm2Widget = new dcm2(m_sql, "dcm_2data",this);
    StackedWidget->addWidget(dcm2Widget);

    dcm3* dcm3Widget = new dcm3(m_sql, "dcm_3data",this);
    StackedWidget->addWidget(dcm3Widget);

    dcm4* dcm4Widget = new dcm4(m_sql, "dcm_4data",this);
    StackedWidget->addWidget(dcm4Widget);

    dcm5* dcm5Widget = new dcm5(m_sql, "foc_1data",this);
    StackedWidget->addWidget(dcm5Widget);

    dcm6* dcm6Widget = new dcm6(m_sql, "foc_2data",this);
    StackedWidget->addWidget(dcm6Widget);

    dcm7* dcm7Widget = new dcm7(m_sql, "foc_3data",this);
    StackedWidget->addWidget(dcm7Widget);

    dcm8* dcm8Widget = new dcm8(m_sql, "foc_4data",this);
    StackedWidget->addWidget(dcm8Widget);

    StackedWidget->addWidget(new show_sql());

    DiagnosisWindow* diagnosisWindow = new DiagnosisWindow();
    StackedWidget->addWidget(diagnosisWindow);

    // 创建串口助手并保存指针
    usarthelper* serialHelper = new usarthelper(m_sql, this);
    StackedWidget->addWidget(serialHelper);

    // 关键：连接Widget的信号到串口助手的槽函数
    connect(this, &Widget::sendSerialCommand,
            serialHelper, &usarthelper::onSendCommand);

    // 原有诊断窗口连接
    connect(serialHelper, &usarthelper::motorStatusUpdated,
            diagnosisWindow, &DiagnosisWindow::handleMotorStatusUpdate);
}

void Widget::buttonClicked()
{
    int id = BtnGroup->checkedId();
    if(id == 0)
    {
        Loop_Init();
    }
    else
    {
        index_widget = 0;
        if (LoopIsOpen == 1)
        {
            Timer_Loop->stop();
            LoopIsOpen = 0;
        }
        if(id == Button_names.count() - 1)
        {
            qDebug()<<id;
            QCoreApplication::quit();
        }
        if(id != 0 && id != (Button_names.count() - 1))
            StackedWidget->setCurrentIndex(id - 1);
    }

    switch(id)
    {
    // 直流电机界面
    case 1: // 直流电机1
        emit sendSerialCommand("dcm1");
        qDebug() << "切换到直流电机1界面，发送dcm1指令";
        break;
    case 2: // 直流电机2
        emit sendSerialCommand("dcm2");
        qDebug() << "切换到直流电机2界面，发送dcm2指令";
        break;
    case 3: // 直流电机3
        emit sendSerialCommand("dcm3");
        qDebug() << "切换到直流电机3界面，发送dcm3指令";
        break;
    case 4: // 直流电机4
        emit sendSerialCommand("dcm4");
        qDebug() << "切换到直流电机4界面，发送dcm4指令";
        break;

    // FOC电机界面
    case 5: // FOC电机1
        emit sendSerialCommand("foc1");
        qDebug() << "切换到FOC电机1界面，发送foc1指令";
        break;
    case 6: // FOC电机2
        emit sendSerialCommand("foc2");
        qDebug() << "切换到FOC电机2界面，发送foc2指令";
        break;
    case 7: // FOC电机3
        emit sendSerialCommand("foc3");
        qDebug() << "切换到FOC电机3界面，发送foc3指令";
        break;
    case 8: // FOC电机4
        emit sendSerialCommand("foc4");
        qDebug() << "切换到FOC电机4界面，发送foc4指令";
        break;
    default:
        qWarning() << "未知的界面ID:" << id;
        break;
    }
}

void Widget::Loop_Init()
{
    static QTimer* motorSendTimer = nullptr;
    static int currentMotorIndex = 0;

    // 如果定时器不存在，则创建并设置
    if (!motorSendTimer) {
        motorSendTimer = new QTimer(this);
        motorSendTimer->setInterval(8000); // 6秒间隔

        connect(motorSendTimer, &QTimer::timeout, this, [this]() {
            // 电机命令列表
            static const QStringList motorCommands = {
                "dcm1", "dcm2", "dcm3", "dcm4",
                "foc1", "foc2", "foc3", "foc4"
            };

            if (currentMotorIndex < motorCommands.size()) {
                // 发送当前电机命令
                emit sendSerialCommand(motorCommands[currentMotorIndex]);
                qDebug() << "发送电机命令:" << motorCommands[currentMotorIndex];

                currentMotorIndex++;
            } else {
                // 所有命令发送完成
                qDebug() << "所有电机命令发送完成";
                motorSendTimer->stop();
                motorSendTimer->deleteLater();
                motorSendTimer = nullptr;
                currentMotorIndex = 0;
            }
        });

        motorSendTimer->start();
        qDebug() << "开始定时发送电机命令...";
    }

    StackedWidget->setCurrentIndex(Button_names.count() - 4);
}

QSqlDatabase getSharedDatabase() {
    static QSqlDatabase db;
    if (!db.isValid()) {
        db = QSqlDatabase::addDatabase("QSQLITE", "shared_conn"); // 命名连接
        db.setDatabaseName("mydatabase.db");
        if (!db.open()) {
            qDebug() << "数据库打开失败：" << db.lastError();
        }
    }
    return db;
}


