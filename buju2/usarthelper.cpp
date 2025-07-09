#include "usarthelper.h"

usarthelper::usarthelper(show_sql *sql,QWidget *parent) : QWidget(parent),my_sql(sql)
{
    SerialPort = new QSerialPort(this);
    // 确保数据库已打开
    if (!DatabaseManager::instance().openDatabase()) {
        qDebug() << "Failed to open database!";
    }
    setupUI();
    setupSerialPort();
    setupConnections();
}

void usarthelper::setupUI()
{
    // 左侧
    ReceiveLabel = new QLabel("接收区");
    ReceiveArea = new QPlainTextEdit();
    ReceiveArea->setReadOnly(true);

    SendLabel = new QLabel("发送区");
    SendArea = new QPlainTextEdit();

    QVBoxLayout *leftLayout = new QVBoxLayout();
    leftLayout->addWidget(ReceiveLabel);
    leftLayout->addWidget(ReceiveArea, 3);
    leftLayout->addWidget(SendLabel);
    leftLayout->addWidget(SendArea, 2);

    QString buttonStyle = R"(
        QPushButton {
            font: 13px;
            padding: 4px 8px;
            background-color: #FFE4B5;  /* 浅橙色背景 */
            border: 1px solid #FFA07A;  /* 浅橙色边框 */
            border-radius: 10px;        /* 更小的圆角 */
            min-width: 110px;           /* 更小的最小宽度 */
            min-height: 26px;           /* 更小的最小高度 */
        }
        QPushButton:hover {
            background-color: #FFDAB9;   /* 悬停时稍深的橙色 */
            border: 1px solid #FF8C69;
        }
        QPushButton:pressed {
            background-color: #FFC080;   /* 按下时更深的橙色 */
            border: 1px solid #FF7043;
        }
    )";

    // 右侧
    SendButton = new QPushButton("发送");
    SendButton->setStyleSheet(buttonStyle);
    ReceivetButton = new QPushButton("接收");
    ReceivetButton->setStyleSheet(buttonStyle);
    startUSART = new QPushButton("打开串口");
    startUSART->setStyleSheet(buttonStyle);
    endUSART = new QPushButton("关闭串口");
    endUSART->setStyleSheet(buttonStyle);
    ClearSendArea = new QPushButton("清除发送");
    ClearSendArea->setStyleSheet(buttonStyle);
    ClearReceiveArea = new QPushButton("清除接收");
    ClearReceiveArea->setStyleSheet(buttonStyle);

    QString comboBoxStyle = R"(
    QComboBox {
        font: 13px;
        padding: 4px 8px;
        background-color: #FFE4B5;  /* 浅橙色背景 */
        border: 1px solid #FFA07A;  /* 浅橙色边框 */
        border-radius: 10px;        /* 圆角 */
        min-width: 110px;           /* 最小宽度 */
        min-height: 26px;           /* 最小高度 */
        padding-right: 20px;        /* 为下拉箭头预留空间 */
    }
    QComboBox:hover {
        background-color: #FFDAB9;   /* 悬停时稍深的橙色 */
        border: 1px solid #FF8C69;
    }
    QComboBox:pressed {
        background-color: #FFC080;   /* 按下时更深的橙色 */
        border: 1px solid #FF7043;
    }
    QComboBox::drop-down {
        subcontrol-origin: padding;
        subcontrol-position: right center;
        width: 20px;                /* 下拉箭头宽度 */
        border-left: 1px solid #FFA07A;  /* 左侧分隔线 */
    }
    QComboBox::down-arrow {
        image: url(:/icons/down_arrow.png);  /* 自定义下拉箭头图标 */
        width: 12px;
        height: 12px;
    }
    QComboBox QAbstractItemView {
        background-color: #FFE4B5;  /* 下拉列表背景 */
        border: 1px solid #FFA07A;  /* 下拉列表边框 */
        selection-background-color: #FFDAB9;  /* 选中项背景 */
    }
)";
    portNumber = new QComboBox();
    portNumber->setStyleSheet(comboBoxStyle);
    baudRate = new QComboBox();
    baudRate->setStyleSheet(comboBoxStyle);
    SendMode = new QComboBox();
    SendMode->setStyleSheet(comboBoxStyle);
    ReceiveMode = new QComboBox();
    ReceiveMode->setStyleSheet(comboBoxStyle);

    PortLabel = new QLabel("串口号");
    BaudLabel = new QLabel("波特率");

    QVBoxLayout *rightLayout = new QVBoxLayout();
    rightLayout->addWidget(PortLabel);
    rightLayout->addWidget(portNumber);
    rightLayout->addWidget(BaudLabel);
    rightLayout->addWidget(baudRate);
    rightLayout->addWidget(new QLabel("发送模式"));
    rightLayout->addWidget(SendMode);
    rightLayout->addWidget(new QLabel("接收模式"));
    rightLayout->addWidget(ReceiveMode);
    rightLayout->addWidget(startUSART);
    rightLayout->addWidget(endUSART);
    rightLayout->addWidget(SendButton);
    rightLayout->addWidget(ReceivetButton);
    rightLayout->addWidget(ClearSendArea);
    rightLayout->addWidget(ClearReceiveArea);
    rightLayout->addStretch();

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->addLayout(leftLayout, 3);
    mainLayout->addLayout(rightLayout, 1);

    this->setLayout(mainLayout);
    this->setWindowTitle("串口助手");
}

