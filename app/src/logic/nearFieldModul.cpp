#include "logic/nearFieldModul.h"

float NearFieldModul::getDistance() const
{
    return Distance;
}

void NearFieldModul::setDistance(float distance)
{
    Distance = distance;
}

void NearFieldModul::parseSensorData()
{
    //Hier Daten vom Sensor auslesen und parsen, wenn echer Sensor vorhanden.
}
