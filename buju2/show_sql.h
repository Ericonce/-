#ifndef SHOW_SQL_H
#define SHOW_SQL_H

#include <QWidget>
#include <QSqlDatabase>
#include <QtDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QLabel>

#include "itemform.h"

class show_sql : public QWidget
{
    Q_OBJECT

public:
    QSqlDatabase db;

private:
    QPushButton *btn1;
    QPushButton *btn2;
    QPushButton *btn3;
    QPushButton *btn4;
    QPushButton *btn5;
    QPushButton *btn6;

    QPushButton *btn_foc_1;
    QPushButton *btn_foc_2;
    QPushButton *btn_foc_3;
    QPushButton *btn_foc_4;

    QListWidget *ListWidget;
    QWidget *BtnWidget;
    QVBoxLayout *GreatLayout;
    QVBoxLayout *BtnLayout;
    QHBoxLayout *MainLayout;
    QHBoxLayout *LabelLayout;
    QListWidgetItem *ListWidgetItem;

    QLabel *Label_Name;
    QLabel *Label_ID;
    QLabel *Label_Current;
    QLabel *Label_Voice;
    QLabel *Label_Shake;


public:
    explicit show_sql(QWidget *parent = nullptr);
    ~show_sql();

public:
    void initUI();

public://查询函数
    void Select_Dcm1();
    void Select_Dcm2();
    void Select_Dcm3();
    void Select_Dcm4();
    void Select_Foc1();
    void Select_Foc2();
    void Select_Foc3();
    void Select_Foc4();

public:
    void open_database();
    void notifyDataChanged();
    QVector<double> ReadDataFromTable(QString type,QString TableName);//从表中读取数据
    QVector<double> ReadDataFromTable(QString type, QString Table_Name, int limit);
    bool Dataloadfromcsv(const QString &csv_path); //从csv读取数据

signals:
    void dataUpdated();  // 新增信号
};

#endif // SHOW_SQL_H