void usarthelper::setupSerialPort()
{
    ports.clear();
    for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts()) {
        ports.append(info.portName());
        portNumber->addItem(info.portName());
    }

    QStringList rates = {"115200","9600", "57600", "38400", "19200", "4800" };
    baudRate->addItems(rates);

    SendMode->addItems({"Text", "Hex"});
    ReceiveMode->addItems({"Text", "Hex"});
}

void usarthelper::setupConnections()
{
    connect(SerialPort, &QSerialPort::readyRead, this, &usarthelper::handleReadyRead);
    connect(SendButton, &QPushButton::clicked, this, &usarthelper::handleSend);
    connect(startUSART, &QPushButton::clicked, this, &usarthelper::openSerial);
    connect(endUSART, &QPushButton::clicked, this, &usarthelper::closeSerial);

    connect(ClearSendArea, &QPushButton::clicked, SendArea, &QPlainTextEdit::clear);
    connect(ClearReceiveArea, &QPushButton::clicked, ReceiveArea, &QPlainTextEdit::clear);
}

void usarthelper::debugPrintReceivedData(const QByteArray &data)
{
    QString hexStr;
    for (char byte : data) {
        hexStr += QString("%1 ").arg(static_cast<unsigned char>(byte), 2, 16, QLatin1Char('0')).toUpper();
    }

    // 使用 Qt 6 的 QStringDecoder 代替 QTextCodec
    QStringDecoder decoder("GBK");
    QString decodedStr = decoder(data);

    qDebug() << "[Raw Hex]:" << hexStr.trimmed();
    qDebug() << "[Decoded Text]:" << decodedStr;
}

