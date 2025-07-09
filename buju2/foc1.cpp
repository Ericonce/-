#include "foc1.h"
#include <QDebug>

foc1::foc1(show_sql *sql,QWidget *parent)
    : QWidget{parent},my_sql(sql)
{
    InitUi();
    ChanneSelectInit();
    InitChartView();
    InitButton();
    InitData();

    timer_generate_Vibration = new QTimer(this);
    timer_generate_Current = new QTimer(this);
    timer_generate_Voice = new QTimer(this);

    timer_generate_Current->setInterval(10);//ms
    timer_generate_Voice->setInterval(10);//ms
    timer_generate_Vibration->setInterval(10);//ms

    connect(timer_generate_Current,&QTimer::timeout,this,&foc1::Current_onTimeout);
    connect(timer_generate_Voice,&QTimer::timeout,this,&foc1::Voice_onTimeout);
    connect(timer_generate_Vibration,&QTimer::timeout,this,&foc1::Vibration_onTimeout);

    connect(my_sql, &show_sql::dataUpdated, this, [this]() {
        isDataLoaded_Vibration = false;
        isDataLoaded_Current = false;  // 如果有其他数据也重置
        isDataLoaded_Voice = false;
        qDebug() << "dcm界面数据库已更新，重置数据加载标志";
    });

    QValueAxis* axisX = nullptr;
    QValueAxis* axisY = nullptr;

    // 检查是否已有坐标轴
    if (chart2->axes(Qt::Horizontal).isEmpty()) {
        axisX = new QValueAxis();
        axisX->setTitleText("频率 (Hz)");
        axisX->setRange(0,500);
        chart2->addAxis(axisX, Qt::AlignBottom);
    } else {
        axisX = qobject_cast<QValueAxis*>(chart2->axes(Qt::Horizontal).first());
    }

    if (chart2->axes(Qt::Vertical).isEmpty()) {
        axisY = new QValueAxis();
        axisY->setTitleText("幅度");
        axisY->setRange(0,5);
        chart2->addAxis(axisY, Qt::AlignLeft);
    } else {
        axisY = qobject_cast<QValueAxis*>(chart2->axes(Qt::Vertical).first());
    }

    axisX2_Vibration->setVisible(false);
    axisY2_Vibration->setVisible(false);
}

void foc1::InitUi()
{
    verticalLayout = new QVBoxLayout(this);
    verticalLayout->setSpacing(0);
    verticalLayout->setContentsMargins(0,0,0,0);
    horizonalLayout = new QHBoxLayout(this);
    horizonalLayout->setContentsMargins(0,0,0,0);
    ButtonLayout = new QVBoxLayout(this);
    ButtonLayout->setContentsMargins(0,0,0,0);
}

