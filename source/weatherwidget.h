#ifndef WEATHERWIDGET_H
#define WEATHERWIDGET_H

#include <QWidget>

namespace Ui {
class WeatherWidget;
}

class WeatherWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WeatherWidget(QWidget *parent = nullptr);
    void updateText(QString s);
    ~WeatherWidget();


private:
    Ui::WeatherWidget *ui;
};

#endif // WEATHERWIDGET_H
