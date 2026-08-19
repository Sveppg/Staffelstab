#pragma once

#include "logic/antPlusModul.h"
#include "logic/bluetoothModul.h"
#include "logic/wlanModul.h"
#include "logic/gyroskopsensor.h"
#include "logic/beschleunigungssensor.h"
#include "logic/trittfrequenzsensor.h"
#include "logic/app.h"
// #include "logic/HardwareSteuerung.h"
#include "logic/ortung.h"

#include <string>
#include <array>

using Vector3 = std::array<float, 3>;
using Vector2 = std::array<float, 2>;

class Kommunikation
{
private:
    AntPlusModul* antPlusModul;
    BluetoothModul* bluetoothModul;
    WLANModul* wlanModul;
    Gyroskopsensor* gyroskopsensor;
    Beschleunigungssensor* beschleunigungssensor;
    Trittfrequenzsensor* trittfrequenzsensor;
    Ortung* ortung;
    App* app;

public:
    void sendePosition(const Ortung& position);
    void sendeLäuferDaten(std::string data);
    void sendeLogs(std::string data);
    void verbindungHerstellen();
};
