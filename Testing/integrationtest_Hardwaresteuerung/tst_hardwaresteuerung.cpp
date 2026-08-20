#include <QtTest>
#include <QSignalSpy>

#include "logic/hardwaresteuerung.h"

    class TestHardwareSteuerung : public QObject
{
    Q_OBJECT

private slots:
    // Grundlegender Zustandstest -> ist der initial zustand richtig?
    void initialerZustand();
    void startStopWirdUmgeschaltet();

    // Batterietests -> wird richtig geladen und begrenzt?
    void batterieWirdUeberLadehubBegrenzt_data();
    void batterieWirdUeberLadehubBegrenzt();
    void ladeEntladeProzentWerdenAnLadehubWeitergegeben();
    void ladestatusWirdGeaendert();

    // Integrationstests: HardwareSteuerung + sDisplay -> gibt er Daten richtig weiter?
    void workoutWerteWerdenDurchDisplayWeitergegeben();
    void statusWerteWerdenDurchDisplayWeitergegeben();
    void timerWerteWerdenDurchDisplayWeitergegeben();
    void runnerWerteWerdenDurchDisplayWeitergegeben();
    void runnerListeWirdAufFuenfBegrenzt();
    void displaySeitenWerdenZyklischGewechselt();
};


/*
 * Prüft inital zustand direkt nach der Initialiserung
 * Diese darf ab start nicht aktiv sein, sie wird aktiv gesetzt
*/
void TestHardwareSteuerung::initialerZustand()
{
    HardwareSteuerung steuerung;

    QCOMPARE(steuerung.getHardwareDaten().startAktiv, false);
}

/*
 * Jeder Klick muss den Zustand invertieren und genau ein Signal
 * mit dem neuen Zustand auslösen.
*/
void TestHardwareSteuerung::startStopWirdUmgeschaltet()
{
    HardwareSteuerung steuerung;
    // überwacht signal und checkt den aufruf (startstopgeaendert)
    QSignalSpy spy(
        &steuerung,
        &HardwareSteuerung::startStopGeaendert
        );

    QVERIFY(spy.isValid());
    //erster click
    steuerung.startStopButtonGeclickt();

    QCOMPARE(steuerung.getHardwareDaten().startAktiv, true);
    QCOMPARE(spy.count(), 1);

    QList<QVariant> argumente = spy.takeFirst();

    QCOMPARE(argumente.size(), 1);
    QCOMPARE(argumente.at(0).toBool(), true);
    // zweiter click
    steuerung.startStopButtonGeclickt();

    QCOMPARE(steuerung.getHardwareDaten().startAktiv, false);
    QCOMPARE(spy.count(), 1);

    argumente = spy.takeFirst();

    QCOMPARE(argumente.size(), 1);
    QCOMPARE(argumente.at(0).toBool(), false);
}


/*
 * Es werden ein gültiger Wert sowie Werte unterhalb und oberhalb
 * des erlaubten Bereichs getestet  klassenbildung usw.
*/
void TestHardwareSteuerung::batterieWirdUeberLadehubBegrenzt_data()
{
    QTest::addColumn<int>("eingabe");
    QTest::addColumn<int>("erwartet");

    QTest::newRow("innerer Wert") << 73 << 73;
    QTest::newRow("unterhalb Minimum") << -10 << 0;
    QTest::newRow("oberhalb Maximum") << 150 << 100;
}


/*
 * Batterie nur von 0-100 oder doch weiter hinaus?
 * Zusätzlich wird geprüft, ob bei einer tatsächlichen Zustandsänderung
 * das Signal mit dem begrenzten Wert ausgesendet wird.
*/
void TestHardwareSteuerung::batterieWirdUeberLadehubBegrenzt()
{
    QFETCH(int, eingabe);
    QFETCH(int, erwartet);

    HardwareSteuerung steuerung;

    QSignalSpy spy(
        &steuerung,
        &HardwareSteuerung::batterieProzentGeandert
        );

    QVERIFY(spy.isValid());

    steuerung.setBatterieProzent(eingabe);

    QCOMPARE(
        steuerung.getHardwareDaten().batterieProzent,
        erwartet
        );

    /*
     * Der initiale Batteriestand ist bereits 0. Deshalb entsteht bei
     * einer Eingabe unterhalb des Minimums keine Zustandsänderung und
     * entsprechend auch kein Signal.
     */
    if (erwartet == 0) {
        QCOMPARE(spy.count(), 0);
    } else {
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.takeFirst().at(0).toInt(), erwartet);
    }
}

