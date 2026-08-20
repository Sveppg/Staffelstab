#include <QtTest>
#include <QSignalSpy>

#include "logic/startstopbutton.h"
#include "logic/menuebutton.h"
#include "logic/ladehub.h"
#include "logic/vibrationsmotor.h"
#include "logic/ledRing.h"
#include "logic/display.h"



/*
 * Unit-Tests für einzelne Hardwarebausteine
 * Diese werden isoliert getestet, dies ohne die zentral Hardwaresteuerung.
*/
class TestHardwareBausteine : public QObject
{
    Q_OBJECT

private slots:
    void startStopButtonSendetSignal();
    void menueButtonSendetSignal();

    void ladehubStartzustand();
    void ladehubBatterieWirdBegrenzt_data();
    void ladehubBatterieWirdBegrenzt();
    void ladehubLaedtUndEntlaedtMitGrenzen();
    void ladehubLadestatusWirdGesendet();

    void vibrationsmotorSetztRotationsspeed();
    void ledRingSetztFarbeUndStatus();

    void displayRunnerListeWirdBegrenzt();
};

/*
 * Prüft ob Start-Stop/Button beim Druecken genau ein Signal sendet.
 * meldet nur gedrueckt oder nicht gedrueckt, Flankenprinzip
*/
void TestHardwareBausteine::startStopButtonSendetSignal()
{
    StartStopButton button;

    QSignalSpy spy(
        &button,
        &StartStopButton::gedrueckt
        );

    QVERIFY(spy.isValid());

    button.druecken();

    QCOMPARE(spy.count(), 1);
}

/*
 * Prüft ob der menueButton ein Signal sendet, dient dem Seitenwechsel des Displayes
*/
void TestHardwareBausteine::menueButtonSendetSignal()
{
    MenueButton button;

    QSignalSpy spy(
        &button,
        &MenueButton::menueButtonGeklickt
        );

    QVERIFY(spy.isValid());

    button.setClicked();

    QCOMPARE(spy.count(), 1);
}

/*
 * Prüft den Anfangszustand vom Ladehub
*/
void TestHardwareBausteine::ladehubStartzustand()
{
    Ladehub ladehub;

    QCOMPARE(ladehub.getBatterieProzent(), 0);
    QCOMPARE(ladehub.getLadeStatus(), false);
}

/*
 * wird die battiere vom Ladehub begrenzt? 0-100 sollwerte keine außerhalb
 * Diese Methode enthält nur testwerte
*/
void TestHardwareBausteine::ladehubBatterieWirdBegrenzt_data()
{
    QTest::addColumn<int>("eingabe");
    QTest::addColumn<int>("erwartet");

    QTest::newRow("unterhalb_Minimum") << -10 << 0;
    QTest::newRow("untere_Grenze") << 0 << 0;
    QTest::newRow("normaler_Wert") << 73 << 73;
    QTest::newRow("obere_Grenze") << 100 << 100;
    QTest::newRow("oberhalb_Maximum") << 150 << 100;
}

/*
 * Und hier wird getestet mit den Werten darüber
*/
void TestHardwareBausteine::ladehubBatterieWirdBegrenzt()
{
    QFETCH(int, eingabe);
    QFETCH(int, erwartet);

    Ladehub ladehub;

    QSignalSpy spy(
        &ladehub,
        &Ladehub::batterieProzentGeandert
        );

    QVERIFY(spy.isValid());

    ladehub.setBatterieProzent(eingabe);

    QCOMPARE(ladehub.getBatterieProzent(), erwartet);

    if (erwartet == 0) {
        /*
         * Falls der Startwert bereits 0 ist und erneut 0 gesetzt wird,
         * entsteht keine echte Änderung.
         */
        QCOMPARE(spy.count(), 0);
    } else {
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.takeFirst().at(0).toInt(), erwartet);
    }
}

/*
 * Wird der Ladestatus per Signal gesendet?
*/
void TestHardwareBausteine::ladehubLadestatusWirdGesendet()
{
    Ladehub ladehub;

    QSignalSpy spy(
        &ladehub,
        &Ladehub::ladeStatusGeandert
        );

    QVERIFY(spy.isValid());

    ladehub.setLadestatus(true);

    QCOMPARE(ladehub.getLadeStatus(), true);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toBool(), true);
}

// Welche Grenzen sind zulässig fürs Laden und entladen? 0-100
void TestHardwareBausteine::ladehubLaedtUndEntlaedtMitGrenzen()
{
    Ladehub ladehub;

    ladehub.setBatterieProzent(95);
    ladehub.ladeProzent(10);

    QCOMPARE(ladehub.getBatterieProzent(), 100);

    ladehub.setBatterieProzent(3);
    ladehub.entladeProzent(10);

    QCOMPARE(ladehub.getBatterieProzent(), 0);
}

//Prüft ob Vibrationsmotor signal weiter gibt und ob er begrenzt (100)
void TestHardwareBausteine::vibrationsmotorSetztRotationsspeed()
{
    Vibrationsmotor motor;

    QSignalSpy spy(
        &motor,
        &Vibrationsmotor::rotationsspeedGeaendert
        );

    QVERIFY(spy.isValid());

    motor.setRotationsspeed(120);

    QCOMPARE(motor.getRotationsspeed(), 100);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toInt(), 100);
}

// Prüft Farbe und Status vom LED Ring
void TestHardwareBausteine::ledRingSetztFarbeUndStatus()
{
    LEDRing ledRing;

    QSignalSpy colorSpy(
        &ledRing,
        &LEDRing::colorGeaendert
        );

    QSignalSpy statusSpy(
        &ledRing,
        &LEDRing::statusGeaendert
        );

    QVERIFY(colorSpy.isValid());
    QVERIFY(statusSpy.isValid());

    ledRing.setColor("#39ff6a");

    QCOMPARE(colorSpy.count(), 1);
    QCOMPARE(colorSpy.takeFirst().at(0).toString(), QString("#39ff6a"));

    QCOMPARE(ledRing.getStatus(), false);

    ledRing.setStatus();

    QCOMPARE(ledRing.getStatus(), true);
    QCOMPARE(statusSpy.count(), 1);
    QCOMPARE(statusSpy.takeFirst().at(0).toBool(), true);
}


// Prüft listen begrenzung und weitergabe der Namen für Löuferlisten
void TestHardwareBausteine::displayRunnerListeWirdBegrenzt()
{
    sDisplay display;

    QSignalSpy spy(
        &display,
        &sDisplay::runnerWerteGeaendert
        );

    QVERIFY(spy.isValid());

    const QStringList runnerNames = {
        "Anna",
        "Ben",
        "Clara",
        "David",
        "Eva",
        "Felix"
    };

    const QStringList erwartet = {
        "Anna",
        "Ben",
        "Clara",
        "David",
        "Eva"
    };

    display.setRunnerWerte(runnerNames);

    QCOMPARE(spy.count(), 1);

    const QStringList empfangen =
        spy.takeFirst().at(0).toStringList();

    QCOMPARE(empfangen, erwartet);
}


QTEST_GUILESS_MAIN(TestHardwareBausteine)

#include "utst_hardwaresteuerung.moc"
