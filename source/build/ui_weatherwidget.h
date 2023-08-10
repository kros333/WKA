/********************************************************************************
** Form generated from reading UI file 'weatherwidget.ui'
**
** Created by: Qt User Interface Compiler version 5.12.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WEATHERWIDGET_H
#define UI_WEATHERWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_WeatherWidget
{
public:
    QLabel *label;

    void setupUi(QWidget *WeatherWidget)
    {
        if (WeatherWidget->objectName().isEmpty())
            WeatherWidget->setObjectName(QString::fromUtf8("WeatherWidget"));
        WeatherWidget->resize(440, 460);
        WeatherWidget->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 0, 0);"));
        label = new QLabel(WeatherWidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(0, 0, 440, 460));
        QFont font;
        font.setFamily(QString::fromUtf8("Courier New"));
        font.setPointSize(14);
        label->setFont(font);
        label->setStyleSheet(QString::fromUtf8("color: rgb(255, 255, 255);"));
        label->setTextFormat(Qt::AutoText);
        label->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        label->setWordWrap(false);

        retranslateUi(WeatherWidget);

        QMetaObject::connectSlotsByName(WeatherWidget);
    } // setupUi

    void retranslateUi(QWidget *WeatherWidget)
    {
        WeatherWidget->setWindowTitle(QApplication::translate("WeatherWidget", "Form", nullptr));
        label->setText(QApplication::translate("WeatherWidget", "\320\227\320\260\320\263\321\200\321\203\320\267\320\272\320\260...", nullptr));
    } // retranslateUi

};

namespace Ui {
    class WeatherWidget: public Ui_WeatherWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WEATHERWIDGET_H
