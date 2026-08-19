#include "logic/wlanNetzwerk.h"

WLANNetzwerk::WLANNetzwerk(const QString& passwort, const QString& ssid, QObject* parent)
    : QObject(parent),
      Passwort(passwort),
      SSID(ssid){
}

void WLANNetzwerk::setPasswort(const QString& passwort){
    Passwort = passwort;
}

void WLANNetzwerk::setSSID(const QString& ssid){
    SSID = ssid;
}

QString WLANNetzwerk::getPasswort() const{
    return Passwort;
}

QString WLANNetzwerk::getSSID() const{
    return SSID;
}