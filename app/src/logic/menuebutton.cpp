#include "logic/menuebutton.h"

#include <QDebug>

MenueButton::MenueButton(QObject *parent)
    : QObject(parent)
{
}

bool MenueButton::getClicked() const
{
    return clicked;
}

int MenueButton::getKlickAnzahl() const
{
    return klickAnzahl;
}

void MenueButton::setClicked()
{
    clicked = true;
    klickAnzahl++;

    qDebug() << "MenueButton wurde geklickt. Klickanzahl:" << klickAnzahl;

    emit clickedZustand(clicked);
    emit menueButtonGeklickt();
    emit klickAnzahlGeaendert(klickAnzahl);

    // reset da kein memory
    clicked = false;
}