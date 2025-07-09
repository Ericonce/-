#ifndef DCM1_H
#define DCM1_H

#include <QWidget>
#include <QChart>
#include <QtCharts/QLineSeries>
#include <QValueAxis>
#include <QTimer>
#include <QLabel>
#include <QPointF>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QChartView>
#include <QChart>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <cmath>
#include <QGraphicsDropShadowEffect>

#include "show_sql.h"

extern "C" {  // 确保 C++ 不会对 KISS FFT 函数进行名称修饰
#include "kissfft/kiss_fft.h"
}

class dcm1 : public QWidget
{
    Q_OBJECT
public:
    explicit dcm1(show_sql* sql, const QString& tableName, QWidget* parent = nullptr);

signals:

private://界面布局
    QVBoxLayout* verticalLayout;
    QHBoxLayout* horizonalLayout;
    QVBoxLayout* ButtonLayout;
    QHBoxLayout *ChannelSelectionLayout;//通道选择按钮布局

    QChart* chart1;
    QChart* chart2;
    QValueAxis *axisX2;
    QValueAxis *axisY2;

    QValueAxis* axisX;

    void InitUi();
    void InitChartView();

private://按钮功能
    bool Current_Data_Init = false;
    bool Voice_Data_Init = false;
    bool Vibration_Data_Init = false;

    QPushButton *VibrationButton;
    QPushButton *VoiceButton;
    QPushButton *CurrentButton;
    QPushButton *Running_Button_Time;
    QPushButton *Running_Button_Fre;

    void ChanneSelectInit();//通道选择按钮初始化函数
    void InitButton();//按钮初始化函数

private://contents of data
    QVector<double> currentData;  // 存储电流数据
    bool isDataLoaded_Current = false;// 电流数据是否已加载
    int dataIndex_Current = 0;           // 当前电流数据索引
    int currentCount;

    QVector<double> voiceData;  // 存储电流数据
    bool isDataLoaded_Voice = false;// 电流数据是否已加载
    int dataIndex_Voice = 0;           // 当前电流数据索引
    int voiceCount;

    QVector<double> vibrationData;  // 存储电流数据
    bool isDataLoaded_Vibration = false;// 电流数据是否已加载
    int dataIndex_Vibration = 0;           // 当前电流数据索引
    int vibrationCount;

    bool m_timeRunning = false; // 跟踪时域运行状态

    QTimer *timer_generate_Vibration;
    QTimer *timer_generate_Current;
    QTimer *timer_generate_Voice;

    QLineSeries *Series_Time_Vibration;
    QLineSeries *Series_Time_Current;
    QLineSeries *Series_Time_Voice;

    QLineSeries *Series_Time_Vibration_Fre;
    QLineSeries *Series_Time_Current_Fre;
    QLineSeries *Series_Time_Voice_Fre;

    QLineSeries *series2;

    QValueAxis *axisX2_Current; // 电流专用X轴
    QValueAxis *axisX2_Voice; // 声音专用X轴
    QValueAxis *axisX2_Vibration; // 振动专用X轴

    QValueAxis *axisY2_Current; // 电流专用Y轴
    QValueAxis *axisY2_Voice; // 声音专用Y轴
    QValueAxis *axisY2_Vibration; // 振动专用Y轴

    QPushButton *sampleRateButton;
    QPushButton *fftSizeButton;
    int currentSampleRateIndex = 0;  // 当前采样率索引
    const QVector<double> sampleRates = {1000, 10000, 100000};  // 1K, 10K, 100K
    int currentFFTSizeIndex = 4;     // 当前FFT点数索引 (1024)
    const QVector<int> fftSizes = {128, 256, 512, 1024, 2048, 4096};  // 2的幂

    show_sql *my_sql;

    int count = 0;

    void InitData();

    void performFFT(const QVector<double>& data, int n,double cutoffFre);


private slots:
    //void onTimeout();//generate data
    void Running_Button_Time_Clicked();
    void Running_Button_Fre_Clicked();

    void Vibration_Data();
    void Current_Data();
    void Voice_Data();

    void Current_onTimeout();
    void Voice_onTimeout();
    void Vibration_onTimeout();

    void ToggleSampleRate();
    void ToggleFFTSize();

private://不同对象不一样的参数
    QString m_tableName;
};

#endif // DCM1_H
