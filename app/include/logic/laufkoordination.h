#pragma once

#include "logic/beschleunigungssensor.h"
#include "logic/gyroskopsensor.h"
#include "logic/herzfrequenzsensor.h"
#include "logic/ortung.h"
#include "logic/trittfrequenzsensor.h"

#include <QString>
#include <QStringList>

class Laufkoordination {
private:
    Trittfrequenzsensor* trittfrequenzsensor;
    Beschleunigungssensor* beschleunigungssensor;
    Herzfrequenzsensor* herzfrequenzsensor;
    Gyroskopsensor* gyroskopsensor;
    int LaeuferID;
    QString logDateipfad;
    QStringList laeuferListe;
    int aktuellerLaeuferIndex;

public:
    explicit Laufkoordination(
        Trittfrequenzsensor* trittfrequenzsensor = nullptr,
        Beschleunigungssensor* beschleunigungssensor = nullptr,
        Herzfrequenzsensor* herzfrequenzsensor = nullptr,
        Gyroskopsensor* gyroskopsensor = nullptr);

    void writeLogDB(const Ortung& ortung);
    int getLaeuferID() const;
    void setLaeuferID();
    void addLaeufer(const QString& name);
    void removeLaeufer(int index);
    void naechsterLaeufer();
    QStringList getLaeuferListe() const;
    QString getAktuellerLaeuferName() const;
    int getAktuellerLaeuferIndex() const;
};
