#include "logic/ortung.h"
#include "logic/gpsModul.h"
#include "logic/nearFieldModul.h"

#include <string>
#include <array>
#include <QtMath>




/*
 * Vector3 = array float 3
 * Vector2 array float 2
 *
 * Ortung berechnet Pace, Positionsdaten, Distanz
 */
Ortung::Ortung(QObject* parent)
    : QObject(parent),
    gpsModul(new GPSModul(this)),
    nearFieldModul(),
    Position{0.0f, 0.0f, 0.0f}  //Anfangsposition wird auf 0 gesetzt
{
}


/*
 * Berechnet Position, schaut ob Module aktiv sind
 */
void Ortung::calculateGPSUWBPosition() {
    if (gpsModul->getFix()) {
        Vector2 gpsPosition = gpsModul->getPosition();
        float height = gpsModul->getHeight();
        Vector3 calcPosition = {gpsPosition[0], gpsPosition[1], height};
        setPosition(calcPosition);
    }
        else {
            setPosition({0.0f, 0.0f, 0.0f});
        }
}


/*
 * parsed GPSData von double zu float und gibt an GPS-Modul weiter
*/
void Ortung::setGpsSensorData(double lat, double lon, double height, double speed, bool fix, int satAmount) {
    gpsModul->setSensorValues({static_cast<float>(lat), static_cast<float>(lon)},
                              static_cast<float>(speed),
                              static_cast<float>(height),
                              fix,
                              satAmount);
    setPosition({static_cast<float>(lat),
                 static_cast<float>(lon),
                 static_cast<float>(height)});
}


/*
 * Erzeugt Messpunkte für die Berechnung der Pace, eliminiert negative
 */
QString Ortung::berechnePace(float secondsFromStart,
                             float distanceFromStartMeters,
                             float speedMps,
                             float windowSeconds) {
    const PaceMesspunkt currentPoint{
        qMax(0.0f, secondsFromStart),
        qMax(0.0f, distanceFromStartMeters),
        qMax(0.0f, speedMps)
    };
//Pace wird zurückgesetzt falls Zeit zurückgesetzt wird
    if (!paceHistory.isEmpty()
        && currentPoint.secondsFromStart < paceHistory.last().secondsFromStart) {
        paceHistory.clear();
    }

//Speichert Messpunkte,Falls keiner vorhanden, speicher momentane Position
    if (paceHistory.isEmpty()) {
        paceHistory.append(currentPoint);
    } else if (qAbs(currentPoint.secondsFromStart
                   - paceHistory.last().secondsFromStart) <= 0.001f) {
        paceHistory.last() = currentPoint;
    } else {
        paceHistory.append(currentPoint);
    }
// enhält nur Messpunkte die jünger sind als windowSeconds
    while (paceHistory.size() > 1
           && currentPoint.secondsFromStart - paceHistory.first().secondsFromStart
                  > windowSeconds) {
        paceHistory.removeFirst();
    }

    float geglaetteteSpeedMps = 0.0f;
//wenn min zwei messpunkte da wird über zeifenster gerechnet, erst nach 5sec nach start
    if (paceHistory.size() > 1) {
        const PaceMesspunkt &startPoint = paceHistory.first();
        const float deltaSeconds =
            currentPoint.secondsFromStart - startPoint.secondsFromStart;
        const float deltaDistance =
            currentPoint.distanceFromStartMeters - startPoint.distanceFromStartMeters;

        if (deltaSeconds >= 5.0f && deltaDistance > 0.0f) {
            geglaetteteSpeedMps = deltaDistance / deltaSeconds;
        }
    }

    if (geglaetteteSpeedMps <= 0.05f
        && currentPoint.secondsFromStart > 0.0f
        && currentPoint.distanceFromStartMeters > 0.0f) {
        geglaetteteSpeedMps =
            currentPoint.distanceFromStartMeters / currentPoint.secondsFromStart;
    }

    if (geglaetteteSpeedMps <= 0.05f) {
        geglaetteteSpeedMps = currentPoint.speedMps;
    }

    return formatierePace(geglaetteteSpeedMps);
}


//clear history
void Ortung::resetPaceBerechnung() {
    paceHistory.clear();
}

void Ortung::setNearFieldDistance(float distance) {
    nearFieldModul.setDistance(qMax(0.0f, distance));
}

float Ortung::getNearFieldDistance() const {
    return nearFieldModul.getDistance();
}


// konvertiere m/s zu pace also min/km
QString Ortung::formatierePace(float speedMps) const {
    if (speedMps <= 0.05f) {
        return "--:--";
    }

    const int secondsPerKm = qRound(1000.0f / speedMps);
    return QString("%1:%2")
        .arg(secondsPerKm / 60)
        .arg(secondsPerKm % 60, 2, 10, QChar('0'));
}


//getter + setter
void Ortung::setPosition(Vector3 position) {
    Position = position;
}

Vector3 Ortung::getPosition() const {
    return Position;
}
