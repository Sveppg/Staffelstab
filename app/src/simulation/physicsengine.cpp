#include <math.h>

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QObject>
#include <QtGlobal>
#include <QXmlStreamReader>

#include "simulation/physicsengine.h"

namespace simulation {

namespace {
// Mittlerer Erdradius fuer die Kugelnaeherung der Haversine-Formel.
constexpr double EarthRadiusMeters = 6371000.0;

double degreesToRadians(double degrees)
{
    return degrees * M_PI / 180.0;
}

double radiansToDegrees(double radians)
{
    return radians * 180.0 / M_PI;
}

double haversineDistanceMeters(const GpxPoint &from, const GpxPoint &to)
{
    // Haversine berechnet die Distanz zwischen zwei GPS-Koordinaten
    // auf einer Kugeloberflaeche. Fuer einen Lauftrack ist diese
    // Naeherung ausreichend und stabiler als eine flache x/y-Rechnung.
    const double lat1 = degreesToRadians(from.lat);
    const double lat2 = degreesToRadians(to.lat);
    const double deltaLat = degreesToRadians(to.lat - from.lat);
    const double deltaLon = degreesToRadians(to.lon - from.lon);

    const double a = std::sin(deltaLat / 2.0) * std::sin(deltaLat / 2.0)
        + std::cos(lat1) * std::cos(lat2)
        * std::sin(deltaLon / 2.0) * std::sin(deltaLon / 2.0);
    const double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));

    return EarthRadiusMeters * c;
}

float bearingDegrees(const GpxPoint &from, const GpxPoint &to)
{
    // Heading beschreibt die Bewegungsrichtung in Grad:
    // 0 = Norden, 90 = Osten, 180 = Sueden, 270 = Westen.
    const double lat1 = degreesToRadians(from.lat);
    const double lat2 = degreesToRadians(to.lat);
    const double deltaLon = degreesToRadians(to.lon - from.lon);

    const double y = std::sin(deltaLon) * std::cos(lat2);
    const double x = std::cos(lat1) * std::sin(lat2)
        - std::sin(lat1) * std::cos(lat2) * std::cos(deltaLon);

    double bearing = std::fmod(radiansToDegrees(std::atan2(y, x)) + 360.0, 360.0);
    return static_cast<float>(bearing);
}

float signedAngleDeltaDegrees(float from, float to)
{
    // Winkel werden zyklisch betrachtet. Der kuerzeste Unterschied von
    // 350 auf 10 Grad ist also +20 Grad statt -340 Grad.
    float delta = to - from;
    while (delta > 180.0f) {
        delta -= 360.0f;
    }
    while (delta < -180.0f) {
        delta += 360.0f;
    }
    return delta;
}

int cadenceFromSpeed(float speedMps)
{
    // Vereinfachtes Sensormodell: aus der Laufgeschwindigkeit wird
    // eine plausible Schrittfrequenz abgeleitet.
    if (speedMps <= 0.2f) {
        return 0;
    }

    return qBound(120, qRound(150.0f + speedMps * 8.0f), 190);
}

void enrichTrack(QVector<GpxPoint> &track)
{
    // Verarbeitungsschritt "Rohdaten -> abgeleitete Daten":
    // Der GPX-Track liefert Position, Zeit, Hoehe und Herzfrequenz.
    // Fuer die Systemsimulation werden daraus Strecke, Geschwindigkeit,
    // Beschleunigung, Richtung, Drehrate und Trittfrequenz berechnet.
    if (track.isEmpty()) {
        return;
    }

    track[0].cadenceSpm = cadenceFromSpeed(track[0].speedMps);

    for (int i = 1; i < track.size(); ++i) {
        GpxPoint &current = track[i];
        const GpxPoint &previous = track[i - 1];

        // deltaSeconds ist die Zeitdifferenz des aktuellen Segmentes.
        // Die Untergrenze verhindert Division durch 0 bei fehlerhaften
        // oder gleichen Zeitstempeln.
        const float deltaSeconds = qMax(0.001f,
            current.secondsFromStart - previous.secondsFromStart);
        const float segmentDistance = static_cast<float>(
            haversineDistanceMeters(previous, current));

        // Diskrete Bewegungsgleichungen fuer zwei aufeinanderfolgende
        // Trackpunkte: v = s / t und a = delta v / delta t.
        current.distanceFromStartMeters =
            previous.distanceFromStartMeters + segmentDistance;
        current.speedMps = segmentDistance / deltaSeconds;
        current.accelerationMps2 =
            (current.speedMps - previous.speedMps) / deltaSeconds;
        current.headingDegrees = bearingDegrees(previous, current);
        current.yawRateDps =
            signedAngleDeltaDegrees(previous.headingDegrees,
                                    current.headingDegrees) / deltaSeconds;
        current.cadenceSpm = cadenceFromSpeed(current.speedMps);
    }
}
}

