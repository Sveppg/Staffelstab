#pragma once

#include "logic/kommunikationsModul.h"
#include <QStringList>
#include <QString>

class IAntPlusModul
{
public:
    virtual ~IAntPlusModul() = default;

    virtual void addDevice(
        const QString& deviceID) = 0;

    virtual bool connectToDevice(
        const QString& deviceID) = 0;

    virtual QStringList getDevices() const = 0;

    virtual void disconnect() = 0;
};

class AntPlusModul : public KommunikationsModul, public IAntPlusModul
{
    Q_OBJECT

private:
    QStringList deviceIDs; // MAC-Adressen
    QString connectedDevice;

public:
    explicit AntPlusModul(QObject* parent = nullptr);

    void addDevice(const QString& deviceID) override;

    QStringList getDevices() const override;

    bool connectToDevice(const QString& ID) override;
    void disconnect() override;
};