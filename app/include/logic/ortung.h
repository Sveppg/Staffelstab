#pragma once
#include "logic/gpsModul.h"
#include "logic/nearFieldModul.h"

#include <string>
#include <array>
#include <QObject>
#include <QString>
#include <QVector>

using Vector3 = std::array<float, 3>;
using Vector2 = std::array<float, 2>;


class Ortung : QObject {
    Q_OBJECT
private:
    struct PaceMesspunkt {
        float secondsFromStart = 0.0f;
        float distanceFromStartMeters = 0.0f;
        float speedMps = 0.0f;
    };

    GPSModul* gpsModul;
    NearFieldModul nearFieldModul;
    QVector<PaceMesspunkt> paceHistory;

    QString formatierePace(float speedMps) const;


public:
    Vector3 Position;
    explicit Ortung(QObject* parent = nullptr);

    
    void calculateGPSUWBPosition();
    void setGpsSensorData(double lat, double lon, double height, double speed, bool fix = true, int satAmount = 8);
    QString berechnePace(float secondsFromStart,
                         float distanceFromStartMeters,
                         float speedMps,
                         float windowSeconds = 30.0f);
    void resetPaceBerechnung();
    void setNearFieldDistance(float distance);
    float getNearFieldDistance() const;
    void setPosition(Vector3 position);
    Vector3 getPosition() const;
};
