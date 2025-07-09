#include "itemform.h"

ItemForm::ItemForm(QWidget *parent)
    : QWidget{parent}
{
    HorizonalLayout = new QHBoxLayout(this);
    TypeLabel = new QLabel(this);
    IdLabel = new QLabel(this);
    CurrentLabel = new QLabel(this);
    VoiceLabel = new QLabel(this);
    ShakeLabel = new QLabel(this);
    HorizonalLayout->addWidget(TypeLabel);
    HorizonalLayout->addWidget(IdLabel);
    HorizonalLayout->addWidget(CurrentLabel);
    HorizonalLayout->addWidget(VoiceLabel);
    HorizonalLayout->addWidget(ShakeLabel);
}

void ItemForm::StaffItem(QString type,int id,float current,float voice,float shake)
{
    TypeLabel->setText(type);
    IdLabel->setText(QString::number(id));
    CurrentLabel->setText(QString::number(static_cast<double>(current))+"A");
    VoiceLabel->setText(QString::number(static_cast<double>(voice))+"V");
    ShakeLabel->setText(QString::number(static_cast<double>(shake))+"V");
}
