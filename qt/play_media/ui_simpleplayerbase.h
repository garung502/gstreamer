/********************************************************************************
** Form generated from reading UI file 'simpleplayerbase.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SIMPLEPLAYERBASE_H
#define UI_SIMPLEPLAYERBASE_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>
#include "basicmedia.h"

QT_BEGIN_NAMESPACE

class Ui_SimplePlayerBase
{
public:
    QGridLayout *gridLayout;
    QWidget *widget;
    QGridLayout *gridLayout1;
    QLabel *label;
    QPushButton *fileButton;
    QPushButton *playButton;
    QPushButton *pauseButton;
    QPushButton *stopButton;
    BasicMedia *PlayScreen;

    void setupUi(QWidget *SimplePlayerBase)
    {
        if (SimplePlayerBase->objectName().isEmpty())
            SimplePlayerBase->setObjectName(QStringLiteral("SimplePlayerBase"));
        SimplePlayerBase->resize(367, 452);
        SimplePlayerBase->setMinimumSize(QSize(0, 0));
        gridLayout = new QGridLayout(SimplePlayerBase);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        gridLayout->setHorizontalSpacing(0);
        gridLayout->setVerticalSpacing(0);
        gridLayout->setContentsMargins(0, 0, 0, 0);
        widget = new QWidget(SimplePlayerBase);
        widget->setObjectName(QStringLiteral("widget"));
        widget->setWindowModality(Qt::NonModal);
        widget->setContextMenuPolicy(Qt::NoContextMenu);
        gridLayout1 = new QGridLayout(widget);
        gridLayout1->setObjectName(QStringLiteral("gridLayout1"));
        label = new QLabel(widget);
        label->setObjectName(QStringLiteral("label"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy);
        label->setMinimumSize(QSize(0, 40));

        gridLayout1->addWidget(label, 0, 0, 1, 4);

        fileButton = new QPushButton(widget);
        fileButton->setObjectName(QStringLiteral("fileButton"));

        gridLayout1->addWidget(fileButton, 1, 0, 1, 1);

        playButton = new QPushButton(widget);
        playButton->setObjectName(QStringLiteral("playButton"));
        const QIcon icon = QIcon(QString::fromUtf8("pics/play.png"));
        playButton->setIcon(icon);

        gridLayout1->addWidget(playButton, 1, 1, 1, 1);

        pauseButton = new QPushButton(widget);
        pauseButton->setObjectName(QStringLiteral("pauseButton"));
        const QIcon icon1 = QIcon(QString::fromUtf8("pics/pause.png"));
        pauseButton->setIcon(icon1);

        gridLayout1->addWidget(pauseButton, 1, 2, 1, 1);

        stopButton = new QPushButton(widget);
        stopButton->setObjectName(QStringLiteral("stopButton"));
        const QIcon icon2 = QIcon(QString::fromUtf8("pics/stop.png"));
        stopButton->setIcon(icon2);

        gridLayout1->addWidget(stopButton, 1, 3, 1, 1);

        PlayScreen = new BasicMedia(widget);
        PlayScreen->setObjectName(QStringLiteral("PlayScreen"));
        QPalette palette;
        QBrush brush(QColor(0, 0, 0, 255));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Base, brush);
        palette.setBrush(QPalette::Active, QPalette::Window, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Base, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Window, brush);
        palette.setBrush(QPalette::Disabled, QPalette::Base, brush);
        palette.setBrush(QPalette::Disabled, QPalette::Window, brush);
        PlayScreen->setPalette(palette);
        PlayScreen->setAutoFillBackground(true);

        gridLayout1->addWidget(PlayScreen, 2, 0, 1, 4);


        gridLayout->addWidget(widget, 0, 0, 1, 1);


        retranslateUi(SimplePlayerBase);

        QMetaObject::connectSlotsByName(SimplePlayerBase);
    } // setupUi

    void retranslateUi(QWidget *SimplePlayerBase)
    {
        SimplePlayerBase->setWindowTitle(QApplication::translate("SimplePlayerBase", "Form", 0));
        label->setText(QString());
        fileButton->setText(QApplication::translate("SimplePlayerBase", "File", 0));
        playButton->setText(QString());
        pauseButton->setText(QString());
        stopButton->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class SimplePlayerBase: public Ui_SimplePlayerBase {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SIMPLEPLAYERBASE_H
