#pragma once

#include <array>
#include <QObject>

using Vector3 = std::array<float, 3>;

class Gyroskopsensor : public QObject
{
    Q_OBJECT

private:
    Vector3 Winkelgeschwindigkeit;

public:
    // void sensorAuslesen();
    explicit Gyroskopsensor(QObject* parent = nullptr);
    void setWinkelgeschwindigkeit(Vector3 winkel_geschwindigkeit);
    Vector3 getWinkelgeschwindigkeit();
};