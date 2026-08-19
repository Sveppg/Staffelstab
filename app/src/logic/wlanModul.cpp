#include "logic/wlanModul.h"
#include <memory>

WLANModul::WLANModul(QObject* parent)
    : KommunikationsModul(parent),
      connectedNetzwerk(nullptr) // Initialize connectedNetzwerk to nullptr sonst macht disconect ein crash weil es auf ein ungültigen Pointer zugreift
{
}

// void WLANModul::addNetzwerk(WLANNetzwerk* netzwerk)
// {
//     netzwerke.append(netzwerk);
// }
void WLANModul::addNetzwerk(const QString& ssid, const QString& password)
{
    // netzwerke.push_back(std::make_unique<WLANNetzwerk>(password, ssid));
    WLANNetzwerk* net = new WLANNetzwerk(password, ssid);
    netzwerke.append(net);
}

bool WLANModul::connectToNetzwerk(const QString& ssid, const QString& passwort)
{
    for (const auto& n : netzwerke) {
        if (n->getSSID() == ssid and n->getPasswort() == passwort) {

            connectedNetzwerk = n;
            emit deviceConnected(connectedNetzwerk->getSSID());

            return true;
        }
    }
    return false;
}

void WLANModul::disconnect()
{

    if (connectedNetzwerk) {

        emit deviceDisconnected(connectedNetzwerk->getSSID());

        connectedNetzwerk = nullptr;
    }
}

QStringList WLANModul::getNetzwerke() const
{
    QStringList result;

    for (const auto& netzwerk : netzwerke)
    {
        if (netzwerk)
        {
            result.append(netzwerk->getSSID());
        }
    }

    return result;
}

WLANModul::~WLANModul()
{
    // for (auto* netzwerk : netzwerke)
    // {
    //     delete netzwerk;
    // }

    // netzwerke.clear();
    qDeleteAll(netzwerke);
}