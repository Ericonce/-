#include "show_sql.h"

show_sql::show_sql(QWidget *parent)
{
    open_database();
    initUI();
}

show_sql::~show_sql(){}

void show_sql::open_database()
{
    db = QSqlDatabase::addDatabase("QSQLITE");//连接数据库
    db.setDatabaseName("mydatabase.db");
    if(!db.open())
    {
        qDebug()<<"Error failed to open"<<db.lastError();
    }
    else
    {
        qDebug()<<"open success!";
    }
}

void show_sql::notifyDataChanged() {
    emit dataUpdated();  // 触发信号
}

QVector<double> show_sql::ReadDataFromTable(QString type, QString Table_Name, int limit)
{
    QVector<double> data;
    QString columnName;

    // 根据类型确定列名
    if (type == "current") {
        columnName = "current";
    } else if (type == "voice") {
        columnName = "voice";
    } else if (type == "vibration") {
        columnName = "shake";  // 注意：表中列名是shake，而不是vibration
    } else {
        qDebug() << "Error: Unknown data type:" << type;
        return data;
    }

    // 构建SQL查询，只选择需要的列并限制数量
    QString queryStr = QString("SELECT %1 FROM %2 WHERE %1 IS NOT NULL ORDER BY id LIMIT %3")
                           .arg(columnName, Table_Name, QString::number(limit));

    QSqlQuery query(queryStr, db);

    while (query.next()) {
        bool ok;
        double val = query.value(0).toDouble(&ok);

        if (!ok) {
            qDebug() << "Invalid data at row" << query.at()
                     << ":" << query.value(0).toString();
            continue;
        }

        data.append(val);
    }

    //qDebug() << "Read" << data.size() << "records for type" << type;
    //qDebug()<<data;
    return data;
}

QVector<double> show_sql::ReadDataFromTable(QString type,QString Table_Name)
{
    QVector<double> data;

    QSqlQuery query("SELECT * FROM " + Table_Name, db);

    if(type == "current")
        while (query.next()) {
            bool ok;
            double val = query.value(2).toDouble(&ok);  // 第三列索引为2
            if (!ok) {
                //qDebug() << "Invalid data at row" << query.at()
                 //            << ":" << query.value(2).toString();
                continue;
            }
            data.append(val);
        }

    else if(type == "voice")
        while (query.next()) {
            bool ok;
            double val = query.value(3).toDouble(&ok);  // 第三列索引为2
            if (!ok) {
                qDebug() << "Invalid data at row" << query.at()
                         << ":" << query.value(3).toString();
                continue;
            }
            data.append(val);
        }

    else if(type == "vibration")
        while (query.next()) {
            bool ok;
            double val = query.value(4).toDouble(&ok);  // 第三列索引为2
            if (!ok) {
                qDebug() << "Invalid data at row" << query.at()
                         << ":" << query.value(4).toString();
                continue;
            }
            data.append(val);
        }
    //qDebug()<<"----------------------------------------------曲线数据制成------------------------------------------";
    return data;
}