PhysicsEngine::PhysicsEngine() : QObject(nullptr), timeElapsedInSeconds(0.0f), currentGpxIndex(0) {
    setSinusValue(0);
};

void PhysicsEngine::onTick (float deltaTSec){
    // Zentrale Taktoperation der Simulation: Die externe Uhr liefert
    // ein Delta t, daraus werden alle zeitabhaengigen Ausgaben bestimmt.
    setTimeElapsed(deltaTSec);
    setSinusValue(calculateNextSinusValue(getTimeElapsed()));
    updateGpxPosition();

    // Ausgabe ueber Signale: GUI und Laufsystem bleiben lose gekoppelt
    // und muessen die PhysicsEngine nicht aktiv pollen.
    emit sinusAmplitudeFloat(getSinusValue());
    emit sinusAmplitudeInt(round(getSinusValue()));
};

float PhysicsEngine::calculateNextSinusValue (float deltaTSec) {
    return std::sin(deltaTSec);
};


// setter Methoden
void PhysicsEngine::setSinusValue(float amplitude){

    sinusValue = amplitude;
    printf("Sinus Amplitude: %f\n", sinusValue);
};

void PhysicsEngine::setTimeElapsed (float detlaTSec) {
    // Zeit laeuft in der Simulation nur vorwaerts und wird gegen
    // negative Tick-Werte abgesichert.
    timeElapsedInSeconds = qMax(0.0f, timeElapsedInSeconds + detlaTSec);
};


// getter Methoden
float PhysicsEngine::getSinusValue(){
    return sinusValue;
}

float PhysicsEngine:: getTimeElapsed () {
    return timeElapsedInSeconds;
};

void PhysicsEngine::startSimulation() {
    // Lazy Loading: Der Track wird erst geladen, wenn die Simulation
    // wirklich gestartet oder abgefragt wird.
    if (gpxTrack.isEmpty()) {
        loadGpxTrack();
    }
    updateGpxPosition();
};

void PhysicsEngine::stopSimulation() {
};

void PhysicsEngine::resetSimulation() {
    // Reset stellt den Anfangszustand des Simulationsautomaten wieder her.
    timeElapsedInSeconds = 0.0f;
    currentGpxIndex = 0;
    if (gpxTrack.isEmpty()) {
        loadGpxTrack();
    }
    updateGpxPosition();
};

bool PhysicsEngine::loadGpxTrack() {
    // Die GPX-Datei ist die Datenquelle der Simulation. Ist sie bereits
    // geladen, bleibt der vorhandene Track erhalten.
    if (!gpxTrack.isEmpty()) {
        return true;
    }

    return parseGpxFile(findDataFile("B2Run_2025.gpx"));
};

QString PhysicsEngine::findDataFile(const QString &fileName) const {
    // Tests und Anwendung starten aus unterschiedlichen Arbeitsordnern.
    // Deshalb werden mehrere typische Datenpfade ausprobiert.
    const QStringList searchDirs = {
        QCoreApplication::applicationDirPath() + "/data",
        QCoreApplication::applicationDirPath() + "/../app/data",
        QCoreApplication::applicationDirPath() + "/../../app/data",
        QCoreApplication::applicationDirPath() + "/../../../app/data",
        QDir::currentPath() + "/app/data",
        QDir::currentPath() + "/data"
    };

    for (const QString &dir : searchDirs) {
        const QString path = QDir(dir).filePath(fileName);
        if (QFile::exists(path)) {
            return path;
        }
    }

    return QString();
}