void foc1::InitChartView()
{
    QChartView* chartview1 = new QChartView(this);
    QChartView* chartview2 = new QChartView(this);
    chart1 = new QChart();
    chart2 = new QChart();
    chart1->setTitle("时域");
    chart2->setTitle("频域");
    chartview1->setChart(chart1);
    chartview2->setChart(chart2);
    verticalLayout->addWidget(chartview1);
    verticalLayout->addLayout(horizonalLayout);
    horizonalLayout->addWidget(chartview2);

    Series_Time_Vibration = new QLineSeries();
    Series_Time_Current = new QLineSeries();
    Series_Time_Voice = new QLineSeries();

    Series_Time_Current_Fre = new QLineSeries();
    Series_Time_Vibration_Fre = new QLineSeries();
    Series_Time_Voice_Fre = new QLineSeries();


    series2 = new QLineSeries();

    Series_Time_Current->setName("电流数据");
    Series_Time_Vibration->setName("振动数据");
    Series_Time_Voice->setName("声音数据");

    series2->setName("频域");

    chart1->addSeries(Series_Time_Vibration);
    chart1->addSeries(Series_Time_Current);
    chart1->addSeries(Series_Time_Voice);

    //生成动态图数据
    axisX2_Current = new QValueAxis;
    //axisX2_Current->setTitleText("电流Time(ms)");
    axisX2_Current->setRange(0,300);//设置值域
    chart1->addAxis(axisX2_Current,Qt::AlignTop);//沿底边显示

    axisX2_Voice = new QValueAxis;
    //axisX2_Voice->setTitleText("声音Time(ms)");
    axisX2_Voice->setRange(0,300);//设置值域
    chart1->addAxis(axisX2_Voice,Qt::AlignBottom);//沿底边显示

    axisX2_Vibration = new QValueAxis;
    //axisX2_Vibration->setTitleText("振动Time(ms)");
    axisX2_Vibration->setRange(0,300);//设置值域
    chart1->addAxis(axisX2_Vibration,Qt::AlignBottom);//沿底边显示

    // 创建电流专用Y轴（左侧）
    axisY2_Current = new QValueAxis;
    axisY2_Current->setTitleText("Current (A)");
    axisY2_Current->setRange(2.2, 2.4);  // 设置略大于实际值的范围
    axisX2_Current->setLabelFormat("%.0f");  // 设置标签格式
    axisY2_Current->setTickCount(5);     // 设置刻度数量
    chart1->addAxis(axisY2_Current, Qt::AlignLeft);

    // 创建声音专用Y轴（右侧）
    axisY2_Voice = new QValueAxis;
    axisY2_Voice->setTitleText("Voice/Vibration(V)");
    axisY2_Voice->setRange(0, 10);  // 设置略大于实际值的范围
    axisY2_Voice->setTickCount(5);     // 设置刻度数量
    chart1->addAxis(axisY2_Voice, Qt::AlignRight);

    axisY2_Vibration = new QValueAxis;
    axisY2_Vibration->setTitleText("Vibration (V)");
    axisY2_Vibration->setRange(0, 10);  // 设置略大于实际值的范围
    axisY2_Vibration->setTickCount(5);     // 设置刻度数量
    chart1->addAxis(axisY2_Vibration, Qt::AlignRight);

    // 绑定系列到坐标轴
    Series_Time_Voice->attachAxis(axisX2_Voice);
    Series_Time_Voice->attachAxis(axisY2_Voice);  // 声音用右侧Y轴

    Series_Time_Current->attachAxis(axisX2_Current);
    Series_Time_Current->attachAxis(axisY2_Current);  // 电流用左侧Y轴

    Series_Time_Vibration->attachAxis(axisX2_Vibration);
    Series_Time_Vibration->attachAxis(axisY2_Vibration);



    //axisX2_Vibration->setVisible(false);
    axisY2_Vibration->setVisible(false);

    //axisX2_Voice->setVisible(false);
    //axisY2_Voice->setVisible(false);

    //axisX2_Current->setVisible(false);
    //axisY2_Current->setVisible(false);
}

