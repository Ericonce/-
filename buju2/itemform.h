#ifndef ITEMFORM_H
#define ITEMFORM_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

class ItemForm : public QWidget
{
    Q_OBJECT
public:
    explicit ItemForm(QWidget *parent = nullptr);

signals:

public:
    void StaffItem(QString type,int id,float current,float voice,float shake);

private:
    QHBoxLayout *HorizonalLayout;
    QLabel *TypeLabel;
    QLabel *IdLabel;
    QLabel *CurrentLabel;
    QLabel *VoiceLabel;
    QLabel *ShakeLabel;
};

#endif // ITEMFORM_H