void usarthelper::parseFrameData(const QByteArray& raw)
{
    qDebug()<<"开始解码数据";
    // UTF-8 解码
    QString text = QString::fromUtf8(raw);
    QStringList lines = text.split('\n', Qt::SkipEmptyParts);

    // 检查帧头和帧尾
    if (lines.size() < 515 || lines.first().trimmed() != "FF FF" || lines.last().trimmed() != "FF FE") {
        qDebug() << "[ERROR] 帧格式不合法！";
        qDebug() << "帧头:" << (lines.isEmpty() ? "空" : lines.first());
        qDebug() << "帧尾:" << (lines.isEmpty() ? "空" : lines.last());
        return;
    }

    // 获取设备名称和通道类型
    QString deviceName = lines[1].trimmed();
    QString channelId = lines[2].trimmed();
    QString channelType;
    QString stateType = lines[3].trimmed();//电机状态
    QString reasonType = lines[4].trimmed();//判断原因

    qDebug()<<"电机名字"<<deviceName<<"通道ID"<<channelId<<"电机状态"<<stateType<<"判断原因"<<reasonType;

    // 确定电机序号
    int motorIndex = -1;
    if (deviceName == "dcm_1") {
        motorIndex = 0; // 直流电机1
    } else if (deviceName == "dcm_2") {
        motorIndex = 1; // 直流电机2
    } else if (deviceName == "dcm_3") {
        motorIndex = 2; // 直流电机3
    } else if (deviceName == "dcm_4") {
        motorIndex = 3; // 直流电机4
    } else if (deviceName == "foc_1") {
        motorIndex = 4; // FOC电机1
    } else if (deviceName == "foc_2") {
        motorIndex = 5; // FOC电机2
    } else if (deviceName == "foc_3") {
        motorIndex = 6; // FOC电机3
    } else if (deviceName == "foc_4") {
        motorIndex = 7; // FOC电机4
    }

    // 发出信号通知更新电机状态
    if (motorIndex != -1) {
        emit motorStatusUpdated(motorIndex, stateType, reasonType);
    }

    if (channelId == "1") {
        channelType = "current";
    } else if (channelId == "2") {
        channelType = "voice";
    } else if (channelId == "3") {
        channelType = "shake";
    } else {
        qDebug() << "[ERROR] 未知通道编号:" << channelId;
        return;
    }

    // 根据设备名称确定表名
    QString tableName;
    if (deviceName == "foc_1") {
        tableName = "foc_1data";
    } else if (deviceName == "foc_2") {
        tableName = "foc_2data";
    } else if (deviceName == "foc_3") {
        tableName = "foc_3data";
    } else if (deviceName == "foc_4") {
        tableName = "foc_4data";
    } else if (deviceName == "dcm_1") {
        tableName = "dcm_1data";
    } else if (deviceName == "dcm_2") {
        tableName = "dcm_2data";
    } else if (deviceName == "dcm_3") {
        tableName = "dcm_3data";
    } else if (deviceName == "dcm_4") {
        tableName = "dcm_4data";
    } else {
        qDebug() << "[ERROR] 未知设备名称:" << deviceName;
        return;
    }

    qDebug() << "Processing data for device:" << deviceName << ", channel:" << channelType << ", table:" << tableName;

    // 获取数据库连接
    QSqlDatabase db = DatabaseManager::instance().database();

    // 检查数据库连接状态
    if (!db.isOpen()) {
        qDebug() << "[ERROR] 数据库未打开，尝试重新连接...";

        if (!DatabaseManager::instance().openDatabase()) {
            qDebug() << "[ERROR] 无法打开数据库！";
            return;
        }
    }

    // 创建表（如果不存在）
    QSqlQuery createQuery(db);
    QString createTableSql = QString(R"(
        CREATE TABLE IF NOT EXISTS %1 (
            name TEXT,
            id INTEGER PRIMARY KEY,
            current REAL,
            voice REAL,
            shake REAL
        )
    )").arg(tableName);

    if (!createQuery.exec(createTableSql)) {
        qDebug() << "创建表失败:" << createQuery.lastError().text();
        return;
    }

    // 开始事务
    db.transaction();

    // 清除该通道的旧数据
    QSqlQuery clearQuery(db);
    QString clearSql = QString("UPDATE %1 SET %2 = NULL WHERE name = :name")
                           .arg(tableName).arg(channelType);

    clearQuery.prepare(clearSql);
    clearQuery.bindValue(":name", deviceName);

    if (!clearQuery.exec()) {
        qDebug() << "清除旧数据失败:" << clearQuery.lastError().text();
        db.rollback();
        return;
    }

    // 记录成功处理的行数
    int successCount = 0;

    // 处理数据行 - 从第4行开始到倒数第2行（共1024个数据点）
    for (int i = 5; i < lines.size() - 1; ++i)
    {
        QString line = lines[i].trimmed();
        if (line.isEmpty()) continue;

        // 解析浮点数
        bool valueOk;
        double value = line.toDouble(&valueOk);

        if (valueOk) {
            // 使用相对索引作为ID（从1开始）
            int id = i - 2;

            // 构建SQL语句，只更新指定通道的值
            QSqlQuery updateQuery(db);
            QString updateSql = QString("UPDATE %1 SET %2 = :value WHERE name = :name AND id = :id")
                                    .arg(tableName).arg(channelType);

            // 如果ID不存在，则插入新记录
            if (!recordExists(db, deviceName, id, tableName)) {
                updateSql = QString("INSERT INTO %1 (name, id, %2) VALUES (:name, :id, :value)")
                                .arg(tableName).arg(channelType);
            }

            updateQuery.prepare(updateSql);
            updateQuery.bindValue(":name", deviceName);
            updateQuery.bindValue(":id", id);
            updateQuery.bindValue(":value", value);

            if (!updateQuery.exec()) {
                qDebug() << "更新数据失败:" << updateQuery.lastError().text();
                qDebug() << "失败的数据:" << line;
                db.rollback();
                return;
            }

            successCount++;
        }


        else {
            qDebug() << "[WARNING] 无法解析数据:" << line;
        }
    }

    // 提交事务
    if (!db.commit()) {
        qDebug() << "提交事务失败:" << db.lastError().text();
        return;
    }

    qDebug() << deviceName << "-" << channelType << "通道数据已更新到表" << tableName << "，共处理" << successCount << "条记录";
}

// 辅助函数：检查记录是否存在
bool usarthelper::recordExists(QSqlDatabase& db, const QString& name, int id, const QString& tableName)
{
    QSqlQuery query(db);
    query.prepare(QString("SELECT 1 FROM %1 WHERE name = :name AND id = :id").arg(tableName));
    query.bindValue(":name", name);
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        return true;
    }

    return false;
}

