#pragma once

#include <QObject>
#include "simulation/clock.h"
#include "simulation/physicsengine.h"
#include "logic/startstopbutton.h"
#include "logic/ortung.h"
#include "logic/trittfrequenzsensor.h"
#include "logic/beschleunigungssensor.h"
#include "logic/herzfrequenzsensor.h"
#include "logic/gyroskopsensor.h"
#include "logic/laufkoordination.h"

class Laufsystem : public QObject {
    Q_OBJECT

    simulation::Clock clock_;
    simulation::PhysicsEngine physicsEngine_;

    StartStopButton startStopButton_;
    Ortung ortung_;
    Trittfrequenzsensor trittfrequenzsensor_;
    Beschleunigungssensor beschleunigungssensor_;
    Herzfrequenzsensor herzfrequenzsensor_;
    Gyroskopsensor gyroskopsensor_;
    Laufkoordination laufkoordination_;
    int lastLoggedSensorIndex_;

public:
    explicit Laufsystem();

    simulation::Clock& getClock();
    simulation::PhysicsEngine& getPhysicsEngine();

    StartStopButton& getStartStopButton();
    Ortung& getOrtung();
    Trittfrequenzsensor& getTrittfrequenzsensor();
    Beschleunigungssensor& getBeschleunigungssensor();
    Herzfrequenzsensor& getHerzfrequenzsensor();
    Gyroskopsensor& getGyroskopsensor();
    Laufkoordination& getLaufkoordination();

public slots:
    void applySimulatedSensorData(double lat,
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

};
