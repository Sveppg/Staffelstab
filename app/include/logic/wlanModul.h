#pragma once

#include "wlanNetzwerk.h"

#include "logic/kommunikationsModul.h"
#include "logic/wlanNetzwerk.h"
#include <QVector>
#include <memory>

class IWLANModul
{
public:
    virtual ~IWLANModul() = default;

    virtual void addNetzwerk(
        const QString& ssid,
        const QString& password) = 0;

    virtual bool connectToNetzwerk(
        const QString& ssid,
        const QString& password) = 0;

    virtual QStringList getNetzwerke() const = 0;

    virtual void disconnect() = 0;
};

class WLANModul : public KommunikationsModul, public IWLANModul
{
    Q_OBJECT

private:
    // QVector<WLANNetzwerk*> netzwerke;
    QVector<WLANNetzwerk*> netzwerke;
    WLANNetzwerk* connectedNetzwerk;

public:
    explicit WLANModul(QObject* parent = nullptr);
    ~WLANModul();

    // void addNetzwerk(WLANNetzwerk* netzwerk);
    void addNetzwerk(const QString& ssid, const QString& password) override;

    bool connectToNetzwerk(const QString& ssid, const QString& password) override;
    void disconnect() override;
    QStringList getNetzwerke() const override;
};