bool PhysicsEngine::parseGpxFile(const QString &filePath) {
    // Parser-Schritt: XML-Struktur der GPX-Datei wird in eine Liste
    // von Trackpunkten ueberfuehrt. Weitere Berechnungen passieren
    // danach bewusst getrennt in enrichTrack().
    //
    // GPX ist XML. Vereinfacht sieht der relevante Teil so aus:
    //
    // <trk>                 Track = der gesamte aufgezeichnete Lauf
    //   <trkseg>            Tracksegment = zusammenhaengender Abschnitt
    //     <trkpt ...>       Trackpoint = eine einzelne GPS-Messung
    //       <ele>...</ele>  Hoehe
    //       <time>...</time> Zeitpunkt
    //     </trkpt>
    //   </trkseg>
    // </trk>
    //
    // Diese Methode sucht nur die einzelnen <trkpt>-Elemente heraus.
    // Aus jedem <trkpt> entsteht genau ein GpxPoint im QVector parsedTrack.
    if (filePath.isEmpty()) {
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QVector<GpxPoint> parsedTrack;
    QDateTime startTime;
    QXmlStreamReader xml(&file);

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();

        // Solange das aktuelle XML-Element kein <trkpt> ist, wird es
        // ignoriert. Dadurch ueberspringen wir z.B. <gpx>, <trk>,
        // <name> oder <trkseg> und beginnen erst bei einer GPS-Messung.
        if (!xml.isStartElement() || xml.name() != QStringLiteral("trkpt")) {
            continue;
        }

        // Ein <trkpt> hat die GPS-Koordinaten als Attribute:
        // <trkpt lat="52.5" lon="13.4">.
        // Diese Attribute werden direkt in unser Datenobjekt uebernommen.
        const QXmlStreamAttributes attrs = xml.attributes();
        GpxPoint point;
        point.lat = attrs.value(QStringLiteral("lat")).toDouble();
        point.lon = attrs.value(QStringLiteral("lon")).toDouble();
        QDateTime pointTime;

        // Jetzt stehen wir innerhalb dieses einen <trkpt>.
        // Die innere Schleife liest seine Unterelemente, bis das passende
        // schliessende </trkpt> erreicht ist. Danach ist der Messpunkt
        // vollstaendig gelesen.
        while (!xml.atEnd()) {
            xml.readNext();
            if (xml.isStartElement() && xml.name() == QStringLiteral("ele")) {
                // <ele> enthaelt die Hoehe ueber Normalnull in Metern.
                point.elevationMeters = xml.readElementText().toDouble();
            }
            if (xml.isStartElement() && xml.name() == QStringLiteral("time")) {
                // <time> ist der echte Aufnahmezeitpunkt des GPS-Punktes.
                // Daraus wird spaeter secondsFromStart berechnet.
                pointTime = QDateTime::fromString(xml.readElementText(), Qt::ISODate);
                point.timestamp = pointTime;
            }
            if (xml.isStartElement() && xml.name() == QStringLiteral("hr")) {
                // <hr> kommt aus den GPX-Erweiterungen und steht fuer
                // heart rate, also Herzfrequenz in bpm.
                point.heartRateBpm = xml.readElementText().toInt();
            }
            if (xml.isEndElement() && xml.name() == QStringLiteral("trkpt")) {
                // Ende dieses Trackpoints: Ab hier gehoeren folgende
                // XML-Elemente zu einem neuen Messpunkt oder zum Rest
                // der Datei.
                break;
            }
        }

        if (!startTime.isValid()) {
            startTime = pointTime;
        }

        const float secondsFromStart = (startTime.isValid() && pointTime.isValid())
            ? startTime.msecsTo(pointTime) / 1000.0f
            : static_cast<float>(parsedTrack.size());
        // Relative Zeit macht die Simulation unabhaengig vom absoluten
        // Datum des aufgezeichneten Laufes.
        point.secondsFromStart = secondsFromStart;

        // Der fertig ausgelesene <trkpt> wird als GpxPoint in die
        // Trackliste geschrieben. Viele solcher Punkte ergeben spaeter
        // die komplette simulierte Laufstrecke.
        parsedTrack.append(point);
    }

    if (parsedTrack.isEmpty()) {
        return false;
    }

    enrichTrack(parsedTrack);
    gpxTrack = parsedTrack;
    currentGpxIndex = 0;
    const GpxPoint &lastPoint = gpxTrack.last();

    // Diagnoseausgabe fuer den Integrationskontext: Sie fasst zusammen,
    // welche Datenbasis die Simulation geladen hat.
    double minElevation = gpxTrack.first().elevationMeters;
    double maxElevation = gpxTrack.first().elevationMeters;
    int minHeartRate = gpxTrack.first().heartRateBpm;
    int maxHeartRate = gpxTrack.first().heartRateBpm;
    for (const GpxPoint &point : gpxTrack) {
        minElevation = qMin(minElevation, point.elevationMeters);
        maxElevation = qMax(maxElevation, point.elevationMeters);
        if (point.heartRateBpm > 0) {
            minHeartRate = minHeartRate > 0
                ? qMin(minHeartRate, point.heartRateBpm)
                : point.heartRateBpm;
            maxHeartRate = qMax(maxHeartRate, point.heartRateBpm);
        }
    }

    qDebug().noquote()
        << QString("GPX geladen: %1 Punkte, Dauer %2 s, Strecke %3 m, Start %4, Ziel %5, Hoehe %6-%7 m, Herzfrequenz %8-%9 bpm")
               .arg(gpxTrack.size())
               .arg(QString::number(lastPoint.secondsFromStart, 'f', 1))
               .arg(QString::number(lastPoint.distanceFromStartMeters, 'f', 1))
               .arg(gpxTrack.first().timestamp.toString(Qt::ISODate))
               .arg(lastPoint.timestamp.toString(Qt::ISODate))
               .arg(QString::number(minElevation, 'f', 1))
               .arg(QString::number(maxElevation, 'f', 1))
               .arg(minHeartRate)
               .arg(maxHeartRate);
    emit gpxTrackLoaded();
    updateGpxPosition();
    return true;
}

