    /****************************************************************************
    **
    ** This file is part of the Qt Extended Commercial Package.
    **
    ** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
    **
    ** $QT_EXTENDED_DUAL_LICENSE$
    **
    ****************************************************************************/

    #ifndef SIMPLEPLAYER_H
    #define SIMPLEPLAYER_H

    #include <QDialog>
    #include <QPixmap>
    #include <QTimer>
    #include <QKeyEvent>

    #include "ui_simpleplayerbase.h"

    #include "basicmedia.h"

    class QTimer;
    class QPixmap;
    class QDocumentSelectorDialog;

    class SimplePlayer : public QWidget, public Ui_SimplePlayerBase
    {
        Q_OBJECT
    public:
        SimplePlayer(QWidget* p, Qt::WFlags f);
        ~SimplePlayer();

        void showEvent( QShowEvent *e );

    public slots:
        void newFile();
        void fileSelector();
        void play();
        void pause();
        void stop();

    protected:
        void keyReleaseEvent( QKeyEvent * );

    private:
        BasicMedia*               picture;
        QDocumentSelectorDialog*  docs;
    };

    #endif