void foc1::ChanneSelectInit()
{
    ChannelSelectionLayout = new QHBoxLayout(this);
    verticalLayout->addLayout(ChannelSelectionLayout);

    // 通用按钮样式
    QString buttonStyle = R"(
    QPushButton {
        font: 16px;
        padding: 8px 15px;
        background-color: #F5F5F5;
        border: 2px solid #D3D3D3;
        border-radius: 15px;
        min-width: 120px;
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

    // 电流通道按钮
    CurrentButton = new QPushButton(this);
    CurrentButton->setText("电流数据通道");
    CurrentButton->setCheckable(true);
    CurrentButton->setStyleSheet(buttonStyle);
    ChannelSelectionLayout->addWidget(CurrentButton);
    connect(CurrentButton, &QPushButton::clicked, this, &foc1::Current_Data);

    // 声音通道按钮
    VoiceButton = new QPushButton(this);
    VoiceButton->setText("声音数据通道");
    VoiceButton->setCheckable(true);
    VoiceButton->setStyleSheet(buttonStyle);
    ChannelSelectionLayout->addWidget(VoiceButton);
    connect(VoiceButton, &QPushButton::clicked, this, &foc1::Voice_Data);

    // 震动通道按钮
    VibrationButton = new QPushButton(this);
    VibrationButton->setText("震动数据通道");
    VibrationButton->setCheckable(true);
    VibrationButton->setStyleSheet(buttonStyle);
    ChannelSelectionLayout->addWidget(VibrationButton);
    connect(VibrationButton, &QPushButton::clicked, this, &foc1::Vibration_Data);

    // 设置布局间距
    ChannelSelectionLayout->setSpacing(10);  // 按钮间距
    ChannelSelectionLayout->setContentsMargins(0, 0, 0, 0);  // 布局边距

    //CurrentButton->setChecked(true);
}

void foc1::InitButton()
{
    horizonalLayout->addLayout(ButtonLayout);

    // 原有按钮
    Running_Button_Time = new QPushButton(this);
    Running_Button_Time->setText("时域数据暂停");
    ButtonLayout->addWidget(Running_Button_Time);
    connect(Running_Button_Time, &QPushButton::clicked, this, &foc1::Running_Button_Time_Clicked);

    Running_Button_Fre = new QPushButton(this);
    Running_Button_Fre->setText("频域数据暂停");
    ButtonLayout->addWidget(Running_Button_Fre);
    connect(Running_Button_Fre, &QPushButton::clicked, this, &foc1::Running_Button_Fre_Clicked);

    // 新增：采样率切换按钮（1K/10K/100K 循环）
    sampleRateButton = new QPushButton(this);
    sampleRateButton->setText("采样率: 1K");
    ButtonLayout->addWidget(sampleRateButton);
    connect(sampleRateButton, &QPushButton::clicked, this, &foc1::ToggleSampleRate);

    // 新增：采样点数切换按钮（128/256/512/1024/2048/4096 循环）
    fftSizeButton = new QPushButton(this);
    fftSizeButton->setText("采样点数: 1024");
    ButtonLayout->addWidget(fftSizeButton);
    connect(fftSizeButton, &QPushButton::clicked, this, &foc1::ToggleFFTSize);

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

    Running_Button_Time->setStyleSheet(buttonStyle);
    Running_Button_Fre->setStyleSheet(buttonStyle);
    sampleRateButton->setStyleSheet(buttonStyle);
    fftSizeButton->setStyleSheet(buttonStyle);
}

void foc1::InitData()
{
    //生成静态图数据
    qreal t = 0,y2,intv = 0.1;
    int icount = 100;
    for(int i=1;i<100;i++)
    {
        y2 = qCos(t);
        series2->append(t,y2);
        t = t+intv;
    }
}

void foc1::Running_Button_Time_Clicked() {
    if (Running_Button_Time->text() == "时域数据暂停") {
        // 暂停所有定时器
        timer_generate_Current->stop();
        timer_generate_Voice->stop();
        timer_generate_Vibration->stop();
        Running_Button_Time->setText("时域数据开始");
    } else {
        // 只启动已选通道的定时器
        //if (CurrentButton->isChecked()) timer_generate_Current->start();
        //if (VoiceButton->isChecked()) timer_generate_Voice->start();
        //if (VibrationButton->isChecked()) timer_generate_Vibration->start();
        timer_generate_Current->start();
        timer_generate_Voice->start();
        timer_generate_Vibration->start();
        Running_Button_Time->setText("时域数据暂停");
    }
}

void foc1::Running_Button_Fre_Clicked()
{
    if(Running_Button_Fre->text() == "频域数据暂停")
    {
        Running_Button_Fre->setText("频域数据开始");
    }
    else if(VibrationButton->isChecked())
    {
        Running_Button_Fre->setText("频域数据暂停");
    }
}

void foc1::Vibration_Data()
{
    if(Current_Data_Init == false)
    {
        timer_generate_Vibration->start();
        Current_Data_Init = true;
    }
    bool show = VibrationButton->isChecked();
    Series_Time_Vibration->setVisible(show);
    Series_Time_Vibration_Fre->setVisible(show);
    axisX2_Vibration->setVisible(false);
    axisY2_Vibration->setVisible(false);
}

void foc1::Current_Data()
{
    if(Voice_Data_Init == false)
    {
        timer_generate_Current->start();
        Voice_Data_Init = true;
    }
    bool show = CurrentButton->isChecked();
    Series_Time_Current->setVisible(show);
    Series_Time_Current_Fre->setVisible(show);
    axisX2_Current->setVisible(show);
    axisY2_Current->setVisible(show);
}

void foc1::Voice_Data()
{
    if(Vibration_Data_Init == false)
    {
        timer_generate_Voice->start();
        Vibration_Data_Init = true;
    }
    bool show = VoiceButton->isChecked();
    Series_Time_Voice->setVisible(show);
    Series_Time_Voice_Fre->setVisible(show);
    axisX2_Voice->setVisible(show);
    axisY2_Voice->setVisible(show);
}

void foc1::Current_onTimeout()
{
    // 数据库检查逻辑（新增部分）
    static double lastFirstCurrentValue = 0.0;
    static bool firstRunCurrent = true;

    // 读取当前数据库的第一个电流数据值
    double currentFirstCurrentValue = 0.0;
    QList<double> firstValueList = my_sql->ReadDataFromTable("current", "foc_1data", 1); // 只读取1个值

    if (!firstValueList.isEmpty()) {
        currentFirstCurrentValue = firstValueList.first();
    } else {
        qDebug() << "Warning: No current data available for comparison";
    }

    // 检查数据是否有变化或首次运行（新增部分）
    bool dataChanged = (!firstRunCurrent && currentFirstCurrentValue != lastFirstCurrentValue);

    if (firstRunCurrent || dataChanged) {
        firstRunCurrent = false;
        lastFirstCurrentValue = currentFirstCurrentValue;
        isDataLoaded_Current = false; // 强制重新加载数据
    }

    // 数据加载逻辑（保持原有结构，增加变化检测）
    if (!isDataLoaded_Current) {
        qDebug() << "Loading current data...";

        // 调试表结构（原有部分）
        QSqlQuery schemaQuery("PRAGMA table_info(foc_1data)", my_sql->db);
        while (schemaQuery.next()) {
            qDebug() << "Column" << schemaQuery.value(0).toInt()
                     << ":" << schemaQuery.value(1).toString()
                     << "Type:" << schemaQuery.value(2).toString();
        }

        // 从数据库读取电流数据（原有部分）
        currentData = my_sql->ReadDataFromTable("current", "foc_1data");
        qDebug() << "Read" << currentData.size() << "current data points";

        if (currentData.isEmpty()) {
            qDebug() << "Error: No valid current data found!";
            return;
        }

        // 动态计算Y轴范围（原有部分）
        auto [min, max] = std::minmax_element(currentData.begin(), currentData.end());
        //qDebug() << "Y-axis range set to:" << *min - 10 << "-" << *max + 10;
        axisY2_Current->setRange(*min - 0.2, *max + 0.2);

        isDataLoaded_Current = true;
        qDebug() << "Successfully loaded current data";

        // 重置图表状态（新增部分）
        currentCount = 0;
        dataIndex_Current = 0;
        Series_Time_Current->clear();
        axisX2_Current->setMin(0);
        axisX2_Current->setMax(300);
        qDebug()<<"电流波形更新完成";
        performFFT(currentData, 256, 20);
    }

    // 如果没有数据，直接返回（原有部分）
    if (currentData.isEmpty()) return;

    // 数据循环显示逻辑（保持原有部分）
    double currentValue = currentData[dataIndex_Current];
    dataIndex_Current = (dataIndex_Current + 1) % currentData.size();

    // 更新图表（保持原有滚动显示逻辑）
    if (currentCount > 300) {
        Series_Time_Current->remove(0);
        axisX2_Current->setMin(currentCount - 300);
        axisX2_Current->setMax(currentCount);
    }
    Series_Time_Current->append(currentCount, currentValue);
    currentCount++;
}

void foc1::Voice_onTimeout()
{
    // 数据库检查逻辑
    static double lastFirstVoiceValue = 0.0;
    static bool firstRunVoice = true;

    // 读取当前数据库的第一个声音数据值
    double currentFirstVoiceValue = 0.0;
    QList<double> firstValueList = my_sql->ReadDataFromTable("voice", "foc_1data", 1); // 只读取1个值

    if (!firstValueList.isEmpty()) {
        currentFirstVoiceValue = firstValueList.first();
    } else {
        qDebug() << "Warning: No voice data available for comparison";
    }

    // 检查数据是否有变化或首次运行
    bool dataChanged = (!firstRunVoice && currentFirstVoiceValue != lastFirstVoiceValue);

    if (firstRunVoice || dataChanged) {
        firstRunVoice = false;
        lastFirstVoiceValue = currentFirstVoiceValue;
        isDataLoaded_Voice = false; // 强制重新加载数据
    }

    if (!isDataLoaded_Voice) {
        // 打印表结构（调试用途）
        qDebug() << "Loading voice data...";
        QSqlQuery schemaQuery("PRAGMA table_info(foc_1data)", my_sql->db);
        while (schemaQuery.next()) {
            qDebug() << "Column" << schemaQuery.value(0).toInt()
                     << ":" << schemaQuery.value(1).toString()
                     << "Type:" << schemaQuery.value(2).toString();
        }

        // 从已有数据库连接读取声音数据
        voiceData = my_sql->ReadDataFromTable("voice", "foc_1data");
        qDebug() << "Read" << voiceData.size() << "voice data points";

        if (voiceData.isEmpty()) {
            qDebug() << "Error: No valid voice data found!";
            return;
        }

        // 动态计算Y轴范围
        auto [min, max] = std::minmax_element(voiceData.begin(), voiceData.end());
        qDebug() << "Y-axis range set to:" << *min - 0.05 << "-" << *max + 0.05;
        axisY2_Voice->setRange(*min - 0.05, *max + 0.05);

        isDataLoaded_Voice = true;
        qDebug() << "Successfully loaded voice data";

        // 重置图表状态
        voiceCount = 0;
        dataIndex_Voice = 0;
        Series_Time_Voice->clear();
        axisX2_Voice->setMin(0);
        axisX2_Voice->setMax(300);

        qDebug()<<"声音波形更新完成";
        performFFT(voiceData, 256, 20);
    }

    // 如果没有数据，直接返回
    if (voiceData.isEmpty()) return;

    // 循环读取数据
    double voiceValue = voiceData[dataIndex_Voice];
    dataIndex_Voice = (dataIndex_Voice + 1) % voiceData.size();

    // 更新图表
    if (voiceCount > 300) {
        Series_Time_Voice->remove(0);
        axisX2_Voice->setMin(voiceCount - 300);
        axisX2_Voice->setMax(voiceCount);
    }

    Series_Time_Voice->append(voiceCount, voiceValue);
    voiceCount++;
}

void foc1::Vibration_onTimeout()
{
    // 数据库检查逻辑（新增部分）
    static double lastFirstVibrationValue = 0.0;
    static bool firstRunVibration = true;

    // 读取当前数据库的第一个振动数据值
    double currentFirstVibrationValue = 0.0;
    QList<double> firstValueList = my_sql->ReadDataFromTable("vibration", "foc_1data", 1); // 只读取1个值

    if (!firstValueList.isEmpty()) {
        currentFirstVibrationValue = firstValueList.first();
    } else {
        qDebug() << "Warning: No vibration data available for comparison";
    }

    // 检查数据是否有变化或首次运行（新增部分）
    bool dataChanged = (!firstRunVibration && currentFirstVibrationValue != lastFirstVibrationValue);

    if (firstRunVibration || dataChanged) {
        firstRunVibration = false;
        lastFirstVibrationValue = currentFirstVibrationValue;
        isDataLoaded_Vibration = false; // 强制重新加载数据
    }

    // 数据加载逻辑（保持原有结构，增加变化检测）
    if (!isDataLoaded_Vibration) {
        qDebug() << "Loading vibration data...";

        // 调试表结构（原有部分）
        QSqlQuery schemaQuery("PRAGMA table_info(foc_1data)", my_sql->db);
        while (schemaQuery.next()) {
            qDebug() << "Column" << schemaQuery.value(0).toInt()
                     << ":" << schemaQuery.value(1).toString()
                     << "Type:" << schemaQuery.value(2).toString();
        }

        // 从数据库读取振动数据（原有部分）
        vibrationData = my_sql->ReadDataFromTable("vibration", "foc_1data");
        qDebug() << "Read" << vibrationData.size() << "vibration data points";

        if (vibrationData.isEmpty()) {
            qDebug() << "Error: No valid vibration data found!";
            return;
        }

        // 动态计算Y轴范围（原有部分）
        auto [min, max] = std::minmax_element(vibrationData.begin(), vibrationData.end());
        qDebug() << "Y-axis range set to:" << *min - 0.05 << "-" << *max + 0.05;
        axisY2_Vibration->setRange(*min - 0.05, *max + 0.05);

        isDataLoaded_Vibration = true;
        qDebug() << "Successfully loaded vibration data";

        // 重置图表状态（新增部分）
        vibrationCount = 0;
        dataIndex_Vibration = 0;
        Series_Time_Vibration->clear();
        axisX2_Vibration->setMin(0);
        axisX2_Vibration->setMax(300);
        qDebug()<<"振动数据更新完成";
        performFFT(vibrationData, 256, 20);
    }

    // 如果没有数据，直接返回（原有部分）
    if (vibrationData.isEmpty()) return;

    // 数据循环显示逻辑（保持原有部分）
    double vibrationValue = vibrationData[dataIndex_Vibration];
    dataIndex_Vibration = (dataIndex_Vibration + 1) % vibrationData.size();

    // 更新图表（保持原有滚动显示逻辑）
    if (vibrationCount > 300) {
        Series_Time_Vibration->remove(0);
        axisX2_Vibration->setMin(vibrationCount - 300);
        axisX2_Vibration->setMax(vibrationCount);
    }
    Series_Time_Vibration->append(vibrationCount, vibrationValue);
    vibrationCount++;
}


void foc1::performFFT(const QVector<double>& data, int n, double cutoffFre)
{
    // 检查 n 是否为 2 的幂
    if ((n & (n - 1)) != 0 || n <= 0) {
        qDebug() << "错误：FFT输入点数 n 必须是正数且为 2 的幂，当前输入：" << n;
        return;
    }

    if (data.size() < n) {
        qDebug() << "错误：data 中的数据不足" << n << "个，仅有" << data.size() << "个";
        return;
    }

    // FFT 设置
    kiss_fft_cfg cfg = kiss_fft_alloc(n, 0, nullptr, nullptr);
    if (!cfg) {
        qDebug() << "kiss_fft_alloc 分配失败";
        return;
    }

    QVector<kiss_fft_cpx> in(n), out(n);
    for (int i = 0; i < n; ++i) {
        in[i].r = data[i];
        in[i].i = 0;
    }

    kiss_fft(cfg, in.data(), out.data());
    free(cfg);

    // FFT 可视化准备
    double sampleRate = 1000.0;  // 采样率，请根据实际系统设置
    double nyquistFreq = sampleRate / 2;
    double newMaxAmplitude = 0;

    QLineSeries* fftSeries = new QLineSeries();
    fftSeries->setName(QString("频域数据 %1").arg(chart2->series().count() + 1));

    for (int i = 0; i < n / 2; ++i) {
        double freq = i * sampleRate / n;

        if (freq < cutoffFre)
            continue;

        double magnitude = sqrt(out[i].r * out[i].r + out[i].i * out[i].i);
        fftSeries->append(freq, magnitude);

        if (magnitude > newMaxAmplitude) {
            newMaxAmplitude = magnitude;
        }
    }

    chart2->addSeries(fftSeries);

    // 坐标轴创建或获取
    QValueAxis* axisX = nullptr;
    QValueAxis* axisY = nullptr;

    if (chart2->axes(Qt::Horizontal).isEmpty()) {
        axisX = new QValueAxis();
        axisX->setTitleText("频率 (Hz)");
        chart2->addAxis(axisX, Qt::AlignBottom);
    } else {
        axisX = qobject_cast<QValueAxis*>(chart2->axes(Qt::Horizontal).first());
    }

    if (chart2->axes(Qt::Vertical).isEmpty()) {
        axisY = new QValueAxis();
        axisY->setTitleText("幅度");
        chart2->addAxis(axisY, Qt::AlignLeft);
    } else {
        axisY = qobject_cast<QValueAxis*>(chart2->axes(Qt::Vertical).first());
    }

    // 绑定曲线到坐标轴
    fftSeries->attachAxis(axisX);
    fftSeries->attachAxis(axisY);

    // 设置 X 轴范围
    axisX->setRange(cutoffFre, nyquistFreq);

    // 设置 Y 轴范围：动态适配所有曲线的最大幅值
    double globalMax = newMaxAmplitude;

    // 扫描已有曲线，寻找全局最大 Y 值
    for (auto* series : chart2->series()) {
        QLineSeries* s = qobject_cast<QLineSeries*>(series);
        if (!s) continue;

        for (const QPointF& p : s->points()) {
            if (p.y() > globalMax)
                globalMax = p.y();
        }
    }

    // 添加余量后设置 Y 轴范围
    axisY->setRange(0, globalMax * 1.1);

    qDebug() << "FFT 绘制完成，共" << fftSeries->count() << "个点，Y轴最大值：" << globalMax;
}


void foc1::ToggleSampleRate()
{
    currentSampleRateIndex = (currentSampleRateIndex + 1) % sampleRates.size();
    double newRate = sampleRates[currentSampleRateIndex];
    sampleRateButton->setText(QString("采样率: %1K").arg(newRate / 1000));
    // 这里可以添加采样率变更后的处理逻辑（如果需要）
}

void foc1::ToggleFFTSize()
{
    currentFFTSizeIndex = (currentFFTSizeIndex + 1) % fftSizes.size();
    int newSize = fftSizes[currentFFTSizeIndex];
    fftSizeButton->setText(QString("采样点数: %1").arg(newSize));
    // 这里可以添加FFT点数变更后的处理逻辑（如果需要）
}




