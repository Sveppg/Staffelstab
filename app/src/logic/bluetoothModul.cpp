#include "logic/bluetoothModul.h"

BluetoothModul::BluetoothModul(QObject* parent)
    : KommunikationsModul(parent)
{
}

void BluetoothModul::addDevice(const QString& deviceName)
{
    if (deviceName.isEmpty() || devices.contains(deviceName)) {
        return;
    }

    devices.append(deviceName);
}

bool BluetoothModul::connectToDevice(const QString& name)
{
    if (name.isEmpty() || !devices.contains(name)) {
        return false;
    }

    connectedDevice = name;
    emit deviceConnected(name);

    return true;
}

void BluetoothModul::disconnect()
{
    if (!connectedDevice.isEmpty()) {

        emit deviceDisconnected(connectedDevice);

        connectedDevice.clear();
    }
}

QStringList BluetoothModul::getDevices() const
{
    return devices;
}