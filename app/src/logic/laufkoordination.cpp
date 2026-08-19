#include "logic/laufkoordination.h"

#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QTextStream>

namespace {
constexpr const char* LogDateiname = "laufkoordination_log.csv";

QString csvWert(const QString& wert)
{
    QString escaped = wert;
    escaped.replace('"', "\"\"");
    return QString("\"%1\"").arg(escaped);
}
}

Laufkoordination::Laufkoordination(
    Trittfrequenzsensor* trittfrequenzsensor,
    Beschleunigungssensor* beschleunigungssensor,
    Herzfrequenzsensor* herzfrequenzsensor,
    Gyroskopsensor* gyroskopsensor)
    : trittfrequenzsensor(trittfrequenzsensor),
      beschleunigungssensor(beschleunigungssensor),
      herzfrequenzsensor(herzfrequenzsensor),
      gyroskopsensor(gyroskopsensor),
      LaeuferID(0),
      logDateipfad(LogDateiname),
      aktuellerLaeuferIndex(-1)
{
    QFile logDatei(logDateipfad);

    if (!logDatei.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        qDebug() << "Laufkoordination: CSV-Datei konnte nicht erstellt werden:"
                 << logDateipfad;
        return;
    }

    QTextStream stream(&logDatei);
    stream << "Zeitstempel,"
           << "LaeuferID,"
           << "LaeuferName,"
           << "PositionX,"
           << "PositionY,"
           << "PositionZ,"
           << "Trittfrequenz,"
           << "Beschleunigung,"
           << "Herzfrequenz,"
           << "GyroX,"
           << "GyroY,"
           << "GyroZ\n";
}

void Laufkoordination::writeLogDB(const Ortung& ortung)
{
    const Vector3 position = ortung.getPosition();
    const QString zeitstempel =
        QDateTime::currentDateTime().toString(Qt::ISODate);

    const QString trittfrequenzWert = trittfrequenzsensor != nullptr
        ? QString::number(trittfrequenzsensor->getTrittfrequenz())
        : "";

    const QString beschleunigungWert = beschleunigungssensor != nullptr
        ? QString::number(beschleunigungssensor->getBeschleunigung())
        : "";

    const QString herzfrequenzWert = herzfrequenzsensor != nullptr
        ? QString::number(herzfrequenzsensor->getHerzfrequenz())
        : "";

    QString gyroX;
    QString gyroY;
    QString gyroZ;

    if (gyroskopsensor != nullptr) {
        const Vector3 winkelgeschwindigkeit =
            gyroskopsensor->getWinkelgeschwindigkeit();

        gyroX = QString::number(winkelgeschwindigkeit[0]);
        gyroY = QString::number(winkelgeschwindigkeit[1]);
        gyroZ = QString::number(winkelgeschwindigkeit[2]);
    }

    QFile logDatei(logDateipfad);

    if (!logDatei.open(QIODevice::Append | QIODevice::Text)) {
        qDebug() << "Laufkoordination: CSV-Datei konnte nicht beschrieben werden:"
                 << logDateipfad;
        return;
    }

    QTextStream stream(&logDatei);
    stream << csvWert(zeitstempel) << ','
           << LaeuferID << ','
           << csvWert(getAktuellerLaeuferName()) << ','
           << position[0] << ','
           << position[1] << ','
           << position[2] << ','
           << trittfrequenzWert << ','
           << beschleunigungWert << ','
           << herzfrequenzWert << ','
           << gyroX << ','
           << gyroY << ','
           << gyroZ << '\n';

    qDebug() << "Laufkoordination: Log fuer Laeufer"
             << LaeuferID
             << "wurde geschrieben nach"
             << logDateipfad;
}

int Laufkoordination::getLaeuferID() const
{
    return LaeuferID;
}

void Laufkoordination::setLaeuferID()
{
    static int naechsteLaeuferID = 1;
    LaeuferID = naechsteLaeuferID++;
}

void Laufkoordination::addLaeufer(const QString& name)
{
    const QString trimmedName = name.trimmed();

    if (trimmedName.isEmpty()) {
        return;
    }

    laeuferListe.append(trimmedName);

    if (aktuellerLaeuferIndex < 0) {
        aktuellerLaeuferIndex = 0;
        LaeuferID = 1;
    }
}

void Laufkoordination::removeLaeufer(int index)
{
    if (index < 0 || index >= laeuferListe.size()) {
        return;
    }

    laeuferListe.removeAt(index);

    if (laeuferListe.isEmpty()) {
        aktuellerLaeuferIndex = -1;
        LaeuferID = 0;
        return;
    }

    if (aktuellerLaeuferIndex >= laeuferListe.size()) {
        aktuellerLaeuferIndex = 0;
    }

    LaeuferID = aktuellerLaeuferIndex + 1;
}

void Laufkoordination::naechsterLaeufer()
{
    if (laeuferListe.isEmpty()) {
        aktuellerLaeuferIndex = -1;
        LaeuferID = 0;
        return;
    }

    aktuellerLaeuferIndex =
        (aktuellerLaeuferIndex + 1) % laeuferListe.size();
    LaeuferID = aktuellerLaeuferIndex + 1;
}

QStringList Laufkoordination::getLaeuferListe() const
{
    return laeuferListe;
}

QString Laufkoordination::getAktuellerLaeuferName() const
{
    if (aktuellerLaeuferIndex < 0 ||
        aktuellerLaeuferIndex >= laeuferListe.size()) {
        return QString();
    }

    return laeuferListe.at(aktuellerLaeuferIndex);
}

int Laufkoordination::getAktuellerLaeuferIndex() const
{
    return aktuellerLaeuferIndex;
}
