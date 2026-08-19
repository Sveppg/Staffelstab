#include "logic/startstopbutton.h"

StartStopButton::StartStopButton(QObject *parent)
    : QObject(parent)
{
}

void StartStopButton::druecken()
{
    emit gedrueckt();
}

void StartStopButton::knopfAuslesen(bool istGedrueckt)
{
    if (istGedrueckt && !vorherGedrueckt) {
        emit gedrueckt();
    }
    //speichert kein state mehr, nur tastendruck und weiter
    vorherGedrueckt = istGedrueckt;
}