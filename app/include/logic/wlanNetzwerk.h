#pragma once


#include <QObject>
#include <QString>

class WLANNetzwerk : public QObject
{
    Q_OBJECT

private:
    QString Passwort;
    QString SSID;

public:
    explicit WLANNetzwerk(const QString& passwort, const QString& ssid, QObject* parent = nullptr);

    void setPasswort(const QString& passwort);
    void setSSID(const QString& ssid);

    QString getPasswort() const;
    QString getSSID() const;
};