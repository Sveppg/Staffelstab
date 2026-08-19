#include "logic/vibrationsmotor.h"

#include <QDebug>
#include <QtGlobal>

Vibrationsmotor::Vibrationsmotor(QObject *parent)
    : QObject(parent)
{
}

int Vibrationsmotor::getRotationsspeed() const{
    return rotationsspeed;
}

void Vibrationsmotor::setRotationsspeed(int speed)
{
    // Begrenzung auf 0 bis 100
    speed = qBound(0, speed, 100);

    if (rotationsspeed == speed)
        return;

    rotationsspeed = speed;

    qDebug() << "Vibrationsmotor rotationsspeed:" << rotationsspeed;

    emit rotationsspeedGeaendert(rotationsspeed);
}