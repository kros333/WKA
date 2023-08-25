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
    bool errors = false;
    bool rain = false;

private slots:
    void on_checkBox_stateChanged(int arg1);

    void on_checkBox_2_stateChanged(int arg1);

private:
    Ui::WeatherWidget *ui;
};

#endif // WEATHERWIDGET_H
