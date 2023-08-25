/********************************************************************************
** Form generated from reading UI file 'controlwidget.ui'
**
** Created by: Qt User Interface Compiler version 5.12.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CONTROLWIDGET_H
#define UI_CONTROLWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ControlWidget
{
public:
    QCheckBox *checkBox;
    QCheckBox *checkBox_2;

    void setupUi(QWidget *ControlWidget)
    {
        if (ControlWidget->objectName().isEmpty())
            ControlWidget->setObjectName(QString::fromUtf8("ControlWidget"));
        ControlWidget->resize(105, 88);
        checkBox = new QCheckBox(ControlWidget);
        checkBox->setObjectName(QString::fromUtf8("checkBox"));
        checkBox->setGeometry(QRect(20, 20, 72, 19));
        checkBox_2 = new QCheckBox(ControlWidget);
        checkBox_2->setObjectName(QString::fromUtf8("checkBox_2"));
        checkBox_2->setGeometry(QRect(20, 50, 72, 19));

        retranslateUi(ControlWidget);

        QMetaObject::connectSlotsByName(ControlWidget);
    } // setupUi

    void retranslateUi(QWidget *ControlWidget)
    {
        ControlWidget->setWindowTitle(QApplication::translate("ControlWidget", "Form", nullptr));
        checkBox->setText(QApplication::translate("ControlWidget", "\320\224\320\276\320\266\320\264\321\214", nullptr));
        checkBox_2->setText(QApplication::translate("ControlWidget", "\320\236\321\210\320\270\320\261\320\272\320\260", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ControlWidget: public Ui_ControlWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CONTROLWIDGET_H
