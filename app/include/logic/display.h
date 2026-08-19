#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

class sDisplay : public QObject
{
    Q_OBJECT

public:
    enum class WatchSite {
        Workout,
        Timer,
        Status,
        Runner
    };
    Q_ENUM(WatchSite)

    explicit sDisplay(QObject *parent = nullptr);

    WatchSite getAktuelleSeite() const;

public slots:
    void naechsteSeite();
    void setWatchSeite(WatchSite seite);

    void setWorkoutWerte(int heartRate,
                         const QString &pace,
                         const QString &distance,
                         int lap,
                         int runnerCount);

    void setStatusWerte(int batteryPercent,
                        const QString &time,
                        int runnerCount,
                        const QString &connectionText);

    void setTimerWerte(const QString &pace,
                       const QString &elapsedTime,
                       const QString &totalMinutes,
                       const QString &distanceToGoal = QString("--"));

    void setRunnerWerte(const QStringList &runnerNames);

signals:
    void watchSeiteGeaendert(sDisplay::WatchSite seite);

    void workoutWerteGeaendert(int heartRate,
                               QString pace,
                               QString distance,
                               int lap,
                               int runnerCount);

    void statusWerteGeaendert(int batteryPercent,
                              QString time,
                              int runnerCount,
                              QString connectionText);

    void timerWerteGeaendert(QString pace,
                             QString elapsedTime,
                             QString totalMinutes,
                             QString distanceToGoal);

    void runnerWerteGeaendert(QStringList runnerNames);

private:
    WatchSite aktuelleSeite = WatchSite::Workout;
};
