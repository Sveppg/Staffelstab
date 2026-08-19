#pragma once

class NearFieldModul {
private:
    float Distance = 0.0f;

public:

    float getDistance() const;
    void setDistance(float distance);
    void parseSensorData();
};