void show_sql::initUI()
{
    GreatLayout = new QVBoxLayout(this);
    LabelLayout = new QHBoxLayout(this);
    MainLayout = new QHBoxLayout(this);

    GreatLayout->addLayout(LabelLayout);
    GreatLayout->addLayout(MainLayout);

    // 标签样式设置
    QString labelStyle = R"(
        QLabel {
            font: bold 14px;
            color: #333333;
            padding: 5px 10px;
        }
    )";

    Label_Name = new QLabel("电机", this);
    Label_ID = new QLabel("序号", this);
    Label_Current = new QLabel("电流数据", this);
    Label_Voice = new QLabel("声音数据", this);
    Label_Shake = new QLabel("振动数据", this);

    // 应用标签样式
    Label_Name->setStyleSheet(labelStyle);
    Label_ID->setStyleSheet(labelStyle);
    Label_Current->setStyleSheet(labelStyle);
    Label_Voice->setStyleSheet(labelStyle);
    Label_Shake->setStyleSheet(labelStyle);

    // 加 spacer 微调（保持原位置不变）
    QSpacerItem *spacerLeft = new QSpacerItem(30, 0, QSizePolicy::Fixed, QSizePolicy::Minimum);
    QSpacerItem *spacerRight = new QSpacerItem(200, 0, QSizePolicy::Fixed, QSizePolicy::Minimum);

    LabelLayout->addSpacerItem(spacerLeft);
    LabelLayout->addWidget(Label_Name);
    LabelLayout->addWidget(Label_ID);
    LabelLayout->addWidget(Label_Current);
    LabelLayout->addWidget(Label_Voice);
    LabelLayout->addWidget(Label_Shake);
    LabelLayout->addSpacerItem(spacerRight);

    ListWidget = new QListWidget(this);
    BtnWidget = new QWidget(this);
    BtnLayout = new QVBoxLayout(BtnWidget);
    MainLayout->addWidget(ListWidget);
    MainLayout->addWidget(BtnWidget);

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

    // 创建按钮并应用新样式
    btn1 = new QPushButton("查询1号电机", BtnWidget);  // 文字也稍微缩短
    btn2 = new QPushButton("查询2号电机", BtnWidget);
    btn3 = new QPushButton("查询3号电机", BtnWidget);
    btn4 = new QPushButton("查询4号电机", BtnWidget);

    btn5 = new QPushButton("导出数据", BtnWidget);
    btn6 = new QPushButton("删除数据", BtnWidget);

    btn_foc_1 = new QPushButton("查询5号电机", BtnWidget);
    btn_foc_2 = new QPushButton("查询6号电机", BtnWidget);
    btn_foc_3 = new QPushButton("查询7号电机", BtnWidget);
    btn_foc_4 = new QPushButton("查询8号电机", BtnWidget);

    btn1->setStyleSheet(buttonStyle);
    btn2->setStyleSheet(buttonStyle);
    btn3->setStyleSheet(buttonStyle);
    btn4->setStyleSheet(buttonStyle);
    btn5->setStyleSheet(buttonStyle);
    btn6->setStyleSheet(buttonStyle);

    btn_foc_1->setStyleSheet(buttonStyle);
    btn_foc_2->setStyleSheet(buttonStyle);
    btn_foc_3->setStyleSheet(buttonStyle);
    btn_foc_4->setStyleSheet(buttonStyle);


    // 更紧凑的按钮布局
    //BtnLayout->setSpacing(8);           // 减小按钮间距
    //BtnLayout->setContentsMargins(2, 2, 2, 2);  // 减小边距

    BtnLayout->addWidget(btn1);
    BtnLayout->addWidget(btn2);
    BtnLayout->addWidget(btn3);
    BtnLayout->addWidget(btn4);
    BtnLayout->addWidget(btn_foc_1);
    BtnLayout->addWidget(btn_foc_2);
    BtnLayout->addWidget(btn_foc_3);
    BtnLayout->addWidget(btn_foc_4);
    BtnLayout->addWidget(btn5);
    BtnLayout->addWidget(btn6);

    // 调整按钮间距（缩小）
    //BtnLayout->setSpacing(10);
    //BtnLayout->setContentsMargins(3, 3, 3, 3);

    connect(btn1, &QPushButton::clicked, this, &show_sql::Select_Dcm1);
    connect(btn2, &QPushButton::clicked, this, &show_sql::Select_Dcm2);
    connect(btn3, &QPushButton::clicked, this, &show_sql::Select_Dcm3);
    connect(btn4, &QPushButton::clicked, this, &show_sql::Select_Dcm4);
    connect(btn_foc_1, &QPushButton::clicked, this, &show_sql::Select_Foc1);
    connect(btn_foc_2, &QPushButton::clicked, this, &show_sql::Select_Foc2);
    connect(btn_foc_3, &QPushButton::clicked, this, &show_sql::Select_Foc3);
    connect(btn_foc_4, &QPushButton::clicked, this, &show_sql::Select_Foc4);

    ListWidgetItem = new QListWidgetItem();
}

void show_sql::Select_Dcm1()
{
    ListWidget->clear();

    QSqlQuery query;
    QString sqlSelect = QString("select * from dcm_1data");
    if(!query.exec(sqlSelect))
        qDebug()<<"Select data error"<<db.lastError();
    else
        while(query.next())
        {
            QString type = query.value("name").toString();
            int id = query.value("id").toInt();
            float current = query.value("current").toFloat();
            float voice = query.value("voice").toFloat();
            float shake = query.value("shake").toFloat();
            ItemForm *staffitem = new ItemForm;
            staffitem->StaffItem(type,id,current,voice,shake);
            QListWidgetItem *item = new QListWidgetItem;
            item->setSizeHint(QSize(574,51));
            ListWidget->addItem(item);
            ListWidget->setItemWidget(item,staffitem);
        }
}

