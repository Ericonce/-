#include "mychartview.h"

Mychartview::Mychartview(QWidget *parent)
    : QChartView{parent}
{}

void Mychartview::mouseMoveEvent(QMouseEvent *event){
    QPoint point=event->pos();
    emit mouseMovePoint(point);//发送自定义信号
    QChartView::mouseMoveEvent(event);
}

void Mychartview::mousePressEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton)
    {
        this->setDragMode(QGraphicsView::RubberBandDrag);//设置成框选
        begin_point = event->pos();
    }
    QChartView::mousePressEvent(event);//调用基类实现可以继承 QChartView 原有的鼠标操作
}

void Mychartview::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton)
    {
        end_point = event->pos();
        if(end_point.x()>begin_point.x())
        {
            QRectF rectf;
            rectf.setTopLeft(begin_point);
            rectf.setBottomRight(end_point);
            this->chart()->zoomIn(rectf);
        }
        else if (end_point.x()<begin_point.x())
        {
            this->chart()->zoomReset();
        }
    }
    QChartView::mouseReleaseEvent(event);
}
