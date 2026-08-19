#include "logic/gpsModul.h"

#include <string>
#include <array>

GPSModul::GPSModul(QObject* parent)
    : QObject(parent),
      Position{0.0f, 0.0f}
{
}

Vector2 GPSModul::getPosition() const {
    return Position;
}

void GPSModul::setPosition(Vector2 position) {
    Position = position;
}

void GPSModul::setSensorValues(Vector2 position, float newSpeed, float newHeight, bool newFix, int newSatAmount) {
    Position = position;
    speed = newSpeed;
    height = newHeight;
    fix = newFix;
    SatAmount = newSatAmount;
}

void GPSModul::parseSensorData(std::string data) {
    //Hier sollen NMEA-Daten von Sensor in die entsprechenden Werte geparse werden.
    //Da wir keinen echten Sensor haben, werden die Werte hier einfach gesetzt.
}

bool GPSModul::getFix() const {
    return fix;
}

float GPSModul::getSpeed() const {
    return speed;
}

float GPSModul::getHeight() const {
    return height;
}

int GPSModul::getSatAmount() const {
    return SatAmount;
}
