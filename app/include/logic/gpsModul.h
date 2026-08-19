#pragma once

#include <string>
#include <array>
#include <QObject>

using Vector3 = std::array<float, 3>;
using Vector2 = std::array<float, 2>;

class GPSModul : public QObject {
    Q_OBJECT
private:

     //Ortung* Ortung;


    Vector2 Position;
    float speed = 0.0f;
    float height = 0.0f;
    bool fix = false;
    int SatAmount = 0;

public:
    explicit GPSModul(QObject* parent = nullptr);

    Vector2 getPosition() const;
    void setPosition(Vector2 position);
    void setSensorValues(Vector2 position, float speed, float height, bool fix, int satAmount);
    void parseSensorData(std::string data);
    bool getFix() const;
    float getSpeed() const;
    float getHeight() const;
    int getSatAmount() const;
};
