#include "logic/ladehub.h"

#include <QtGlobal>

Ladehub::Ladehub(QObject *parent)
    : QObject(parent),
    batterieProzent(0),
    ladestatus(false)
{
}

int Ladehub::getBatterieProzent() const{
    return batterieProzent;
}

bool Ladehub::getLadeStatus() const{
    return ladestatus;
}

void Ladehub::setBatterieProzent(int prozent){
    const int begrenzterWert = qBound(0, prozent, 100);

    if (batterieProzent == begrenzterWert) {
        return;
    }

    batterieProzent = begrenzterWert;

    emit batterieProzentGeandert(batterieProzent);
}

void Ladehub::setLadestatus(bool aktiv){
    if (ladestatus == aktiv) {
        return;
    }

    ladestatus = aktiv;
    emit ladeStatusGeandert(ladestatus);
}

void Ladehub::ladeProzent(int prozent)
{
    if (prozent <= 0) {
        return;
    }

    setBatterieProzent(batterieProzent + prozent);
}

void Ladehub::entladeProzent(int prozent)
{
    if (prozent <= 0) {
        return;
    }

    setBatterieProzent(batterieProzent - prozent);
}
