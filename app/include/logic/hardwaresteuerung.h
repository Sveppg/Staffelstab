#pragma once

#include <QObject>


#include "logic/startstopbutton.h"
#include "logic/menuebutton.h"
#include "logic/hardwaredaten.h"
#include "logic/vibrationsmotor.h"
#include "logic/display.h"
#include "logic/ledRing.h"
#include "logic/ladehub.h"


class HardwareSteuerung : public QObject
{
    Q_OBJECT

private:

    StartStopButton startButton;
    MenueButton menueButton;
    HardwareDaten daten;
    Vibrationsmotor vibrationsmotor;
    Ladehub ladehub;
    sDisplay display;
    LEDRing ledRing;

public:

    explicit HardwareSteuerung(QObject *parent = nullptr);

    const HardwareDaten& getHardwareDaten() const;

public slots:
    void startStopButtonGeclickt();
    void menueButtonGeclickt();
    void anfeuernButtonGeclickt();

    void setBatterieProzent(int prozent);
    void setLadestatus(bool aktiv);
    void ladeBatterieProzent(int prozent = 1);
    void entladeBatterieProzent(int prozent = 1);

    void setRotationsspeed(int speed);

    void setColor(const QString &colorcode);
    void toggleLedRing();


    //neue Methoden für Display, mehrere da sonst überladung von Methoden
    void setDisplayWorkoutWerte(int heartRate,
                                const QString &pace,
                                const QString &distance,
                                int lap,
                                int runnerCount);

    void setDisplayStatusWerte(int batteryPercent,
                               const QString &time,
                               int runnerCount,
                               const QString &connectionText);

    void setDisplayTimerWerte(const QString &pace,
                              const QString &elapsedTime,
                              const QString &totalMinutes,
                              const QString &distanceToGoal = QString("--"));

    void setDisplayRunnerWerte(const QStringList &runnerNames);

signals:

    void startStopGeaendert(bool aktiv);
    void menueButtonGeandert();

    void ladestatusGeaendert(bool aktiv);

    void batterieProzentGeandert(int prozent);
// Display gibt nur die Watch-Sites weiter, was da rauf kommt entscheidet hardware
    void displaySeiteGeaendert(sDisplay::WatchSite seite);

    void rotationsspeedGeandert(int speed);

    void colorGeaendert(const QString &color);
    void statusGeandert(bool status);
    void anfeuernAngefordert();

    void datenAktualisiert(const HardwareDaten &daten);

    //gleiche für die Signals, mehrere da sonst die Methoden von Display überladen sind
    void displayWorkoutWerteGeaendert(int heartRate,
                                      QString pace,
                                      QString distance,
                                      int lap,
                                      int runnerCount);

    void displayStatusWerteGeaendert(int batteryPercent,
                                     QString time,
                                     int runnerCount,
                                     QString connectionText);

    void displayTimerWerteGeaendert(QString pace,
                                    QString elapsedTime,
                                    QString totalMinutes,
                                    QString distanceToGoal);

    void displayRunnerWerteGeaendert(QStringList runnerNames);

};
