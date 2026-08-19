#include "logic/antPlusModul.h"

AntPlusModul::AntPlusModul(QObject* parent)
    : KommunikationsModul(parent)
{
}

void AntPlusModul::addDevice(const QString& deviceID)
{
    if (!deviceID.isEmpty() && !deviceIDs.contains(deviceID)) {
        deviceIDs.append(deviceID);
    }
}

bool AntPlusModul::connectToDevice(const QString& ID)
{
    if (ID.isEmpty() || !deviceIDs.contains(ID)) {
        return false;
    }

    connectedDevice = ID;

    emit deviceConnected(ID);

    return true;
}

void AntPlusModul::disconnect()
{
    if (!connectedDevice.isEmpty()) {

        emit deviceDisconnected(connectedDevice);

        connectedDevice.clear();
    }
}

QStringList AntPlusModul::getDevices() const
{
    return deviceIDs;
}