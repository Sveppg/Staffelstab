#pragma once

#include <QObject>
#include <QString>

class KommunikationsModul : public QObject
{
    Q_OBJECT

public:
    explicit KommunikationsModul(QObject* parent = nullptr);
    virtual ~KommunikationsModul() = default;

    // virtual void disconnect() = 0;

signals:
    void deviceConnected(const QString& deviceName);
    void deviceDisconnected(const QString& deviceName);
};