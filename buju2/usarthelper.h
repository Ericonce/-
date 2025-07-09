#ifndef USARTHELPER_H
#define USARTHELPER_H

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QComboBox>
#include <QScrollBar>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QStringDecoder>   // Qt 6 解码文本
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>

#include "show_sql.h"
#include "database_manager.h"

class usarthelper : public QWidget
{
    Q_OBJECT
public:
    explicit usarthelper(show_sql *sql = nullptr,QWidget *parent = nullptr);
    ~usarthelper();

signals:
    void motorStatusUpdated(int motorIndex, const QString& status, const QString& reason);//电机状态更新信号函数

private:
    QPlainTextEdit *SendArea;
    QPlainTextEdit *ReceiveArea;
    QPushButton * SendButton;
    QPushButton * ReceivetButton;
    QPushButton *startUSART;
    QPushButton *endUSART;
    QPushButton *ClearSendArea;
    QPushButton *ClearReceiveArea;

    QComboBox *portNumber;
    QComboBox *baudRate;
    QComboBox *ReceiveMode;
    QComboBox *SendMode;

    QSerialPort *SerialPort;
    QVector<QString>ports;
    QVector<QComboBox*>setups;
    QVector<QLabel*>labels;

    QLabel *ReceiveLabel;
    QLabel *SendLabel;
    QLabel *PortLabel;
    QLabel *BaudLabel;

    show_sql *my_sql;

private:
    void setupUI();
    void setupSerialPort();
    void setupConnections();
    void handleReadyRead();
    void handleSend();
    void openSerial();
    void closeSerial();
    void debugPrintReceivedData(const QByteArray &data);
    void parseFrameData(const QByteArray& raw);
    bool recordExists(QSqlDatabase& db, const QString& name, int id, const QString& tableName);
    void limitLineCount(int maxLines);
    void displayReceivedData(const QByteArray& data);

public slots:
    // 新增：接收Widget发送的指令并发送到串口
    void onSendCommand(const QString &cmd);
};

#endif // USARTHELPER_H
