    /****************************************************************************
    **
    ** This file is part of the Qt Extended Commercial Package.
    **
    ** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
    **
    ** $QT_EXTENDED_DUAL_LICENSE$
    **
    ****************************************************************************/

    #ifndef BASICMEDIA_H
    #define BASICMEDIA_H

    #include <QWidget>
    #include <QString>

    class QMediaContent;
    class QMediaControl;
    class QMediaContentContext;
    class QMediaVideoControl;
    class QVBoxLayout;

    class BasicMedia : public QWidget
    {
    Q_OBJECT

    public:
        BasicMedia(QWidget* parent);
        ~BasicMedia();

        void stop();
        void start();
        void setFilename(QString filename);
        void volumeup();
        void volumedown();

    protected:
        void keyReleaseEvent( QKeyEvent * );

    private:
        QMediaContentContext* context;
        QMediaContent*        m_mediaContent;
        QMediaControl*        m_control;
        QMediaVideoControl*   m_video;

        int  m_state;
        int  volume;

    private slots:
        void mediaVideoControlValid();
        void mediaControlValid();

    private:
        QWidget*     videoWidget;
        QString      vfile;
        QVBoxLayout* layout;
    };
    #endif

