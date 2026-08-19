#include <QtTest>

#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTextStream>

#include "laufsystem.h"

// ============================================================
// Systemtest fuer das Laufsystem.
//
// Im Gegensatz zu den Unit-Tests werden hier mehrere reale
// Bausteine gemeinsam getestet:
// - PhysicsEngine laedt den echten GPX-Track aus app/data
// - PhysicsEngine sendet simulierte Sensordaten
// - Laufsystem uebernimmt diese Daten in Ortung und Sensoren
// - Laufkoordination verwaltet die Laeufer und schreibt die CSV
//
// Der Test prueft damit den zentralen End-to-End-Ablauf vom
// simulierten Lauf bis zur protokollierten Systemausgabe.
// ============================================================

namespace {
constexpr const char* LogDateiname = "laufkoordination_log.csv";
constexpr float Epsilon = 0.0001f;

QStringList liesLogZeilen()
{
    QFile file(LogDateiname);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    QTextStream stream(&file);
    return stream.readAll().trimmed().split('\n');
}

void pruefeVector3(const Vector3 &actual,
                   float expectedX,
                   float expectedY,
                   float expectedZ)
{
    QVERIFY(qAbs(actual[0] - expectedX) < Epsilon);
    QVERIFY(qAbs(actual[1] - expectedY) < Epsilon);
    QVERIFY(qAbs(actual[2] - expectedZ) < Epsilon);
}

QString csvWert(const QString &wert)
{
    QString escaped = wert;
    escaped.replace('"', "\"\"");
    return QString("\"%1\"").arg(escaped);
}
}

class SystemtestLaufsystem : public QObject {
    Q_OBJECT

private slots:
    void kompletterLaufablauf_uebernimmtSensorenLaeuferUndSchreibtLog() {
        const QString oldCurrentPath = QDir::currentPath();
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        QVERIFY(QDir::setCurrent(tempDir.path()));

        Laufsystem laufsystem;
        auto &physicsEngine = laufsystem.getPhysicsEngine();
        auto &laufkoordination = laufsystem.getLaufkoordination();

        laufkoordination.addLaeufer("  Mira  ");
        laufkoordination.addLaeufer("Noah");

        QSignalSpy sensorSpy(
            &physicsEngine,
            &simulation::PhysicsEngine::simulatedSensorDataChanged
            );
        QVERIFY(sensorSpy.isValid());

        QVERIFY(physicsEngine.loadGpxTrack());
        QVERIFY(physicsEngine.getGpxPointCount() > 1);

        const int targetIndex = qMin(3, physicsEngine.getGpxPointCount() - 1);
        const simulation::GpxPoint targetPoint =
            physicsEngine.getGpxPoint(targetIndex);

        physicsEngine.onTick(targetPoint.secondsFromStart);

        QVERIFY(sensorSpy.count() >= 2);
        QCOMPARE(laufkoordination.getLaeuferListe(),
                 QStringList({"Mira", "Noah"}));
        QCOMPARE(laufkoordination.getAktuellerLaeuferName(), QString("Mira"));
        QCOMPARE(laufkoordination.getLaeuferID(), 1);

        pruefeVector3(laufsystem.getOrtung().getPosition(),
                      static_cast<float>(targetPoint.lat),
                      static_cast<float>(targetPoint.lon),
                      static_cast<float>(targetPoint.elevationMeters));
        if (targetIndex < physicsEngine.getGpxPointCount() - 1) {
            QVERIFY(laufsystem.getOrtung().getNearFieldDistance() > 0.0f);
        } else {
            QVERIFY(qAbs(laufsystem.getOrtung().getNearFieldDistance()) < Epsilon);
        }
        QCOMPARE(laufsystem.getTrittfrequenzsensor().getTrittfrequenz(),
                 targetPoint.cadenceSpm);
        QCOMPARE(laufsystem.getBeschleunigungssensor().getBeschleunigung(),
                 static_cast<double>(targetPoint.accelerationMps2));
        QCOMPARE(laufsystem.getHerzfrequenzsensor().getHerzfrequenz(),
                 targetPoint.heartRateBpm);
        pruefeVector3(laufsystem.getGyroskopsensor().getWinkelgeschwindigkeit(),
                      0.0f,
                      0.0f,
                      targetPoint.yawRateDps);

        laufkoordination.naechsterLaeufer();
        QCOMPARE(laufkoordination.getAktuellerLaeuferName(), QString("Noah"));
        QCOMPARE(laufkoordination.getLaeuferID(), 2);

        physicsEngine.onTick(1.0f);

        const QStringList zeilen = liesLogZeilen();
        QVERIFY(QDir::setCurrent(oldCurrentPath));

        QVERIFY(zeilen.size() >= 3);
        QCOMPARE(zeilen.first(),
                 QString("Zeitstempel,LaeuferID,LaeuferName,PositionX,PositionY,"
                         "PositionZ,Trittfrequenz,Beschleunigung,Herzfrequenz,"
                         "GyroX,GyroY,GyroZ"));

        const QString letzteZeile = zeilen.last();
        QVERIFY(letzteZeile.contains(",2," + csvWert("Noah") + ","));
        QVERIFY(letzteZeile.contains(
            QString(",%1,").arg(laufsystem.getTrittfrequenzsensor()
                                    .getTrittfrequenz())));
        QVERIFY(letzteZeile.contains(
            QString(",%1,").arg(laufsystem.getHerzfrequenzsensor()
                                    .getHerzfrequenz())));
    }
};

QTEST_MAIN(SystemtestLaufsystem)
#include "tst_laufsystem.moc"
