#include <QtTest>

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTextStream>

#include "logic/laufkoordination.h"
#include "logic/herzfrequenzsensor.h"

// ============================================================
// Unit-Test fuer Laufkoordination.
//
// Reine Getter und Setter werden nicht isoliert getestet.
// Geprueft wird nur fachliche Logik:
// - Laeuferverwaltung ignoriert ungueltige Eingaben
// - Namen werden normalisiert und der erste Laeufer wird aktiv
// - Wechseln und Entfernen halten aktuellen Index und LaeuferID konsistent
// - writeLogDB() schreibt Ortungs- und Sensordaten in eine CSV-Datei
//
// Aequivalenzklassen fuer addLaeufer(name):
//   gAeK1  name nach trimmen nicht leer -> Laeufer wird aufgenommen
//   uAeK1  name nach trimmen leer       -> Eingabe wird ignoriert
//
// Aequivalenzklassen fuer removeLaeufer(index):
//   gAeK1  0 <= index < anzahl Laeufer  -> Laeufer wird entfernt
//   uAeK1  index < 0                    -> Liste bleibt unveraendert
//   uAeK2  index >= anzahl Laeufer      -> Liste bleibt unveraendert
//
// Entscheidungsueberdeckung:
// - leere Liste bei naechsterLaeufer()
// - Rotation vom letzten zurueck zum ersten Laeufer
// - Entfernen des letzten Laeufers setzt Zustand zurueck
// ============================================================

namespace {
constexpr const char* LogDateiname = "laufkoordination_log.csv";

QStringList liesLogZeilen()
{
    QFile file(LogDateiname);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    QTextStream stream(&file);
    return stream.readAll().trimmed().split('\n');
}
}

class TestLaufkoordination : public QObject {
    Q_OBJECT

private slots:
    void addLaeufer_ungueltigeNamenWerdenIgnoriert_data() {
        QTest::addColumn<QString>("name");

        QTest::newRow("leer") << QString("");
        QTest::newRow("nur Leerzeichen") << QString("   ");
    }

    void addLaeufer_ungueltigeNamenWerdenIgnoriert() {
        QFETCH(QString, name);

        Laufkoordination laufkoordination;

        laufkoordination.addLaeufer(name);

        QVERIFY(laufkoordination.getLaeuferListe().isEmpty());
        QCOMPARE(laufkoordination.getAktuellerLaeuferIndex(), -1);
        QCOMPARE(laufkoordination.getLaeuferID(), 0);
    }

    void addLaeufer_trimmtNamenUndInitialisiertAktuellenLaeufer() {
        Laufkoordination laufkoordination;

        laufkoordination.addLaeufer("  Mira  ");

        QCOMPARE(laufkoordination.getLaeuferListe(), QStringList{"Mira"});
        QCOMPARE(laufkoordination.getAktuellerLaeuferName(), QString("Mira"));
        QCOMPARE(laufkoordination.getAktuellerLaeuferIndex(), 0);
        QCOMPARE(laufkoordination.getLaeuferID(), 1);
    }

    void naechsterLaeufer_rotiertDurchListe() {
        Laufkoordination laufkoordination;

        laufkoordination.addLaeufer("Mira");
        laufkoordination.addLaeufer("Noah");

        laufkoordination.naechsterLaeufer();
        QCOMPARE(laufkoordination.getAktuellerLaeuferName(), QString("Noah"));
        QCOMPARE(laufkoordination.getLaeuferID(), 2);

        laufkoordination.naechsterLaeufer();
        QCOMPARE(laufkoordination.getAktuellerLaeuferName(), QString("Mira"));
        QCOMPARE(laufkoordination.getLaeuferID(), 1);
    }

    void naechsterLaeufer_leereListeBleibtLeer() {
        Laufkoordination laufkoordination;

        laufkoordination.naechsterLaeufer();

        QVERIFY(laufkoordination.getLaeuferListe().isEmpty());
        QCOMPARE(laufkoordination.getAktuellerLaeuferIndex(), -1);
        QCOMPARE(laufkoordination.getLaeuferID(), 0);
    }

    void removeLaeufer_ungueltigeIndizesWerdenIgnoriert_data() {
        QTest::addColumn<int>("index");

        QTest::newRow("unterhalb Minimum") << -1;
        QTest::newRow("oberhalb Maximum") << 2;
    }

