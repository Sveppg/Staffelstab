#pragma once

#include "logic/auswertung.h"

class App
{
public:
    virtual Auswertung empfangeDaten() = 0;
    virtual Auswertung visualisiereDaten() = 0;
};