void show_sql::Select_Dcm2()
{
    ListWidget->clear();

    QSqlQuery query;
    QString sqlSelect = QString("select * from dcm_2data");
    if(!query.exec(sqlSelect))
        qDebug()<<"Select data error"<<db.lastError();
    else
        while(query.next())
        {
            QString type = query.value("name").toString();
            int id = query.value("id").toInt();
            float current = query.value("current").toFloat();
            float voice = query.value("voice").toFloat();
            float shake = query.value("shake").toFloat();
            ItemForm *staffitem = new ItemForm;
            staffitem->StaffItem(type,id,current,voice,shake);
            QListWidgetItem *item = new QListWidgetItem;
            item->setSizeHint(QSize(574,51));
            ListWidget->addItem(item);
            ListWidget->setItemWidget(item,staffitem);
        }
}

void show_sql::Select_Dcm3()
{
    ListWidget->clear();

    QSqlQuery query;
    QString sqlSelect = QString("select * from dcm_3data");
    if(!query.exec(sqlSelect))
        qDebug()<<"Select data error"<<db.lastError();
    else
        while(query.next())
        {
            QString type = query.value("name").toString();
            int id = query.value("id").toInt();
            float current = query.value("current").toFloat();
            float voice = query.value("voice").toFloat();
            float shake = query.value("shake").toFloat();
            ItemForm *staffitem = new ItemForm;
            staffitem->StaffItem(type,id,current,voice,shake);
            QListWidgetItem *item = new QListWidgetItem;
            item->setSizeHint(QSize(574,51));
            ListWidget->addItem(item);
            ListWidget->setItemWidget(item,staffitem);
        }
}

void show_sql::Select_Dcm4()
{
    ListWidget->clear();

    QSqlQuery query;
    QString sqlSelect = QString("select * from dcm_4data");
    if(!query.exec(sqlSelect))
        qDebug()<<"Select data error"<<db.lastError();
    else
        while(query.next())
        {
            QString type = query.value("name").toString();
            int id = query.value("id").toInt();
            float current = query.value("current").toFloat();
            float voice = query.value("voice").toFloat();
            float shake = query.value("shake").toFloat();
            ItemForm *staffitem = new ItemForm;
            staffitem->StaffItem(type,id,current,voice,shake);
            QListWidgetItem *item = new QListWidgetItem;
            item->setSizeHint(QSize(574,51));
            ListWidget->addItem(item);
            ListWidget->setItemWidget(item,staffitem);
        }
}

void show_sql::Select_Foc1()
{
    ListWidget->clear();

    QSqlQuery query;
    QString sqlSelect = QString("select * from foc_1data");
    if(!query.exec(sqlSelect))
        qDebug()<<"Select data error"<<db.lastError();
    else
        while(query.next())
        {
            QString type = query.value("name").toString();
            int id = query.value("id").toInt();
            float current = query.value("current").toFloat();
            float voice = query.value("voice").toFloat();
            float shake = query.value("shake").toFloat();
            ItemForm *staffitem = new ItemForm;
            staffitem->StaffItem(type,id,current,voice,shake);
            QListWidgetItem *item = new QListWidgetItem;
            item->setSizeHint(QSize(574,51));
            ListWidget->addItem(item);
            ListWidget->setItemWidget(item,staffitem);
        }
}

void show_sql::Select_Foc2()
{
    ListWidget->clear();

    QSqlQuery query;
    QString sqlSelect = QString("select * from foc_2data");
    if(!query.exec(sqlSelect))
        qDebug()<<"Select data error"<<db.lastError();
    else
        while(query.next())
        {
            QString type = query.value("name").toString();
            int id = query.value("id").toInt();
            float current = query.value("current").toFloat();
            float voice = query.value("voice").toFloat();
            float shake = query.value("shake").toFloat();
            ItemForm *staffitem = new ItemForm;
            staffitem->StaffItem(type,id,current,voice,shake);
            QListWidgetItem *item = new QListWidgetItem;
            item->setSizeHint(QSize(574,51));
            ListWidget->addItem(item);
            ListWidget->setItemWidget(item,staffitem);
        }
}

