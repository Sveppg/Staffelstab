#pragma once

#include <QObject>
#include <QTimer>

namespace simulation {

class Clock : public QObject {
    Q_OBJECT

private:
    QTimer timer;
    float speedFactor;

signals:
    void deltaTimeSeconds(float deltaTSec);

public:
    explicit Clock();

    void start();
    void stop();


    void setTickRate(int ticksPerSecond);
    int getTickRate();

    void setSpeedFactor(float speedFactor);
    float getSpeedFactor() const;

public slots:
    void onTick();
};

};
