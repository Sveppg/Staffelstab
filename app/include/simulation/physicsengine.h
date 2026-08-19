#pragma once

#include <QObject>
#include <QDateTime>
#include <QString>
#include <QVector>

namespace simulation{

// Datenobjekt fuer einen Trackpunkt im GPX-Lauf.
// Ein GpxPoint ist das C++-Gegenstueck zu einem <trkpt>-Element
// in der GPX-Datei.
// Entspricht im Sinne der Folien einem einfachen DTO:
// Es transportiert Rohdaten aus der Datei und daraus berechnete Sensordaten.
struct GpxPoint {
    // Rohdaten aus dem GPX-Track:
    // <trkpt lat="..." lon="..."> liefert lat/lon,
    // die Unterelemente <ele> und <time> liefern Hoehe und Zeitpunkt.
    double lat = 0.0;
    double lon = 0.0;
    double elevationMeters = 0.0;
    QDateTime timestamp;

    // Simulations- und Bewegungsdaten, die aus aufeinanderfolgenden
    // Trackpunkten abgeleitet werden.
    float secondsFromStart = 0.0f;
    float distanceFromStartMeters = 0.0f;
    float speedMps = 0.0f;
    float accelerationMps2 = 0.0f;

    // Simulierte Koerper- und Bewegungssensoren fuer das Laufsystem.
    int heartRateBpm = 0;
    int cadenceSpm = 0;
    float headingDegrees = 0.0f;
    float yawRateDps = 0.0f;
};

// Die PhysicsEngine bildet die Simulations-Komponente.
// Ihre Zustaendigkeit ist die Umwandlung eines realen GPX-Tracks in
// zeitabhaengige Sensordaten. Nach aussen kommuniziert sie nur ueber
// eine kleine Schnittstelle und Qt-Signale, damit Laufsystem und GUI
// als getrennte Komponenten daran angebunden werden koennen.
class PhysicsEngine : public QObject{
    Q_OBJECT
private:
    // Interner Zustand der Simulation: aktueller Sinus-Testwert,
    // vergangene Simulationszeit und Position im geladenen Track.
    float sinusValue;
    float timeElapsedInSeconds;
    QVector<GpxPoint> gpxTrack;
    int currentGpxIndex;

    // Interne Hilfsoperationen bleiben gekapselt, weil sie nur die
    // Implementierung des GPX-Datenflusses betreffen.
    bool parseGpxFile(const QString &filePath);
    QString findDataFile(const QString &fileName) const;
    void updateGpxPosition();

signals:
    // Signale sind die Ausgangsseite der Komponente:
    // Andere Objekte reagieren darauf, ohne die PhysicsEngine direkt
    // nach jedem Detail abfragen zu muessen.
    void sinusAmplitudeFloat(float amplitude);
    void sinusAmplitudeInt(int amplitude);
    void gpxPositionChanged(double lat, double lon, int index, int count);
    void simulatedSensorDataChanged(double lat,
                                    double lon,
                                    double elevationMeters,
                                    float speedMps,
                                    float accelerationMps2,
                                    int heartRateBpm,
                                    int cadenceSpm,
                                    float headingDegrees,
                                    float yawRateDps,
                                    int index,
                                    int count);
    void gpxTrackLoaded();

public:
    explicit PhysicsEngine();

    // Steuerung des Simulationslaufs.
    void startSimulation();
    void stopSimulation();
    void resetSimulation();

    // Initialisiert die Datenbasis der Simulation aus der GPX-Datei.
    bool loadGpxTrack();

    // Einfache Testfunktion aus der urspruenglichen Simulation:
    // liefert einen periodischen Wert zur Anzeige/Signalpruefung.
    float calculateNextSinusValue(float currentSinusValue);

    void setSinusValue(float sinusValue);
    float getSinusValue();

    void setTimeElapsed(float timeElapsedInSeconds);
    float getTimeElapsed();

    int getGpxPointCount() const;
    GpxPoint getGpxPoint(int index) const;

    // Schreibt den aktuellen Simulationszustand erneut auf die
    // Ausgangssignale, z.B. wenn Sensoren aktiv gepollt werden.
    void writeSensors();

public slots:
    // Takt-Eingang der Simulation. Jede Tick-Dauer erhoeht die
    // Simulationszeit und bestimmt daraus den passenden GPX-Punkt.
    void onTick(float deltaTSec);
};

};
