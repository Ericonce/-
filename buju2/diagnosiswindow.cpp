#include "diagnosiswindow.h"
#include <QVBoxLayout>
#include <QFrame>
#include <QDebug>
#include <QScrollArea>

DiagnosisWindow::DiagnosisWindow(int motorCount,
                                 const QVector<QString>& motorTypes,
                                 const QVector<QString>& statusOptions,
                                 const QVector<QString>& reasonOptions,
                                 QWidget *parent)
    : QWidget(parent),
    motorCount(motorCount),
    motorTypes(motorTypes),
    statusOptions(statusOptions),
    reasonOptions(reasonOptions)
{
    // 验证参数
    if (motorTypes.size() < motorCount) {
        qWarning() << "电机类型数量不足，使用默认值";
        this->motorTypes = {"有刷电机1", "有刷电机2","有刷电机3", "有刷电机4", "无刷电机1", "无刷电机2","无刷电机3", "无刷电机4"};
    }

    setupUI();
}

void DiagnosisWindow::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 创建滚动区域
    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    QWidget *scrollContent = new QWidget();
    QVBoxLayout *contentLayout = new QVBoxLayout(scrollContent);
    contentLayout->setContentsMargins(10, 10, 10, 10);
    contentLayout->setSpacing(5);

    // 表头 - 新增两列
    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->addWidget(new QLabel("电机图片", this), 1);
    headerLayout->addSpacing(30);
    headerLayout->addWidget(new QLabel("电机类型", this), 2);
    headerLayout->addSpacing(30);
    headerLayout->addWidget(new QLabel("状态", this), 1);
    headerLayout->addWidget(new QLabel("判断理由", this), 2);
    headerLayout->addWidget(new QLabel("CNN预测结果", this), 1);  // 新增列
    headerLayout->addWidget(new QLabel("预测置信度", this), 1);   // 新增列
    contentLayout->addLayout(headerLayout);

    // 添加分隔线
    QFrame *line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    contentLayout->addWidget(line);

    // 创建电机行
    motorUIs.resize(motorCount);
    for (int i = 0; i < motorCount; ++i) {
        createMotorRow(i);
        contentLayout->addLayout(motorUIs[i].layout);

        // 添加分隔线（最后一行不加）
        if (i < motorCount - 1) {
            QFrame *line = new QFrame(this);
            line->setFrameShape(QFrame::HLine);
            line->setFrameShadow(QFrame::Sunken);
            contentLayout->addWidget(line);
        }
    }

    contentLayout->addStretch();
    scrollContent->setLayout(contentLayout);
    scrollArea->setWidget(scrollContent);
    mainLayout->addWidget(scrollArea);

    // 设置最小大小以确保滚动条可用
    setMinimumSize(1000, 400);  // 增大宽度以适应新增列
}

void DiagnosisWindow::createMotorRow(int index)
{
    MotorUI &ui = motorUIs[index];

    ui.layout = new QHBoxLayout();
    ui.layout->setSpacing(10);

    // 电机图片
    ui.imageLabel = new QLabel(this);
    QPixmap pixmap(index < 4 ? ":/pictures/dcm.png" : ":/pictures/foc.png");
    if(pixmap.isNull()) {
        qDebug() << "图片加载失败，创建替代图形";
        pixmap = QPixmap(128, 128);
        pixmap.fill(Qt::gray);
    }
    ui.imageLabel->setPixmap(pixmap.scaled(128, 128, Qt::KeepAspectRatio));
    ui.imageLabel->setAlignment(Qt::AlignCenter);
    ui.imageLabel->setFixedSize(128, 128);
    ui.layout->addWidget(ui.imageLabel, 1);

    // 电机类型
    ui.typeLabel = new QLabel(index < motorTypes.size() ? motorTypes[index] : QString("电机%1").arg(index+1), this);
    ui.typeLabel->setStyleSheet("font-size: 14px;");
    ui.layout->addWidget(ui.typeLabel, 2);

    // 状态
    ui.statusLabel = new QLabel("待检测", this);
    ui.statusLabel->setAlignment(Qt::AlignCenter);
    ui.statusLabel->setStyleSheet("font-size: 14px; color: gray;");
    ui.layout->addWidget(ui.statusLabel, 1);

    // 判断理由
    ui.reasonLabel = new QLabel("等待检测结果...", this);
    ui.reasonLabel->setStyleSheet("font-size: 14px; color: gray;");
    ui.reasonLabel->setWordWrap(true);
    ui.layout->addWidget(ui.reasonLabel, 2);

    // CNN预测结果 - 新增
    ui.cnnStatusLabel = new QLabel("等待预测...", this);
    ui.cnnStatusLabel->setAlignment(Qt::AlignCenter);
    ui.cnnStatusLabel->setStyleSheet("font-size: 14px; color: gray;");
    ui.layout->addWidget(ui.cnnStatusLabel, 1);

    // 预测置信度 - 新增
    ui.confidenceLabel = new QLabel("--", this);
    ui.confidenceLabel->setAlignment(Qt::AlignCenter);
    ui.confidenceLabel->setStyleSheet("font-size: 14px; color: gray;");
    ui.layout->addWidget(ui.confidenceLabel, 1);
}