void PhysicsEngine::updateGpxPosition() {
    // Abbildung Simulationszeit -> Trackindex.
    // Der Index wird inkrementell angepasst, damit nicht bei jedem Tick
    // der gesamte Track durchsucht werden muss.
    if (gpxTrack.isEmpty()) {
        return;
    }

    while (currentGpxIndex > 0
           && gpxTrack[currentGpxIndex].secondsFromStart > timeElapsedInSeconds) {
        --currentGpxIndex;
    }

    while (currentGpxIndex + 1 < gpxTrack.size()
           && gpxTrack[currentGpxIndex + 1].secondsFromStart <= timeElapsedInSeconds) {
        ++currentGpxIndex;
    }

    const GpxPoint &point = gpxTrack[currentGpxIndex];
    // Die aktuellen Trackdaten werden als Position und als vollstaendiges
    // Sensorpaket verteilt. Dadurch koennen GUI, Ortung und Sensoren
    // dieselbe simulierte Wahrheit verwenden.
    emit simulatedSensorDataChanged(point.lat,
                                    point.lon,
                                    point.elevationMeters,
                                    point.speedMps,
                                    point.accelerationMps2,
                                    point.heartRateBpm,
                                    point.cadenceSpm,
                                    point.headingDegrees,
                                    point.yawRateDps,
                                    currentGpxIndex,
                                    gpxTrack.size());
    emit gpxPositionChanged(point.lat, point.lon, currentGpxIndex, gpxTrack.size());
}

int PhysicsEngine::getGpxPointCount() const {
    return gpxTrack.size();
}

GpxPoint PhysicsEngine::getGpxPoint(int index) const {
    // Robuste Leseschnittstelle fuer Tests und GUI: Indizes ausserhalb
    // des Tracks werden auf den gueltigen Bereich begrenzt.
    if (gpxTrack.isEmpty()) {
        return {0.0, 0.0, 0.0f};
    }

    return gpxTrack[qBound(0, index, gpxTrack.size() - 1)];
}

void PhysicsEngine::writeSensors() {
    updateGpxPosition();
}

};
