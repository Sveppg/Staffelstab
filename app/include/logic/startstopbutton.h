#ifndef STARTSTOPBUTTON_H
#define STARTSTOPBUTTON_H

#include <QObject>

class StartStopButton : public QObject
{
    Q_OBJECT

public:
    explicit StartStopButton(QObject *parent = nullptr);

    void druecken();
    void knopfAuslesen(bool istGedrueckt);

signals:
    void gedrueckt();

private:
    bool vorherGedrueckt = false;
};

#endif