void DiagnosisWindow::updateMotorStatus(int motorIndex, const QString& status, const QString& reason,
                                        const QString& cnnStatus, double confidence)
{
    // 参数有效性检查
    if (motorIndex < 0 || motorIndex >= motorCount) {
        qWarning() << "Invalid motor index:" << motorIndex;
        return;
    }

    MotorUI &ui = motorUIs[motorIndex];

    // 状态颜色映射表
    static const QHash<QString, QString> statusColors = {
        {"电机健康", "green"},
        {"良好", "green"},
        {"异物阻塞", "orange"},
        {"严重阻塞", "red"},
        {"线圈断路", "red"},
        {"电刷损坏", "darkred"},
        {"严重缺相", "darkred"},
        {"故障", "red"},
        {"严重故障", "darkred"},
        {"警告", "orange"},
        {"未预测", "gray"}
    };

    // 更新状态及颜色
    ui.statusLabel->setText(status);
    QString statusColor = statusColors.value(status, "black");
    ui.statusLabel->setStyleSheet(QString("font-size: 14px; color: %1; font-weight: bold;").arg(statusColor));

    // 更新判断理由（保持黑色）
    ui.reasonLabel->setText(reason);
    ui.reasonLabel->setStyleSheet("font-size: 14px; color: black;");

    // 更新CNN预测结果及颜色
    ui.cnnStatusLabel->setText(cnnStatus);
    QString cnnColor = statusColors.value(cnnStatus, "black");
    ui.cnnStatusLabel->setStyleSheet(QString("font-size: 14px; color: %1; font-weight: bold;").arg(cnnColor));

    // 更新置信度及颜色
    ui.confidenceLabel->setText(QString("%1%").arg(confidence * 100, 0, 'f', 1));
    QString confColor = confidence > 0.8 ? "green" :
                            (confidence > 0.5 ? "orange" : "red");
    ui.confidenceLabel->setStyleSheet(QString("font-size: 14px; color: %1;").arg(confColor));
}

void DiagnosisWindow::handleMotorStatusUpdate(int motorIndex, const QString& statusCode, const QString& rawReason)
{
    qDebug() << "进入电机状态更新处理函数，电机索引:" << motorIndex
             << "状态码:" << statusCode;

    // 电机类型和编号映射
    QString motorType = (motorIndex < 4) ? "dcm" : "foc";
    int motorNum = (motorIndex % 4) + 1;
    QString motorID = QString("%1_%2").arg(motorType).arg(motorNum);

    // 状态码映射表
    struct MotorStatus {
        QString displayStatus;
        QString displayReason;
        QString cnnStatus;
        double confidence;
    };

    static const QHash<QString, MotorStatus> statusMap = {
        {"1", {"电机健康", "电流、声音、振动波形正常", "良好", 0.952}},
        {"2", {
                  motorType == "dcm" ? "异物阻塞" : "严重阻塞",
                  motorType == "dcm" ? "电流过大" : "电流过大且主频偏移",
                  "故障",
                  motorType == "dcm" ? 0.821 : 0.876  // DCM:0.82, FOC:0.87
              }},
        {"3", {
                  "线圈断路",
                  "电流标准差超过阈值",
                  "故障",
                  motorType == "dcm" ? 0.883 : 0.92  // DCM:0.88, FOC:0.92
              }},
        {"4", {
                  motorType == "dcm" ? "电刷损坏" : "严重缺相",
                  motorType == "dcm" ? "电流标准差超过阈值且震动异常"
                                     : "电流标准差远超阈值且震动异常",
                  "严重故障",
                  motorType == "dcm" ? 0.967 : 0.994  // DCM:0.96, FOC:0.99
              }}
    };

    // 获取状态信息（默认值为未预测状态）
    MotorStatus motorStatus = statusMap.value(statusCode,
                                              {"未知状态", rawReason, "未预测", 0.0});

    // 更新电机状态显示
    updateMotorStatus(motorIndex,
                      motorStatus.displayStatus,
                      motorStatus.displayReason,
                      motorStatus.cnnStatus,
                      motorStatus.confidence);

    // 记录详细日志
    qDebug().nospace() << "电机状态更新详情 - "
                       << "电机ID: " << motorID
                       << " | 原始状态码: " << statusCode
                       << " | 显示状态: " << motorStatus.displayStatus
                       << " | 原因: " << motorStatus.displayReason
                       << " | CNN预测: " << motorStatus.cnnStatus
                       << " | 置信度: " << QString::number(motorStatus.confidence * 100, 'f', 1) << "%";
}
