#include <QObject>
#include <QDebug>
#include <QtMath>

#include "laufsystem.h"

namespace {
constexpr double EarthRadiusMeters = 6371000.0;

float distanceBetweenCoordinatesMeters(double fromLat,
                                       double fromLon,
                                       double toLat,
                                       double toLon)
{
    const double lat1 = qDegreesToRadians(fromLat);
    const double lat2 = qDegreesToRadians(toLat);
    const double deltaLat = qDegreesToRadians(toLat - fromLat);
    const double deltaLon = qDegreesToRadians(toLon - fromLon);

    const double sinHalfLat = qSin(deltaLat / 2.0);
    const double sinHalfLon = qSin(deltaLon / 2.0);
    const double a = sinHalfLat * sinHalfLat
        + qCos(lat1) * qCos(lat2) * sinHalfLon * sinHalfLon;
    const double c = 2.0 * qAtan2(qSqrt(a), qSqrt(1.0 - a));

    return static_cast<float>(EarthRadiusMeters * c);
}
}

Laufsystem::Laufsystem() : QObject(nullptr),
    clock_ (),
    physicsEngine_ (),
    startStopButton_(),
    ortung_(),
    gyroskopsensor_(),
    laufkoordination_(&trittfrequenzsensor_,
                      &beschleunigungssensor_,
                      &herzfrequenzsensor_,
                      &gyroskopsensor_),
    lastLoggedSensorIndex_(-1) {
    connect(&physicsEngine_, &simulation::PhysicsEngine::simulatedSensorDataChanged,
            this, &Laufsystem::applySimulatedSensorData);
};

simulation::Clock& Laufsystem::getClock() {
    return clock_;
}

simulation::PhysicsEngine& Laufsystem::getPhysicsEngine() {
    return physicsEngine_;
}

StartStopButton& Laufsystem::getStartStopButton() {
    return startStopButton_;
}

Ortung& Laufsystem::getOrtung() {
    return ortung_;
}

Trittfrequenzsensor& Laufsystem::getTrittfrequenzsensor() {
    return trittfrequenzsensor_;
}

Beschleunigungssensor& Laufsystem::getBeschleunigungssensor() {
    return beschleunigungssensor_;
}

Herzfrequenzsensor& Laufsystem::getHerzfrequenzsensor() {
    return herzfrequenzsensor_;
}

Gyroskopsensor& Laufsystem::getGyroskopsensor() {
    return gyroskopsensor_;
}

Laufkoordination& Laufsystem::getLaufkoordination() {
    return laufkoordination_;
}

void Laufsystem::applySimulatedSensorData(double lat,
                                          double lon,
                                          double elevationMeters,
                                          float speedMps,
                                          float accelerationMps2,
                                          int heartRateBpm,
                                          int cadenceSpm,
                                          float headingDegrees,
                                          float yawRateDps,
                                          int index,
                                          int count) {
    Q_UNUSED(headingDegrees);

    ortung_.setGpsSensorData(lat, lon, elevationMeters, speedMps);
    if (count > 0) {
        const simulation::GpxPoint goalPoint =
            physicsEngine_.getGpxPoint(count - 1);
        const float distanceToGoal = distanceBetweenCoordinatesMeters(
            lat,
            lon,
            goalPoint.lat,
            goalPoint.lon);
        ortung_.setNearFieldDistance(distanceToGoal);
    }

    trittfrequenzsensor_.setTrittfrequenz(cadenceSpm);
    beschleunigungssensor_.setBeschleunigung(accelerationMps2);
    herzfrequenzsensor_.setHerzfrequenz(heartRateBpm);
    gyroskopsensor_.setWinkelgeschwindigkeit({0.0f, 0.0f, yawRateDps});

    if (index != lastLoggedSensorIndex_) {
        laufkoordination_.writeLogDB(ortung_);
        lastLoggedSensorIndex_ = index;
    }

    if (index == 0 || index % 250 == 0) {
        qDebug().noquote()
            << QString("Sensoren befuettert: idx=%1 lat=%2 lon=%3 hoehe=%4m speed=%5m/s acc=%6m/s2 hr=%7bpm cadence=%8spm gyroZ=%9deg/s")
                   .arg(index)
                   .arg(lat, 0, 'f', 6)
                   .arg(lon, 0, 'f', 6)
                   .arg(elevationMeters, 0, 'f', 1)
                   .arg(speedMps, 0, 'f', 2)
                   .arg(accelerationMps2, 0, 'f', 2)
                   .arg(heartRateBpm)
                   .arg(cadenceSpm)
                   .arg(yawRateDps, 0, 'f', 2);
    }
}
