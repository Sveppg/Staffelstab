#pragma once


#include "logic/kommunikationsModul.h"
#include <QString>
#include <QStringList>

class IBluetoothModul
{
public:
    virtual ~IBluetoothModul() = default;

    virtual void addDevice(
        const QString& deviceName) = 0;

    virtual bool connectToDevice(
        const QString& deviceName) = 0;

    virtual QStringList getDevices() const = 0;

    virtual void disconnect() = 0;
};

class BluetoothModul : public KommunikationsModul, public IBluetoothModul
{
    Q_OBJECT

private:
    QStringList devices; // MAC-Adressen
    QString connectedDevice;

public:
    explicit BluetoothModul(QObject* parent = nullptr);

    void addDevice(const QString& deviceName) override;

    QStringList getDevices() const override;

    bool connectToDevice(const QString& name) override;
    void disconnect() override;
};