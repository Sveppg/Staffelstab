#include "logic/gyroskopsensor.h"


Gyroskopsensor::Gyroskopsensor(QObject* parent) : QObject(parent){
    Winkelgeschwindigkeit = {0, 0, 0};
}

void Gyroskopsensor::setWinkelgeschwindigkeit(Vector3 winkel_geschwindigkeit){
    Winkelgeschwindigkeit = winkel_geschwindigkeit;
}

Vector3 Gyroskopsensor::getWinkelgeschwindigkeit(){
    return Winkelgeschwindigkeit;
}