void show_sql::Select_Foc3()
{
    ListWidget->clear();

    QSqlQuery query;
    QString sqlSelect = QString("select * from foc_3data");
    if(!query.exec(sqlSelect))
        qDebug()<<"Select data error"<<db.lastError();
    else
        while(query.next())
        {
            QString type = query.value("name").toString();
            int id = query.value("id").toInt();
            float current = query.value("current").toFloat();
            float voice = query.value("voice").toFloat();
            float shake = query.value("shake").toFloat();
            ItemForm *staffitem = new ItemForm;
            staffitem->StaffItem(type,id,current,voice,shake);
            QListWidgetItem *item = new QListWidgetItem;
            item->setSizeHint(QSize(574,51));
            ListWidget->addItem(item);
            ListWidget->setItemWidget(item,staffitem);
        }
}

void show_sql::Select_Foc4()
{
    ListWidget->clear();

    QSqlQuery query;
    QString sqlSelect = QString("select * from foc_4data");
    if(!query.exec(sqlSelect))
        qDebug()<<"Select data error"<<db.lastError();
    else
        while(query.next())
        {
            QString type = query.value("name").toString();
            int id = query.value("id").toInt();
            float current = query.value("current").toFloat();
            float voice = query.value("voice").toFloat();
            float shake = query.value("shake").toFloat();
            ItemForm *staffitem = new ItemForm;
            staffitem->StaffItem(type,id,current,voice,shake);
            QListWidgetItem *item = new QListWidgetItem;
            item->setSizeHint(QSize(574,51));
            ListWidget->addItem(item);
            ListWidget->setItemWidget(item,staffitem);
        }
}

bool show_sql::Dataloadfromcsv(const QString &csv_path)
{
    // 获取CSV文件名（不含路径和扩展名）作为表名
    QFileInfo fileInfo(csv_path);
    QString tableName = fileInfo.baseName(); // 如"type.csv"变为"type"

    // 1. 删除已存在的表（实现覆盖）
    QSqlQuery dropQuery(db);
    if (!dropQuery.exec(QString("DROP TABLE IF EXISTS %1").arg(tableName))) {
        qDebug() << "Failed to drop table:" << dropQuery.lastError();
        return false;
    }

    // 2. 创建新表（使用动态表名）
    QSqlQuery createQuery(db);
    QString createSql = QString(
                            "CREATE TABLE %1 ("
                            "name TEXT, "       // 存储表名本身
                            "id INTEGER, "      // 行号
                            "current REAL, "    // 第一列数据
                            "voice REAL, "      // 第二列数据
                            "shake REAL)"       // 第三列数据
                            ).arg(tableName);

    if (!createQuery.exec(createSql)) {
        qDebug() << "Failed to create table:" << createQuery.lastError();
        return false;
    }

    // 3. 读取CSV文件
    QFile file(csv_path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Cannot open CSV file:" << file.errorString();
        return false;
    }

    QTextStream in(&file);
    int lineNumber = 1; // 行号作为ID

    // 4. 使用事务提高插入性能
    db.transaction();
    QSqlQuery insertQuery(db);
    insertQuery.prepare(
        QString("INSERT INTO %1 (name, id, current, voice, shake) VALUES (?, ?, ?, ?, ?)")
            .arg(tableName)
        );

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList values = line.split(" ", Qt::SkipEmptyParts);
        if (values.size() != 3) {
            qDebug() << "Skipping malformed line" << lineNumber << ":" << line;
            continue;
        }

        // 绑定参数
        insertQuery.addBindValue(tableName);    // name字段=表名
        insertQuery.addBindValue(lineNumber);  // id
        insertQuery.addBindValue(values[0]);   // current
        insertQuery.addBindValue(values[1]);   // voice
        insertQuery.addBindValue(values[2]);   // shake

        if (!insertQuery.exec()) {
            qDebug() << "Insert failed at line" << lineNumber
                     << ":" << insertQuery.lastError();
            db.rollback();
            file.close();
            return false;
        }

        lineNumber++;
    }

    // 5. 提交事务
    if (!db.commit()) {
        qDebug() << "Commit failed:" << db.lastError();
        file.close();
        return false;
    }

    file.close();
    qDebug() << QString("Successfully imported %1 records into table %2")
                    .arg(lineNumber-1)
                    .arg(tableName);
    return true;
}






