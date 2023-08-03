#include "weather.h"
#include "ui_weather.h"

Weather::Weather(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Weather)
{
    ui->setupUi(this);
}

Weather::~Weather()
{
    delete ui;
}
