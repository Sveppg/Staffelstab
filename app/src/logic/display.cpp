#include "logic/display.h"

sDisplay::sDisplay(QObject *parent)
    : QObject(parent)
{
}

sDisplay::WatchSite sDisplay::getAktuelleSeite() const
{
    return aktuelleSeite;
}

void sDisplay::setWatchSeite(WatchSite seite)
{
    if (aktuelleSeite == seite) {
        return;
    }

    aktuelleSeite = seite;
    emit watchSeiteGeaendert(aktuelleSeite);
}

void sDisplay::naechsteSeite()
{
    switch (aktuelleSeite) {
    case WatchSite::Workout:
        setWatchSeite(WatchSite::Timer);
        break;

    case WatchSite::Timer:
        setWatchSeite(WatchSite::Status);
        break;

    case WatchSite::Status:
        setWatchSeite(WatchSite::Runner);
        break;

    case WatchSite::Runner:
        setWatchSeite(WatchSite::Workout);
        break;
    }
}

void sDisplay::setWorkoutWerte(int heartRate,
                               const QString &pace,
                               const QString &distance,
                               int lap,
                               int runnerCount)
{
    emit workoutWerteGeaendert(heartRate,
                               pace,
                               distance,
                               lap,
                               runnerCount);
}

void sDisplay::setStatusWerte(int batteryPercent,
                              const QString &time,
                              int runnerCount,
                              const QString &connectionText)
{
    emit statusWerteGeaendert(batteryPercent,
                              time,
                              runnerCount,
                              connectionText);
}

void sDisplay::setTimerWerte(const QString &pace,
                             const QString &elapsedTime,
                             const QString &totalMinutes,
                             const QString &distanceToGoal)
{
    emit timerWerteGeaendert(pace,
                             elapsedTime,
                             totalMinutes,
                             distanceToGoal);
}

void sDisplay::setRunnerWerte(const QStringList &runnerNames)
{
    emit runnerWerteGeaendert(runnerNames.mid(0, 5));
}
