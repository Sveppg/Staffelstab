#pragma once

class Beschleunigungssensor
{
private:
    double Beschleunigung = 0.0;

public:
    void sensorAuslesen();
    double getBeschleunigung() const;
    void setBeschleunigung(double beschleunigung);
};
