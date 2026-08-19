#include <QObject>
#include "simulation/clock.h"

namespace simulation{
// SimulationClock::SimulationClock(int tickRate) : QObject(nullptr) {
Clock::Clock () : QObject(nullptr), speedFactor(1.0f) {
    setTickRate(20);
    connect(&timer, &QTimer::timeout, this, &Clock::onTick);
};

void Clock::start() {
    printf("Der Timer wird gestart\n");
    timer.start();
};

void Clock::stop() {
    timer.stop();
};

// TODO Es wird noch nicht die tatsächlich vergangene Zeit zurückgegeben sondern 
// lediglich die aktuelle TickRate in Sekunden. Das kann zu Fehlern (nicht akkuraten Werten) führen falls 
// die TickRate während dem Aufruf verändert wurde. 
void Clock::onTick() {
    emit Clock::deltaTimeSeconds((timer.interval() / 1000.0f) * speedFactor);
};

void Clock::setTickRate(int ticksPerSecond) {
    timer.setInterval(1000 / ticksPerSecond);
};

int Clock::getTickRate() {
    return 1000 / timer.interval();
};

void Clock::setSpeedFactor(float newSpeedFactor) {
    speedFactor = newSpeedFactor;
};

float Clock::getSpeedFactor() const {
    return speedFactor;
};

};
