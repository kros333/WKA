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



