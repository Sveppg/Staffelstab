#include "logic/kommunikationController.h"
#include <QString>
#include <logic/wlanNetzwerk.h>

// SystemController::SystemController(QObject* parent)
//     : QObject(parent)
// {
//     // wlan = new WLANModul(this);
//     // bluetooth = new BluetoothModul(this);
//     // antplus = new AntPlusModul(this);
// }
SystemController::SystemController(
    IWLANModul* wlan,
    IBluetoothModul* bluetooth,
    IAntPlusModul* antplus,
    QObject* parent)
    : QObject(parent),
      wlan(wlan),
      bluetooth(bluetooth),
      antplus(antplus)
{
}

// void SystemController::addWLAN(const QString& ssid,
//                                   const QString& password)
// {

//     WLANNetzwerk* net = new WLANNetzwerk(password, ssid);
//     wlan.addNetzwerk(net);
// }
void SystemController::addWLAN(
    const QString& ssid,
    const QString& password)
{
    wlan->addNetzwerk(ssid, password);
}

void SystemController::addBluetooth(const QString& deviceName)
{
    bluetooth->addDevice(deviceName);
}

void SystemController::addANTPlus(const QString& deviceId)
{
    antplus->addDevice(deviceId);
}

bool SystemController::connectWLAN(const QString& ssid,
                                  const QString& password)
{
    return wlan->connectToNetzwerk(ssid, password);
}

bool SystemController::connectBluetooth(const QString& deviceName)
{
    return bluetooth->connectToDevice(deviceName);
}

bool SystemController::connectANTPlus(const QString& deviceID)
{
    return antplus->connectToDevice(deviceID);
}

void SystemController::disconnectAll()
{
    wlan->disconnect();
    bluetooth->disconnect();
    antplus->disconnect();
}

void SystemController::getWLANNetzwerke()
{
    emit WLANNetzwerke(wlan->getNetzwerke());
}

void SystemController::getBluetoothDevices()
{
    emit BluetoothDevices(bluetooth->getDevices());
}

void SystemController::getAntPlusDevices()
{
    emit AntPlusDevices(antplus->getDevices());
}