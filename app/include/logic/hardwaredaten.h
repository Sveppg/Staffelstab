#ifndef HARDWAREDATEN_H
#define HARDWAREDATEN_H
#pragma once

#include <QString>

struct HardwareDaten
{
    bool startAktiv = false;
    int menueKlicks = 0;
    bool ladeStatus = false;
    int rotationsspeed = 0;
    bool ledRingStatus = false;
    QString ledRingColor = "#000000";
    int batterieProzent = 0;
};
#endif // HARDWAREDATEN_H
