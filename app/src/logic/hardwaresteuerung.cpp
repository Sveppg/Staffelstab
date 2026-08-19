#include "logic/hardwaresteuerung.h"

#include <iostream>
#include <QDebug>
#include <QTimer>


HardwareSteuerung::HardwareSteuerung(QObject *parent)
    : QObject(parent)
{

    connect(&startButton, &StartStopButton::gedrueckt,
            this, [this]() {
                daten.startAktiv = !daten.startAktiv;

                qDebug() << "HardwareSteuerung: StartStop geändert:"
                          << daten.startAktiv;

                if (daten.startAktiv) {
                    setColor("#39ff6a"); // grün
                } else {
                    setColor("#ff2f92"); // pink/rot
                }

                emit startStopGeaendert(daten.startAktiv);
                emit datenAktualisiert(daten);
            });

    connect(&menueButton, &MenueButton::menueButtonGeklickt,
            this, [this]() {
                qDebug() << "HardwareSteuerung: MenueButton geklickt";

                emit menueButtonGeandert();
                emit datenAktualisiert(daten);
            });

    connect(&vibrationsmotor, &Vibrationsmotor::rotationsspeedGeaendert,
            this, [this](int speed){
                daten.rotationsspeed = speed;
        qDebug() << "Rotationsspeed geändert" << speed;
                emit rotationsspeedGeandert(speed);
                emit datenAktualisiert(daten);
            });



    connect(&display, &sDisplay::workoutWerteGeaendert,
            this, [this](int heartRate,
                   const QString &pace,
                   const QString &distance,
                   int lap,
                   int runnerCount) {

                qDebug() << "HardwareSteuerung: Display Workout geändert";

                emit displayWorkoutWerteGeaendert(heartRate,
                                                  pace,
                                                  distance,
                                                  lap,
                                                  runnerCount);
            });
    connect(&display, &sDisplay::statusWerteGeaendert,
            this, [this](int batteryPercent,
                   const QString &time,
                   int runnerCount,
                   const QString &connectionText) {
        qDebug() << "HardwareSteuerung: Display Status geändert";

                emit displayStatusWerteGeaendert(batteryPercent,
                                                 time,
                                                 runnerCount,
                                                 connectionText);
            });

    connect(&display, &sDisplay::timerWerteGeaendert,
            this, [this](const QString &pace,
                   const QString &elapsedTime,
                   const QString &totalMinutes,
                   const QString &distanceToGoal) {
        qDebug() << "HardwareSteuerung: Display Timer geändert";

                emit displayTimerWerteGeaendert(pace,
                                                elapsedTime,
                                                totalMinutes,
                                                distanceToGoal);
            });

    connect(&display, &sDisplay::runnerWerteGeaendert,
            this, [this](const QStringList &runnerNames) {
        qDebug() << "HardwareSteuerung: Läufer geändert";

                emit displayRunnerWerteGeaendert(runnerNames);
            });

    connect(&ledRing, &LEDRing::colorGeaendert,
            this,[this](const QString &color){
                daten.ledRingColor = color;
        qDebug() << "HardwareSteuerung: Farbe für LEDRing wurde gesetzt";
                emit colorGeaendert(color);
                emit datenAktualisiert(daten);
            });

    connect(&ledRing, &LEDRing::statusGeaendert,
            this,[this](bool status){
        qDebug() << "HardwareSteuerung: Status von LEDRing changed";
                emit statusGeandert(status);
            });

    // Alle fürs Laden und den Ladehub
    connect(&ladehub, &Ladehub::batterieProzentGeandert,
            this, [this](int prozent) {
                daten.batterieProzent = prozent;

        qDebug() << "HardwareSteuerung: Batterie geändert:" << prozent;

                emit batterieProzentGeandert(prozent);
                emit datenAktualisiert(daten);
            });

    connect(&ladehub, &Ladehub::ladeStatusGeandert,
            this, [this](bool aktiv) {
                daten.ladeStatus = aktiv;

        qDebug() << "HardwareSteuerung: Ladestatus geändert:" << aktiv;

                emit ladestatusGeaendert(aktiv);
                emit datenAktualisiert(daten);
            });
    connect(&display, &sDisplay::watchSeiteGeaendert,
            this, [this](sDisplay::WatchSite seite) {
        qDebug() << "HardwareSteuerung: Display-Seite geändert";

                emit displaySeiteGeaendert(seite);
                emit datenAktualisiert(daten);
            });
}





const HardwareDaten& HardwareSteuerung::getHardwareDaten() const{
    return daten;
}

void HardwareSteuerung::startStopButtonGeclickt()
{
    startButton.druecken();
}

void HardwareSteuerung::menueButtonGeclickt(){
    menueButton.setClicked();
    display.naechsteSeite();
}

void HardwareSteuerung::anfeuernButtonGeclickt(){
    setRotationsspeed(100);
    emit anfeuernAngefordert();
}

void HardwareSteuerung::setRotationsspeed(int speed){
    vibrationsmotor.setRotationsspeed(speed);
    QTimer::singleShot(140, this, [this](){
        vibrationsmotor.setRotationsspeed(0);
    });
}



void HardwareSteuerung::setDisplayWorkoutWerte(int heartRate,
                                               const QString &pace,
                                               const QString &distance,
                                               int lap,
                                               int runnerCount)
{
    display.setWorkoutWerte(heartRate,
                             pace,
                             distance,
                             lap,
                             runnerCount);
}

void HardwareSteuerung::setDisplayStatusWerte(int batteryPercent,
                                              const QString &time,
                                              int runnerCount,
                                              const QString &connectionText)
{
    display.setStatusWerte(batteryPercent,
                            time,
                            runnerCount,
                            connectionText);
}

void HardwareSteuerung::setDisplayTimerWerte(const QString &pace,
                                             const QString &elapsedTime,
                                             const QString &totalMinutes,
                                             const QString &distanceToGoal)
{
    display.setTimerWerte(pace,
                           elapsedTime,
                           totalMinutes,
                           distanceToGoal);
}

void HardwareSteuerung::setDisplayRunnerWerte(const QStringList &runnerNames)
{
    display.setRunnerWerte(runnerNames);
}


void HardwareSteuerung::setColor(const QString &color)
{
    ledRing.setColor(color);

    if (!ledRing.getStatus()) {
        ledRing.setStatus();
    }

    QTimer::singleShot(180, this, [this]() {
        if (ledRing.getStatus()) {
            ledRing.setStatus();
        }
    });
}

void HardwareSteuerung::toggleLedRing(){
    ledRing.setStatus();
}

void HardwareSteuerung::setBatterieProzent(int prozent){
    ladehub.setBatterieProzent(prozent);
}

void HardwareSteuerung::setLadestatus(bool aktiv){
    ladehub.setLadestatus(aktiv);
}

void HardwareSteuerung::ladeBatterieProzent(int prozent)
{
    ladehub.ladeProzent(prozent);
}

void HardwareSteuerung::entladeBatterieProzent(int prozent)
{
    ladehub.entladeProzent(prozent);
}
