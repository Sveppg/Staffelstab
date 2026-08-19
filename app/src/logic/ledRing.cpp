#include "logic/ledRing.h"

LEDRing::LEDRing(QObject *parent)
    : QObject(parent),
    status(false),
    color("#000000")
{
}

bool LEDRing::getStatus() const
{
    return status;
}

QString LEDRing::getColor() const
{
    return color;
}

void LEDRing::setStatus()
{
    status = !status;
    emit statusGeaendert(status);
}

void LEDRing::setColor(const QString &colorcode)
{
    if (color == colorcode) {
        return;
    }

    color = colorcode;
    emit colorGeaendert(color);
}