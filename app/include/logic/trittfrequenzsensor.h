#pragma once

class Trittfrequenzsensor
{
private:
    int Trittfrequenz = 0;

public:
    void sensorAuslesen();
    int getTrittfrequenz() const;
    void setTrittfrequenz(int trittfrequenz);
};
