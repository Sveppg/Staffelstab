#include <QtTest>

#include "logic/gpsModul.h"
#include "logic/nearFieldModul.h"
#include "logic/ortung.h"

// ============================================================
// Unit-Test fuer die Ortungsbausteine GPSModul, NearFieldModul
// und Ortung.
//
// Aus den Folien uebernommen:
// - Komponententest: einzelne Softwarebausteine werden isoliert
//   und systematisch getestet.
// - Black-Box: Aequivalenzklassenbildung und Grenzwertanalyse
//   fuer Eingabe-/Ausgabeverhalten.
// - White-Box: Entscheidungsueberdeckung fuer if/else-Zweige.
//
// GPSModul::setSensorValues()
//   gAeK1  fix == true   -> Sensordaten werden gespeichert
//   gAeK2  fix == false  -> Sensordaten werden gespeichert,
//                           Fix-Status bleibt false
//
// NearFieldModul::setDistance()
//   gAeK1  distance >= 0 -> Distanz wird gespeichert
// Ortung::setNearFieldDistance()
//   gAeK1  distance >= 0 -> Distanz wird im NearFieldModul gespeichert
//   gAeK2  distance < 0  -> Distanz wird auf 0 begrenzt
//
// Ortung::calculateGPSUWBPosition()
//   Entscheidung GPS-Fix:
//     true  -> GPS-Position + Hoehe werden als Position gesetzt
//     false -> Position wird auf 0/0/0 gesetzt
//
// Ortung::berechnePace()
//   gAeK1  speedMps > 0.05   -> Pace wird als min:sec formatiert
//   uAeK1  speedMps <= 0.05  -> Platzhalter "--:--"
//   Grenzwert: 0.05 m/s
// ============================================================

namespace {

constexpr float Epsilon = 0.0001f;

void pruefeVector2(const Vector2 &actual, float expectedX, float expectedY)
{
    QVERIFY(qAbs(actual[0] - expectedX) < Epsilon);
    QVERIFY(qAbs(actual[1] - expectedY) < Epsilon);
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

} // namespace

class TestOrtung : public QObject {
    Q_OBJECT

private slots:
    void gpsModul_startzustand() {
        GPSModul gps;

        pruefeVector2(gps.getPosition(), 0.0f, 0.0f);
        QCOMPARE(gps.getSpeed(), 0.0f);
        QCOMPARE(gps.getHeight(), 0.0f);
        QCOMPARE(gps.getFix(), false);
        QCOMPARE(gps.getSatAmount(), 0);
    }

    void gpsModul_sensorwerte_data() {
        QTest::addColumn<float>("lat");
        QTest::addColumn<float>("lon");
        QTest::addColumn<float>("speed");
        QTest::addColumn<float>("height");
        QTest::addColumn<bool>("fix");
        QTest::addColumn<int>("satAmount");

        QTest::newRow("TF1 GPS-Fix vorhanden")
            << 52.5200f << 13.4050f << 4.20f << 38.0f << true << 8;
        QTest::newRow("TF2 kein GPS-Fix")
            << 48.1372f << 11.5756f << 0.00f << 0.0f << false << 0;
    }

    void gpsModul_sensorwerte() {
        QFETCH(float, lat);
        QFETCH(float, lon);
        QFETCH(float, speed);
        QFETCH(float, height);
        QFETCH(bool, fix);
        QFETCH(int, satAmount);

        GPSModul gps;

        gps.setSensorValues({lat, lon}, speed, height, fix, satAmount);

        pruefeVector2(gps.getPosition(), lat, lon);
        QCOMPARE(gps.getSpeed(), speed);
        QCOMPARE(gps.getHeight(), height);
        QCOMPARE(gps.getFix(), fix);
        QCOMPARE(gps.getSatAmount(), satAmount);
    }

    void nearFieldModul_startzustand() {
        NearFieldModul nearField;

        QCOMPARE(nearField.getDistance(), 0.0f);
    }

    void nearFieldModul_distanceSetzen() {
        NearFieldModul nearField;

        nearField.setDistance(42.5f);

        QVERIFY(qAbs(nearField.getDistance() - 42.5f) < Epsilon);
    }

    void ortung_nearFieldDistance_data() {
        QTest::addColumn<float>("distance");
        QTest::addColumn<float>("expectedDistance");

        QTest::newRow("positive Distanz") << 123.4f << 123.4f;
        QTest::newRow("negative Distanz wird begrenzt") << -7.0f << 0.0f;
        QTest::newRow("Null Distanz") << 0.0f << 0.0f;
    }

    void ortung_nearFieldDistance() {
        QFETCH(float, distance);
        QFETCH(float, expectedDistance);

        Ortung ortung;

        ortung.setNearFieldDistance(distance);

        QVERIFY(qAbs(ortung.getNearFieldDistance() - expectedDistance) < Epsilon);
    }

    void ortung_calculateGPSUWBPosition_data() {
        QTest::addColumn<double>("lat");
        QTest::addColumn<double>("lon");
        QTest::addColumn<double>("height");
        QTest::addColumn<double>("speed");
        QTest::addColumn<bool>("fix");
        QTest::addColumn<float>("expectedX");
        QTest::addColumn<float>("expectedY");
        QTest::addColumn<float>("expectedZ");

        QTest::newRow("TF3 GPS-Fix true")
            << 52.5200 << 13.4050 << 38.0 << 4.2 << true
            << 52.5200f << 13.4050f << 38.0f;
        QTest::newRow("TF4 GPS-Fix false")
            << 52.5200 << 13.4050 << 38.0 << 4.2 << false
            << 0.0f << 0.0f << 0.0f;
    }

    void ortung_calculateGPSUWBPosition() {
        QFETCH(double, lat);
        QFETCH(double, lon);
        QFETCH(double, height);
        QFETCH(double, speed);
        QFETCH(bool, fix);
        QFETCH(float, expectedX);
        QFETCH(float, expectedY);
        QFETCH(float, expectedZ);

        Ortung ortung;

        ortung.setPosition({99.0f, 99.0f, 99.0f});
        ortung.setGpsSensorData(lat, lon, height, speed, fix);
        ortung.calculateGPSUWBPosition();

        pruefeVector3(ortung.getPosition(), expectedX, expectedY, expectedZ);
    }

    void ortung_berechnePace_grenzwerte_data() {
        QTest::addColumn<float>("speed");
        QTest::addColumn<QString>("expectedPace");

        QTest::newRow("unterhalb Grenzwert") << 0.04f << QString("--:--");
        QTest::newRow("Grenzwert")           << 0.05f << QString("--:--");
        QTest::newRow("oberhalb Grenzwert")  << 0.10f << QString("166:40");
        QTest::newRow("normaler Laufwert")   << 5.00f << QString("3:20");
    }

    void ortung_berechnePace_grenzwerte() {
        QFETCH(float, speed);
        QFETCH(QString, expectedPace);

        Ortung ortung;

        QCOMPARE(ortung.berechnePace(0.0f, 0.0f, speed), expectedPace);
    }
};

QTEST_MAIN(TestOrtung)
#include "tst_ortung.moc"
