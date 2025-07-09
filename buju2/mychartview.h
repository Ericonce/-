#ifndef MYCHARTVIEW_H
#define MYCHARTVIEW_H

#include <QWidget>
#include <QPoint>
#include <QChartView>

class Mychartview : public QChartView
{
    Q_OBJECT
public:
    explicit Mychartview(QWidget *parent = nullptr);

signals:
    void mouseMovePoint(QPoint point);//用来给MainWindow发送信号

protected:
    void mouseMoveEvent(QMouseEvent *event);//鼠标移动事件,用于发送信号
    void mousePressEvent(QMouseEvent *event);//鼠标按下事件
    void mouseReleaseEvent(QMouseEvent *event);//鼠标松开事件

private:
    QPoint begin_point;//框选起点
    QPoint end_point;
};

#endif // MYCHARTVIEW_H
