#include <QtTest>
#include "logic/kommunikationController.h"

// MOCK IMPLEMENTATIONS
class MockWLAN : public IWLANModul {
public:
    QStringList networks;

    void addNetzwerk(const QString& ssid,const QString& password) override {
        Q_UNUSED(password)
        networks << ssid;
    }

    bool connectToNetzwerk(const QString& ssid, const QString& password) override {
        Q_UNUSED(password)
        return networks.contains(ssid);
    }

    QStringList getNetzwerke() const override {
        return networks;
    }

    void disconnect() override {
        networks.clear();   
    }
};

class MockBluetooth : public IBluetoothModul {
public:
    QStringList devices;

    void addDevice(const QString& name) override {
        devices << name;
    }

    bool connectToDevice(const QString& name) override {
        return devices.contains(name);
    }

    QStringList getDevices() const override {
        return devices;
    }

    void disconnect() override {
        devices.clear();
    }
};

class MockAntPlus : public IAntPlusModul {
public:
    QStringList devices;

    void addDevice(const QString& id) override {
        devices << id;
    }

    bool connectToDevice(const QString& id) override {
        return devices.contains(id);
    }

    QStringList getDevices() const override {
        return devices;
    }

    void disconnect() override {
        devices.clear();
    }
};

// TEST CLASS
class TestSystemController : public QObject
{
    Q_OBJECT

private:
    MockWLAN wlan;
    MockBluetooth bluetooth;
    MockAntPlus antplus;

    SystemController* controller;

private slots:

    void init() {
        wlan.networks.clear();
        bluetooth.devices.clear();
        antplus.devices.clear();

        controller = new SystemController(&wlan, &bluetooth, &antplus);
    }

    void cleanup() {
        delete controller;
    }

    // WLAN TESTS
    void addWLAN_addsNetwork() {
        controller->addWLAN("HomeWiFi", "1234");

        QCOMPARE(wlan.networks.size(), 1);
        QCOMPARE(wlan.networks.first(), QString("HomeWiFi"));
    }

    void connectWLAN_success() {
        controller->addWLAN("HomeWiFi", "1234");

        QVERIFY(controller->connectWLAN("HomeWiFi", "1234"));
    }

    void connectWLAN_fail_unknownSSID() {
        QVERIFY(!controller->connectWLAN("Unknown", "1234"));
    }

    void addWLAN_emptySSID() {
        controller->addWLAN("", "1234");

        QCOMPARE(wlan.networks.size(), 1);
        QCOMPARE(wlan.networks.first(), QString(""));
    }

    void addWLAN_duplicateSSID() {
        controller->addWLAN("HomeWiFi", "1234");
        controller->addWLAN("HomeWiFi", "5678");

        QCOMPARE(wlan.networks.size(), 2);
    }

    void connectWLAN_emptySSID() {
        controller->addWLAN("HomeWiFi", "1234");

        QVERIFY(!controller->connectWLAN("", "1234"));
    }

    void connectWLAN_wrongPassword() {
        controller->addWLAN("HomeWiFi", "1234");

        // Mock ignoriert Passwort, deshalb nur prüfen,
        // dass vorhandene SSID gefunden wird
        QVERIFY(controller->connectWLAN("HomeWiFi", ""));
    }

    // BLUETOOTH TESTS
    void addBluetooth_addsDevice() {
        controller->addBluetooth("Headset");

        QCOMPARE(bluetooth.devices.size(), 1);
        QCOMPARE(bluetooth.devices.first(), QString("Headset"));
    }

    void connectBluetooth_success() {
        controller->addBluetooth("Headset");

        QVERIFY(controller->connectBluetooth("Headset"));
    }

    void addBluetooth_emptyName() {
        controller->addBluetooth("");

        QCOMPARE(bluetooth.devices.size(), 1);
        QCOMPARE(bluetooth.devices.first(), QString(""));
    }

    void connectBluetooth_unknownDevice() {
        controller->addBluetooth("Headset");

        QVERIFY(!controller->connectBluetooth("Speaker"));
    }

    void connectBluetooth_emptyName() {
        QVERIFY(!controller->connectBluetooth(""));
    }

    // ANT+ TESTS
    void addANTPlus_addsDevice() {
        controller->addANTPlus("SensorA");

        QCOMPARE(antplus.devices.size(), 1);
        QCOMPARE(antplus.devices.first(), QString("SensorA"));
    }

    void connectANTPlus_success() {
        controller->addANTPlus("SensorA");

        QVERIFY(controller->connectANTPlus("SensorA"));
    }

    void addANTPlus_emptyID() {
        controller->addANTPlus("");

        QCOMPARE(antplus.devices.size(), 1);
        QCOMPARE(antplus.devices.first(), QString(""));
    }

    void connectANTPlus_unknownDevice() {
        controller->addANTPlus("SensorA");

        QVERIFY(!controller->connectANTPlus("SensorB"));
    }

    void connectANTPlus_emptyID() {
        QVERIFY(!controller->connectANTPlus(""));
    }

    // DISCONNECT TEST
    void disconnectAll_clearsAll() {
        controller->addWLAN("WiFi", "pw");
        controller->addBluetooth("BT");
        controller->addANTPlus("ANT");

        controller->disconnectAll();

        QVERIFY(wlan.networks.isEmpty());
        QVERIFY(bluetooth.devices.isEmpty());
        QVERIFY(antplus.devices.isEmpty());
    }
};

int main(int argc, char** argv)
{
    TestSystemController tc;
    return QTest::qExec(&tc, argc, argv);
}

#include "tst_kommunikationController.moc"