#pragma once

#include <QObject>
#include <QStringList>
#include "logic/wlanModul.h"
#include "logic/bluetoothModul.h"
#include "logic/antPlusModul.h"

class SystemController : public QObject
{
    Q_OBJECT

private:
    IWLANModul* wlan;
    IBluetoothModul* bluetooth;
    IAntPlusModul* antplus;

public:
    // explicit SystemController(QObject* parent = nullptr);
    explicit SystemController(
        IWLANModul* wlan,
        IBluetoothModul* bluetooth,
        IAntPlusModul* antplus,
        QObject* parent = nullptr);

    void addWLAN(const QString& ssid, const QString& password);
    void addBluetooth(const QString& deviceName);
    void addANTPlus(const QString& deviceID);

    bool connectWLAN(const QString& ssid, const QString& password);
    bool connectBluetooth(const QString& deviceName);
    bool connectANTPlus(const QString& deviceID);

    void getWLANNetzwerke();
    void getBluetoothDevices();
    void getAntPlusDevices();

    void disconnectAll();

signals:
    void WLANNetzwerke(QStringList);
    void BluetoothDevices(QStringList);
    void AntPlusDevices(QStringList);


};