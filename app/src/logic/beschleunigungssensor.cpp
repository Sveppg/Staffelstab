#include "logic/beschleunigungssensor.h"

void Beschleunigungssensor::sensorAuslesen()
{
}

double Beschleunigungssensor::getBeschleunigung() const
{
    return Beschleunigung;
}

void Beschleunigungssensor::setBeschleunigung(double beschleunigung)
{
    Beschleunigung = beschleunigung;
}
