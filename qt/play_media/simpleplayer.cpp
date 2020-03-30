    /****************************************************************************
    **
    ** This file is part of the Qt Extended Commercial Package.
    **
    ** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
    **
    ** $QT_EXTENDED_DUAL_LICENSE$
    **
    ****************************************************************************/

    #include "simpleplayer.h"
    #include "basicmedia.h"

    #include <QDesktopWidget>
    #include <QApplication>

    #include <QMediaControl>

    #include <QDocumentSelectorDialog>
    #include <QtopiaApplication>

    SimplePlayer::SimplePlayer(QWidget* p, Qt::WFlags f)
        : QWidget(p, f)
    {
        setupUi(this);

        connect(fileButton, SIGNAL(clicked()), this, SLOT(fileSelector()));
        connect(playButton, SIGNAL(clicked()), this, SLOT(play()));
        connect(pauseButton, SIGNAL(clicked()), this, SLOT(pause()));
        connect(stopButton, SIGNAL(clicked()), this, SLOT(stop()));

        docs = new QDocumentSelectorDialog(this);
        docs->setFilter(
                QContentFilter( QContentFilter::MimeType, "audio/*" )
                | QContentFilter( QContentFilter::MimeType, "video/*" ));

        connect(docs,SIGNAL(accepted()),this,SLOT(newFile()));
        QtopiaApplication::showDialog(docs);
    }

    SimplePlayer::~SimplePlayer()
    {
        PlayScreen->stop();
        PlayScreen->deleteLater();
    }

    void SimplePlayer::keyReleaseEvent( QKeyEvent *ke )
    {
        switch(ke->key())  {
            case  Qt::Key_Backspace:
                ke->ignore();
                break;
            case  Qt::Key_Back:
                ke->ignore();
                break;
            default:
                ke->ignore();
                break;
        };
    }

    void SimplePlayer::showEvent(QShowEvent *)
    {
        QDesktopWidget *desktop = QApplication::desktop();
        QRect desk = desktop->screenGeometry(desktop->primaryScreen());
        setGeometry(desk);
        setVisible(true);
        setFocusPolicy( Qt::StrongFocus );
        setFocus();
        showFullScreen();
    }

    void SimplePlayer::newFile()
    {
        QContent c = docs->selectedDocument();
        PlayScreen->stop();
        PlayScreen->setFilename(c.fileName());
        PlayScreen->start();
    }

    void SimplePlayer::fileSelector()
    {
        QtopiaApplication::showDialog(docs);
    }

    void SimplePlayer::play()
    {
        PlayScreen->start();
    }

    void SimplePlayer::pause()
    {
        PlayScreen->stop();
    }

    void SimplePlayer::stop()
    {
        PlayScreen->stop();
    }