/*
 * Läd dat ding och? bzw gibt er dat och weter?
 * Die Methoden müssen die zentrale Begrenzungslogik verwenden, sodass
 * der Batteriestand weder über 100 noch unter 0 fallen kann.
*/
void TestHardwareSteuerung::ladeEntladeProzentWerdenAnLadehubWeitergegeben()
{
    HardwareSteuerung steuerung;

    QSignalSpy spy(
        &steuerung,
        &HardwareSteuerung::batterieProzentGeandert
        );

    QVERIFY(spy.isValid());

    steuerung.setBatterieProzent(99);
    spy.clear();

    steuerung.ladeBatterieProzent(5);

    QCOMPARE(
        steuerung.getHardwareDaten().batterieProzent,
        100
        );
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toInt(), 100);

    // beim entladen von 2 um 5 Prozent muss auf 0 begrenzt werden
    steuerung.setBatterieProzent(2);
    spy.clear();

    steuerung.entladeBatterieProzent(5);

    QCOMPARE(
        steuerung.getHardwareDaten().batterieProzent,
        0
        );
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toInt(), 0);
}

/*
 * Der neue Ladestatus muss gespeichert und über ein Signal weitergegeben werden.
*/
void TestHardwareSteuerung::ladestatusWirdGeaendert()
{
    HardwareSteuerung steuerung;

    QSignalSpy spy(
        &steuerung,
        &HardwareSteuerung::ladestatusGeaendert
        );

    QVERIFY(spy.isValid());

    steuerung.setLadestatus(true);

    QCOMPARE(
        steuerung.getHardwareDaten().ladeStatus,
        true
        );
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toBool(), true);
}


/*
 * prüft die Weitergabe der Workoutwerte an das Display
 * Das Signal muss alle fünf Werte unverändert und in der richtigen
 * Reihenfolge übertragen.
 */
void TestHardwareSteuerung::workoutWerteWerdenDurchDisplayWeitergegeben()
{
    HardwareSteuerung steuerung;

    QSignalSpy spy(
        &steuerung,
        &HardwareSteuerung::displayWorkoutWerteGeaendert
        );

    QVERIFY(spy.isValid());

    steuerung.setDisplayWorkoutWerte(
        168,
        "4:42 min/km",
        "3,50 km",
        2,
        4
        );

    QCOMPARE(spy.count(), 1);

    const QList<QVariant> argumente = spy.takeFirst();

    QCOMPARE(argumente.size(), 5);
    QCOMPARE(argumente.at(0).toInt(), 168);
    QCOMPARE(
        argumente.at(1).toString(),
        QString("4:42 min/km")
        );
    QCOMPARE(
        argumente.at(2).toString(),
        QString("3,50 km")
        );
    QCOMPARE(argumente.at(3).toInt(), 2);
    QCOMPARE(argumente.at(4).toInt(), 4);
}


/*
 * prüft die Weitergabe der Statuswerte an das Display
 *
 * Batteriestand, Uhrzeit, Statusnummer und Text müssen unverändert
 * übertragen werden.
 */
void TestHardwareSteuerung::statusWerteWerdenDurchDisplayWeitergegeben()
{
    HardwareSteuerung steuerung;

    QSignalSpy spy(
        &steuerung,
        &HardwareSteuerung::displayStatusWerteGeaendert
        );

    QVERIFY(spy.isValid());

    steuerung.setDisplayStatusWerte(
        65,
        "14:32",
        3,
        "GPS"
        );

    QCOMPARE(spy.count(), 1);

    const QList<QVariant> argumente = spy.takeFirst();

    QCOMPARE(argumente.size(), 4);
    QCOMPARE(argumente.at(0).toInt(), 65);
    QCOMPARE(
        argumente.at(1).toString(),
        QString("14:32")
        );
    QCOMPARE(argumente.at(2).toInt(), 3);
    QCOMPARE(
        argumente.at(3).toString(),
        QString("GPS")
        );
}


/**
 * checkt die Weitergabe der Timerwerte an das Display
 * Pace, aktuelle Zeit und Zielzeit müssen in der übergebenen
 * Reihenfolge im Signal enthalten sein.
 */
