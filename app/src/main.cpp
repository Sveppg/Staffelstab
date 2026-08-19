#include <QApplication>
#include <QObject>
#include <QComboBox>
#include "mainwindow.h"
#include "laufsystem.h"
#include "logic/kommunikationController.h"
#include "logic/herzfrequenzsensor.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    WLANModul wlan;
    BluetoothModul bluetooth;
    AntPlusModul antplus;

    SystemController kommController(&wlan, &bluetooth, &antplus);
    
    Herzfrequenzsensor herzfrequenzsensor;
    Laufsystem laufsystem;
    MainWindow window(&laufsystem.getPhysicsEngine(),
                      &laufsystem.getLaufkoordination(),
                      &kommController,
                      &herzfrequenzsensor,
                      &laufsystem.getOrtung());


    kommController.addWLAN("WLAND", "1234");
    kommController.addWLAN("IchWilLAN", "1234");

    kommController.addANTPlus("359183");
    kommController.addANTPlus("458599");
    kommController.addANTPlus("323004");

    kommController.addBluetooth("MusikBox");
    kommController.addBluetooth("Kopfhörer");

    QObject::connect(&laufsystem.getClock(), &simulation::Clock::deltaTimeSeconds,
        &laufsystem.getPhysicsEngine(), &simulation::PhysicsEngine::onTick);
    QObject::connect(
        &window,
        &MainWindow::startStopZustandGeaendert,
        &laufsystem.getClock(),
        [&laufsystem](bool aktiv) {
            if (aktiv) {
                laufsystem.getClock().start();
            } else {
                laufsystem.getClock().stop();
            }
        }
        );
    QObject::connect(
        &window,
        &MainWindow::startStopZustandGeaendert,
        &laufsystem.getPhysicsEngine(),
        [&laufsystem](bool aktiv) {
            if (aktiv) {
                laufsystem.getPhysicsEngine().startSimulation();
            } else {
                laufsystem.getPhysicsEngine().stopSimulation();
            }
        }
        );
    QObject::connect(&window, &MainWindow::startSimulationClicked,
        &laufsystem.getPhysicsEngine(), &simulation::PhysicsEngine::resetSimulation);
    QObject::connect(&window, &MainWindow::simulationSpeedChanged,
        &laufsystem.getClock(), &simulation::Clock::setSpeedFactor);
    

    // send signal that comboBox was clicked
    QObject::connect(&window, &MainWindow::wlanComboBoxClicked,
        &kommController, &SystemController::getWLANNetzwerke);
    QObject::connect(&window, &MainWindow::bluetoothComboBoxClicked,
        &kommController, &SystemController::getBluetoothDevices);
    QObject::connect(&window, &MainWindow::antPlusComboBoxClicked,
        &kommController, &SystemController::getAntPlusDevices);

    // update ComboBox
    QObject::connect(&kommController, &SystemController::WLANNetzwerke,
        &window, &MainWindow::updateWlanComboBox);
    QObject::connect(&kommController, &SystemController::BluetoothDevices,
        &window, &MainWindow::updateBluetoothComboBox);
    QObject::connect(&kommController, &SystemController::AntPlusDevices,
        &window, &MainWindow::updateAntPlusComboBox);

    // "verbinde" die Devices
    QObject::connect(&window, &MainWindow::connectWlanRequest,
        &window,
        [&kommController, &window](const QString& ssid, const QString& password) {
            window.setWlanConnectionActive(kommController.connectWLAN(ssid, password));
        });
    QObject::connect(&window, &MainWindow::connectBluetoothRequest,
        &window,
        [&kommController, &window](const QString& name) {
            window.setBluetoothConnectionActive(kommController.connectBluetooth(name));
        });
    QObject::connect(&window, &MainWindow::connectAntPlusRequest,
        &window,
        [&kommController, &window](const QString& id) {
            window.setAntPlusConnectionActive(kommController.connectANTPlus(id));
        });

    // // disconnect devices
    // QObject::connect(&window, &MainWindow::wlanComboBoxClicked,
    //     &kommController, &SystemController::getWLANNetzwerke);


    window.show();

    return app.exec();
}
