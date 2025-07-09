#include "widget.h"
#include <QApplication>
#include <QPalette>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 设置全局背景为白色
    //QPalette palette;
    //palette.setColor(QPalette::Window, Qt::white);  // 所有 QWidget 背景
    //palette.setColor(QPalette::Base, Qt::white);    // QLineEdit、QTextEdit 背景
    //palette.setColor(QPalette::Button, Qt::white);  // QPushButton 背景
    //a.setPalette(palette);

    Widget w;
    w.show();

    return a.exec();
}