void usarthelper::handleReadyRead()
{
    static QByteArray buffer;

    QByteArray data = SerialPort->readAll();
    buffer.append(data);

    // 新增：显示接收到的数据到接收区
    displayReceivedData(data);
    //qDebug()<<"进入函数";

    // 帧分隔逻辑（保持原有）
    while (true) {
        int startIndex = buffer.indexOf("FF FF\n");
        if (startIndex == -1) break;

        int endIndex = buffer.indexOf("\nFF FE\n", startIndex);
        if (endIndex == -1) break;

        qDebug()<<"进入函数";

        // 计算完整帧长度（包含FF FE）
        int frameLen = endIndex + 7 - startIndex; // 7是"\nFF FE\n"长度
        QByteArray frame = buffer.mid(startIndex, frameLen);

        // 从缓冲中移除这一帧
        buffer.remove(0, startIndex + frameLen);

        parseFrameData(frame);
    }
}

// 新增：显示接收到的数据到接收区
void usarthelper::displayReceivedData(const QByteArray& data)
{
    QString displayText;

    // 添加时间戳
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    displayText += "[" + timestamp + "] ";

    // 根据接收模式格式化数据
    if (ReceiveMode->currentText() == "Text") {
        // 文本模式：使用UTF-8解码（可根据实际需求修改为其他编码）
        displayText += QString::fromUtf8(data);
    } else {
        // 十六进制模式：将每个字节转换为两位十六进制字符串
        for (unsigned char byte : data) {
            displayText += QString("%1 ").arg(byte, 2, 16, QLatin1Char('0')).toUpper();
        }
    }

    // 追加到接收区
    ReceiveArea->appendPlainText(displayText);

    // 限制最大行数（避免内存溢出）
    limitLineCount(2000);

    // 滚动到底部以显示最新内容
    ReceiveArea->moveCursor(QTextCursor::End);
}

// 新增：限制接收区最大行数
void usarthelper::limitLineCount(int maxLines)
{
    QTextDocument* doc = ReceiveArea->document();
    if (doc->lineCount() <= maxLines) return;

    // 计算需要删除的行数
    int linesToRemove = doc->lineCount() - maxLines;

    // 使用QTextCursor删除最旧的行
    QTextCursor cursor(doc);
    cursor.movePosition(QTextCursor::Start);

    // 移动到要删除的第一行的末尾
    for (int i = 0; i < linesToRemove; ++i) {
        cursor.movePosition(QTextCursor::EndOfLine);
        cursor.movePosition(QTextCursor::NextCharacter); // 移动到下一行的开头
    }

    // 删除从开始到当前位置的所有内容
    cursor.movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
}

void usarthelper::handleSend()
{
    if (!SerialPort->isOpen()) return;

    QString text = SendArea->toPlainText();
    QByteArray bytes;

    if (SendMode->currentText() == "Hex") {
        QStringList hexList = text.simplified().split(' ');
        for (QString s : hexList) {
            bool ok;
            char byte = s.toUInt(&ok, 16);
            if (ok) bytes.append(byte);
        }
    } else {
        bytes = text.toUtf8();
    }

    SerialPort->write(bytes);
}

void usarthelper::openSerial()
{
    if (SerialPort->isOpen()) SerialPort->close();

    SerialPort->setPortName(portNumber->currentText());
    SerialPort->setBaudRate(baudRate->currentText().toInt());

    if (SerialPort->open(QIODevice::ReadWrite)) {
        qDebug() << "串口打开成功";
    } else {
        qDebug() << "串口打开失败";
    }
}

void usarthelper::closeSerial()
{
    if (SerialPort->isOpen()) {
        SerialPort->close();
        qDebug() << "串口已关闭";
    }
}

usarthelper::~usarthelper()
{
    if (SerialPort->isOpen()) SerialPort->close();
    delete SerialPort;
}

void usarthelper::onSendCommand(const QString &cmd)
{
    if (!SerialPort->isOpen()) {
        qDebug() << "串口未打开，无法发送指令：" << cmd;
        return;
    }

    // 根据当前发送模式处理指令（文本/十六进制）
    if (SendMode->currentText() == "Text") {
        // 文本模式：直接发送字符串（UTF-8编码）
        SerialPort->write(cmd.toUtf8());
    } else {
        // 十六进制模式：如果需要将字符串转为十六进制发送（根据需求调整）
        QByteArray hexData;
        QStringList hexList = cmd.split(' ');
        for (const QString &hex : hexList) {
            bool ok;
            char byte = hex.toUInt(&ok, 16);
            if (ok) hexData.append(byte);
        }
        SerialPort->write(hexData);
    }

    qDebug() << "发送指令：" << cmd;
}




