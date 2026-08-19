#include "logic/herzfrequenzsensor.h"

// Herzfrequenzsensor::Herzfrequenzsensor()
//     : Herzfrequenz(0)
// {
// }

// int Herzfrequenzsensor::getHerzfrequenz() const
// {
//     return Herzfrequenz;
// }

// void Herzfrequenzsensor::setHerzfrequenz(int herzfrequenz)
// {
//     Herzfrequenz = herzfrequenz;
//     emit herzfrequenzGeandert();
// }
Herzfrequenzsensor::Herzfrequenzsensor(QObject *parent) : QObject(parent), Herzfrequenz(0)
{
}

int Herzfrequenzsensor::getHerzfrequenz() const
{
    return Herzfrequenz;
}

void Herzfrequenzsensor::setHerzfrequenz(int herzfrequenz)
{
    if (Herzfrequenz != herzfrequenz)
    {
        Herzfrequenz = herzfrequenz;
        emit herzfrequenzGeandert();
    }
}