void TestHardwareSteuerung::timerWerteWerdenDurchDisplayWeitergegeben()
{
    HardwareSteuerung steuerung;

    QSignalSpy spy(
        &steuerung,
        &HardwareSteuerung::displayTimerWerteGeaendert
        );

    QVERIFY(spy.isValid());

    steuerung.setDisplayTimerWerte(
        "4:42 min/km",
        "12:38",
        "45:00"
        );

    QCOMPARE(spy.count(), 1);

    const QList<QVariant> argumente = spy.takeFirst();

    QCOMPARE(argumente.size(), 4);
    QCOMPARE(
        argumente.at(0).toString(),
        QString("4:42 min/km")
        );
    QCOMPARE(
        argumente.at(1).toString(),
        QString("12:38")
        );
    QCOMPARE(
        argumente.at(2).toString(),
        QString("45:00")
        );
    QCOMPARE(
        argumente.at(3).toString(),
        QString("--")
        );
}


/*
 * prüft die Weitergabe einer Runnerliste an das Display
 * Bei einer Liste mit höchstens fünf Einträgen müssen alle Namen
 * unverändert übertragen werden.
 */
void TestHardwareSteuerung::runnerWerteWerdenDurchDisplayWeitergegeben()
{
    HardwareSteuerung steuerung;

    QSignalSpy spy(
        &steuerung,
        &HardwareSteuerung::displayRunnerWerteGeaendert
        );

    QVERIFY(spy.isValid());

    const QStringList runnerNames = {
        "Anna",
        "Ben",
        "Clara"
    };

    steuerung.setDisplayRunnerWerte(runnerNames);

    QCOMPARE(spy.count(), 1);

    const QList<QVariant> argumente = spy.takeFirst();

    QCOMPARE(argumente.size(), 1);
    QCOMPARE(
        argumente.at(0).toStringList(),
        runnerNames
        );
}


/*
 * Bei einer Liste mit höchstens fünf Einträgen müssen alle Namen
 * unverändert übertragen werden
*/
void TestHardwareSteuerung::runnerListeWirdAufFuenfBegrenzt()
{
    HardwareSteuerung steuerung;

    QSignalSpy spy(
        &steuerung,
        &HardwareSteuerung::displayRunnerWerteGeaendert
        );

    QVERIFY(spy.isValid());

    const QStringList runnerNames = {
        "Anna",
        "Ben",
        "Clara",
        "David",
        "Eva",
        "Felix",
        "Gina"
    };

    const QStringList erwarteteRunner = {
        "Anna",
        "Ben",
        "Clara",
        "David",
        "Eva"
    };

    steuerung.setDisplayRunnerWerte(runnerNames);

    QCOMPARE(spy.count(), 1);

    const QStringList empfangeneRunner =
        spy.takeFirst().at(0).toStringList();

    QCOMPARE(empfangeneRunner.size(), 5);
    QCOMPARE(empfangeneRunner, erwarteteRunner);
}


/*
 * Wechsel der Displayseiten richtig?
 * Ausgehend von der Workoutseite müssen vier Menü-Klicks durch die
 * Seiten Timer, Status und Runner wieder zurück zu Workout führen.
*/
void TestHardwareSteuerung::displaySeitenWerdenZyklischGewechselt()
{
    HardwareSteuerung steuerung;

    QList<sDisplay::WatchSite> empfangeneSeiten;
    // anstatt spy mit enum arbeiten, so direkte verbindung
    QObject::connect(
        &steuerung,
        &HardwareSteuerung::displaySeiteGeaendert,
        this,
        [&empfangeneSeiten](sDisplay::WatchSite seite) {
            empfangeneSeiten.append(seite);
        }
        );

    // Erwarteter Anfangszustand: Workout
    steuerung.menueButtonGeclickt();
    steuerung.menueButtonGeclickt();
    steuerung.menueButtonGeclickt();
    steuerung.menueButtonGeclickt();

    QCOMPARE(empfangeneSeiten.size(), 4);

    QCOMPARE(
        empfangeneSeiten.at(0),
        sDisplay::WatchSite::Timer
        );
    QCOMPARE(
        empfangeneSeiten.at(1),
        sDisplay::WatchSite::Status
        );
    QCOMPARE(
        empfangeneSeiten.at(2),
        sDisplay::WatchSite::Runner
        );
    QCOMPARE(
        empfangeneSeiten.at(3),
        sDisplay::WatchSite::Workout
        );
}

QTEST_GUILESS_MAIN(TestHardwareSteuerung)

#include "tst_hardwaresteuerung.moc"
