#pragma once
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPixmap>
#include <QPointF>
#include <QPropertyAnimation>
#include <QRect>
#include <QString>
#include <QStringList>
#include <QTimer>

#include "logic/hardwaresteuerung.h"
#include "logic/display.h"
#include "logic/laufkoordination.h"
#include "logic/ortung.h"
#include "logic/kommunikationController.h"
#include "logic/herzfrequenzsensor.h"

class QResizeEvent;
class QLabel;
class QWidget;

namespace simulation {
struct GpxPoint;
class PhysicsEngine;
}

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

public:
    explicit MainWindow(simulation::PhysicsEngine *physicsEngine = nullptr,
                        Laufkoordination *laufkoordination = nullptr,
                        SystemController *kommController = nullptr,
                        Herzfrequenzsensor *herzfrequenzsensor = nullptr,
                        Ortung *laufsystemOrtung = nullptr,
                        QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void setSmartWatchValues(int heartRate,
                             const QString &pace,
                             const QString &distance,
                             int lap,
                             int runnerCount);

    void setSmartWatchStatusValues(int batteryPercent,
                                   const QString &time,
                                   int runnerCount,
                                   const QString &connectionText);

    void setSmartWatchTimerValues(const QString &pace,
                                  const QString &elapsedTime,
                                  const QString &totalMinutes,
                                  const QString &distanceToGoal);

    void setSmartWatchRunnerValues(const QStringList &runnerNames);

    void animateSmartWatchPress();

    void updateWlanComboBox(const QStringList& netze);
    void updateBluetoothComboBox(const QStringList& netze);
    void updateAntPlusComboBox(const QStringList& netze);

    void setWlanConnectionActive(bool active);
    void setBluetoothConnectionActive(bool active);
    void setAntPlusConnectionActive(bool active);

    void setDisconnected();

    void onWlanButtonClicked();
    void onBluetoothButtonClicked();
    void onAntPlusButtonClicked();


signals:
    void startStopZustandGeaendert(bool aktiv);
    void startSimulationClicked();
    void simulationSpeedChanged(float speedFactor);

    void wlanComboBoxClicked();
    void bluetoothComboBoxClicked();
    void antPlusComboBoxClicked();


    void connectWlanRequest(const QString& ssid, const QString& password);
    void connectBluetoothRequest(const QString& name);
    void connectAntPlusRequest(const QString& ID);


private slots:
    void onStartSimulationClicked();
    void onGpxPositionChanged(double lat, double lon, int index, int count);
    void redrawMap();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    Ui::MainWindow *ui;
    simulation::PhysicsEngine *physicsEngine;
    HardwareSteuerung *hardwareSteuerung;
    Laufkoordination *laufkoordination;
    SystemController *kommController;
    Herzfrequenzsensor *herzfrequenzsensor;
    Ortung *laufsystemOrtung;
    Ortung watchOrtung;

    QPixmap mapPixmap;
    QPixmap mapLayerPixmap;
    QRect mapLayerRect;

    QTimer watchLedRingAnimationTimer;
    QTimer watchChargeTimer;
    QPropertyAnimation watchShakeAnimation;
    QWidget *watchLedRingOverlay;
    QWidget *watchChargeCableWidget;
    QWidget *watchChargeCableDetachHotspot;
    QLabel *watchChargeCableStatusLabel;

    double mapMinLat;
    double mapMaxLat;
    double mapMinLon;
    double mapMaxLon;

    int highlightedGpxIndex;
    bool mapLayerDirty;
    bool watchRunnerListInitialized;
    bool simulationStarted;
    bool simulationRunning;
    float currentSpeedFactor;
    QStringList currentWatchRunnerNames;
    int watchLedRingAnimationStep;
    int currentLap;
    int lastBatteryDrainStep;
    QString connectionType;

    void animateSmartWatchVibration();
    void setConnectionLabelState(QLabel *label, bool active);

    void loadMapPixmap();
    QString findDataFile(const QString &fileName) const;
    void invalidateMapLayer();
    void rebuildMapLayer();
    QPointF mapPointForGpxPoint(const simulation::GpxPoint &point) const;
    QPixmap createMapPixmap(int highlightIndex);

    float speedFactorFromSlider(int value) const;
    void setSimulationRunning(bool running);
    void updateSpeedDisplay();
    void updateSimulationStatusDisplay();

    void updateSmartWatchRunnerListDisplay();
    void syncSmartWatchRunnersFromLaufkoordination();
    void addSmartWatchRunner();
    void switchToNextSmartWatchRunner();
    void removeSmartWatchRunner(int runnerIndex);
    void refreshSmartWatchSensorValues();
    void updateSmartWatchFromGpxPoint(const simulation::GpxPoint &point);
    QString formatDistance(float distanceMeters) const;
    QString formatElapsedTime(float seconds) const;
    int currentRunnerNumber() const;

    void setWatchMenuStateFromDisplay(sDisplay::WatchSite seite);
//->

    QString setConnectionType(QString connectionTye);

    void startWatchLedRingAnimation();
    void updateWatchLedRingAnimation();
    void setWatchLedRingColor(const QString &color, int borderWidth);
    void setWatchLedRingProgressColor(const QString &color);
    void setWatchLedRingProgress(double progress);
    void resetWatchLedRing();
    void createWatchChargeCableUi();
    void toggleWatchChargeCable();
    void setWatchChargeCableConnected(bool connected);
    void updateWatchChargeCableUi();
    void updateWatchBatteryUi();
    QString batteryColorForPercent(int percent) const;
    void onWatchChargeTimer();
    void updateBatteryFromSimulationTime(float secondsFromStart);
    int currentSimulationStep() const;
    void syncBatteryDrainStepToCurrentSimulationTime();
};

#endif