    void removeLaeufer_ungueltigeIndizesWerdenIgnoriert() {
        QFETCH(int, index);

        Laufkoordination laufkoordination;
        laufkoordination.addLaeufer("Mira");
        laufkoordination.addLaeufer("Noah");

        laufkoordination.removeLaeufer(index);

        QCOMPARE(laufkoordination.getLaeuferListe(),
                 QStringList({"Mira", "Noah"}));
        QCOMPARE(laufkoordination.getAktuellerLaeuferName(), QString("Mira"));
        QCOMPARE(laufkoordination.getLaeuferID(), 1);
    }

    void removeLaeufer_aktuellerIndexWirdNachLetztemElementZurueckgesetzt() {
        Laufkoordination laufkoordination;
        laufkoordination.addLaeufer("Mira");
        laufkoordination.addLaeufer("Noah");
        laufkoordination.naechsterLaeufer();

        laufkoordination.removeLaeufer(1);

        QCOMPARE(laufkoordination.getLaeuferListe(), QStringList{"Mira"});
        QCOMPARE(laufkoordination.getAktuellerLaeuferName(), QString("Mira"));
        QCOMPARE(laufkoordination.getAktuellerLaeuferIndex(), 0);
        QCOMPARE(laufkoordination.getLaeuferID(), 1);
    }

    void removeLaeufer_letzterLaeuferSetztZustandZurueck() {
        Laufkoordination laufkoordination;
        laufkoordination.addLaeufer("Mira");

        laufkoordination.removeLaeufer(0);

        QVERIFY(laufkoordination.getLaeuferListe().isEmpty());
        QCOMPARE(laufkoordination.getAktuellerLaeuferIndex(), -1);
        QCOMPARE(laufkoordination.getLaeuferID(), 0);
    }

    void writeLogDB_schreibtHeaderUndSensordaten() {
        const QString oldCurrentPath = QDir::currentPath();
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        QVERIFY(QDir::setCurrent(tempDir.path()));

        Trittfrequenzsensor trittfrequenzsensor;
        Beschleunigungssensor beschleunigungssensor;
        Herzfrequenzsensor herzfrequenzsensor;
        Gyroskopsensor gyroskopsensor;

        trittfrequenzsensor.setTrittfrequenz(180);
        beschleunigungssensor.setBeschleunigung(1.25);
        herzfrequenzsensor.setHerzfrequenz(155);
        gyroskopsensor.setWinkelgeschwindigkeit({0.1f, 0.2f, 0.3f});

        Ortung ortung;
        ortung.setPosition({52.5f, 13.4f, 42.0f});

        Laufkoordination laufkoordination(&trittfrequenzsensor,
                                          &beschleunigungssensor,
                                          &herzfrequenzsensor,
                                          &gyroskopsensor);
        laufkoordination.addLaeufer("Mira");
        laufkoordination.writeLogDB(ortung);

        const QStringList zeilen = liesLogZeilen();
        QVERIFY(QDir::setCurrent(oldCurrentPath));

        QCOMPARE(zeilen.size(), 2);
        QCOMPARE(zeilen.at(0),
                 QString("Zeitstempel,LaeuferID,LaeuferName,PositionX,PositionY,"
                         "PositionZ,Trittfrequenz,Beschleunigung,Herzfrequenz,"
                         "GyroX,GyroY,GyroZ"));
        QVERIFY(zeilen.at(1).contains(",1,\"Mira\",52.5,13.4,42,180,1.25,155,0.1,0.2,0.3"));
    }

    void konstruktor_leertBestehendeLogDatei() {
        const QString oldCurrentPath = QDir::currentPath();
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        QVERIFY(QDir::setCurrent(tempDir.path()));

        QFile alteDatei(LogDateiname);
        QVERIFY(alteDatei.open(QIODevice::WriteOnly | QIODevice::Text));
        alteDatei.write("alter Inhalt\n");
        alteDatei.close();

        Laufkoordination laufkoordination;
        Q_UNUSED(laufkoordination);

        const QStringList zeilen = liesLogZeilen();
        QVERIFY(QDir::setCurrent(oldCurrentPath));

        QCOMPARE(zeilen.size(), 1);
        QVERIFY(!zeilen.first().contains("alter Inhalt"));
        QVERIFY(zeilen.first().startsWith("Zeitstempel,LaeuferID"));
    }
};

QTEST_MAIN(TestLaufkoordination)
#include "tst_laufkoordination.moc"
