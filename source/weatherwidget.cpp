#include "weatherwidget.h"
#include "ui_weatherwidget.h"
#include "ui_weatherwidget.h"



WeatherWidget::WeatherWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WeatherWidget)
{
    ui->setupUi(this);
}

WeatherWidget::~WeatherWidget()
{
    delete ui;
}

void WeatherWidget::updateText(QString s)
{
   ui->label->setText(s);
}




void WeatherWidget::on_checkBox_stateChanged(int arg1)
{
    if(rain)
    {
        rain = false;
    }
    else
    {
        rain = true;
    }
}

void WeatherWidget::on_checkBox_2_stateChanged(int arg1)
{
    if(errors)
    {
        errors = false;
    }
    else
    {
        errors = true;
    }
}
