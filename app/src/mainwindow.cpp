#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "simulation/physicsengine.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QPainter>
#include <QPainterPath>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QTimer>
#include <QtMath>
#include <QEvent>
#include <QWidget>
#include <functional>

class WatchLedRingOverlay : public QWidget
{
public:
    explicit WatchLedRingOverlay(QWidget *parent = nullptr)
        : QWidget(parent),
        progressColor("#39ff6a"),
        color(progressColor),
        borderWidth(6),
        progress(0.0),
        flashActive(false)
    {
        setAttribute(Qt::WA_TransparentForMouseEvents);
    }

    void setRingColor(const QString &colorCode, int width)
    {
        color = QColor(colorCode);
        borderWidth = width;
        flashActive = true;
        show();
        update();
    }

    void setProgressColor(const QString &colorCode)
    {
        progressColor = QColor(colorCode);

        if (!flashActive) {
            color = progressColor;
            borderWidth = 6;
            update();
        }
    }

    void setProgress(double value)
    {
        progress = qBound(0.0, value, 1.0);

        if (progress > 0.0 || flashActive) {
            show();
        } else {
            hide();
        }

        update();
    }

    void resetFlash()
    {
        flashActive = false;
        color = progressColor;
        borderWidth = 6;

        if (progress <= 0.0) {
            hide();
        }

        update();
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event);

        //const double visibleProgress = progress > 0.0 ? progress : 1.0;
        const double visibleProgress = 1.0;
        const qreal penWidth = qMax(1, borderWidth);
        const QRectF ringRect = rect().adjusted(penWidth, penWidth, -penWidth, -penWidth);

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(QPen(QColor("#202020"), penWidth, Qt::SolidLine, Qt::RoundCap));
        painter.drawEllipse(ringRect);

        painter.setPen(QPen(color, penWidth, Qt::SolidLine, Qt::RoundCap));
        painter.drawArc(ringRect, 90 * 16, -qRound(360.0 * 16.0 * visibleProgress));
    }

private:
    QColor progressColor;
    QColor color;
    int borderWidth;
    double progress;
    bool flashActive;
};

class WatchBatteryGauge : public QProgressBar
{
public:
    explicit WatchBatteryGauge(QWidget *parent = nullptr)
        : QProgressBar(parent),
          batteryColor("#e8f600")
    {
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setFocusPolicy(Qt::NoFocus);
        setCursor(Qt::ArrowCursor);
        setTextVisible(false);
    }

    void setBatteryColor(const QString &colorCode)
    {
        batteryColor = QColor(colorCode);
        update();
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event);

        constexpr qreal nubWidth = 5.0;
        constexpr qreal nubHeight = 8.0;
        constexpr qreal nubGap = 1.0;

        const QRectF bodyRect(0.5,
                              0.5,
                              width() - nubWidth - nubGap - 1.0,
                              height() - 1.0);
        const QRectF nubRect(bodyRect.right() + nubGap - 3.0,
                             (height() - nubHeight) / 2.0,
                             nubWidth,
                             nubHeight);
        const QRectF fillBounds = bodyRect.adjusted(3.0, 3.0, -3.0, -3.0);
        const double ratio = maximum() > minimum()
            ? static_cast<double>(value() - minimum()) /
                  static_cast<double>(maximum() - minimum())
            : 0.0;
        QRectF fillRect = fillBounds;
        fillRect.setWidth(fillBounds.width() * qBound(0.0, ratio, 1.0));

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        painter.setPen(QPen(Qt::white, 1));
        painter.setBrush(Qt::white);
        painter.drawRoundedRect(nubRect, 2.0, 2.0);

        painter.setPen(QPen(Qt::white, 1));
        painter.setBrush(QColor("#050505"));
        painter.drawRoundedRect(bodyRect, 3.0, 3.0);

        if (fillRect.width() > 0.0) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(batteryColor);
            painter.drawRoundedRect(fillRect, 1.5, 1.5);
        }
    }

private:
    QColor batteryColor;
};

class WatchChargeCableWidget : public QWidget
{
public:
    explicit WatchChargeCableWidget(std::function<void(bool)> connectionChanged,
                                    QWidget *parent = nullptr)
        : QWidget(parent),
          connectionChanged(std::move(connectionChanged)),
          plugPosition(restPosition()),
          dragging(false),
          dragStartedConnected(false),
          connected(false)
    {
        setAttribute(Qt::WA_TranslucentBackground);
        setMouseTracking(true);
        setCursor(Qt::OpenHandCursor);
    }

    void setConnectedVisual(bool isConnected)
    {
        connected = isConnected;
        dragging = false;
        dragStartedConnected = false;
        dragGrabOffset = QPointF();
        plugPosition = connected ? targetPosition() : restPosition();
        lower();
        update();
    }

    QRect connectedHotspotInParent() const
    {
        const QRectF hotspot =
            plugRectAt(targetPosition()).adjusted(-22.0, -24.0, 22.0, 24.0);
        return hotspot.translated(pos()).toAlignedRect();
    }

    void beginDragFromParentPosition(const QPointF &parentPosition)
    {
        beginDragAt(parentPosition - QPointF(pos()));
    }

    void dragToParentPosition(const QPointF &parentPosition)
    {
        dragTo(parentPosition - QPointF(pos()));
    }

    void finishDragFromParentPosition()
    {
        finishDrag();
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event);

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        const QPointF base = basePosition();
        const QPointF plug = plugPosition;
        const QPointF target = targetPosition();
        const bool nearTarget =
            QLineF(plug, target).length() <= snapDistance();
        const bool dockedVisual = connected || nearTarget;

        drawCable(painter, base, plug);
        drawPlug(painter, plug, dockedVisual);
    }

    void mousePressEvent(QMouseEvent *event) override
    {
        if (plugRect().adjusted(-12, -12, 12, 12).contains(event->position())) {
            beginDragAt(event->position());
        }
    }

    void mouseMoveEvent(QMouseEvent *event) override
    {
        if (!dragging) {
            return;
        }

        dragTo(event->position());
    }

    void mouseReleaseEvent(QMouseEvent *event) override
    {
        Q_UNUSED(event);

        if (!dragging) {
            return;
        }

        finishDrag();
    }

private:
    void beginDragAt(const QPointF &position)
    {
        dragging = true;
        dragStartedConnected = connected;
        connected = false;
        dragGrabOffset = plugPosition - position;
        raise();
        setCursor(Qt::ClosedHandCursor);
        dragTo(position);
    }

    void dragTo(const QPointF &position)
    {
        if (!dragging) {
            return;
        }

        const QPointF nextPosition = position + dragGrabOffset;
        plugPosition = QPointF(qBound(40.0, nextPosition.x(), width() - 40.0),
                               qBound(20.0, nextPosition.y(), height() - 70.0));
        update();
    }

    void finishDrag()
    {
        if (!dragging) {
            return;
        }

        dragging = false;
        setCursor(Qt::OpenHandCursor);

        const double distanceToTarget =
            QLineF(plugPosition, targetPosition()).length();
        const bool pulledOutOfDock =
            dragStartedConnected && distanceToTarget > detachDistance();
        const bool snaps =
            !pulledOutOfDock && distanceToTarget <= snapDistance();

        connected = snaps;
        dragStartedConnected = false;
        dragGrabOffset = QPointF();
        plugPosition = connected ? targetPosition() : restPosition();

        if (connectionChanged) {
            connectionChanged(connected);
        }

        lower();
        update();
    }

    void drawCable(QPainter &painter, const QPointF &base, const QPointF &plug) const
    {
        const QPointF cableEnd(plug.x(), plug.y() + 17.0);

        QPainterPath cablePath;
        cablePath.moveTo(base.x(), base.y() - 3.0);

        if (connected) {
            cablePath.cubicTo(QPointF(base.x() - 11.0, base.y() - 105.0),
                              QPointF(cableEnd.x() + 8.0, cableEnd.y() + 125.0),
                              cableEnd);
        } else {
            const qreal pull = qBound(-70.0, (plug.x() - base.x()) * 0.42, 70.0);
            cablePath.cubicTo(QPointF(base.x() - 26.0 + pull, base.y() - 92.0),
                              QPointF(cableEnd.x() + 18.0 - pull, cableEnd.y() + 104.0),
                              cableEnd);
        }

        painter.setPen(QPen(QColor(0, 0, 0, 95), 10, Qt::SolidLine, Qt::RoundCap));
        painter.drawPath(cablePath.translated(2.0, 2.0));

        painter.setPen(QPen(QColor("#2e2e2e"), 8, Qt::SolidLine, Qt::RoundCap));
        painter.drawPath(cablePath);

        painter.setPen(QPen(QColor("#4a4a4a"), 2, Qt::SolidLine, Qt::RoundCap));
        painter.drawPath(cablePath);
    }

    void drawPlug(QPainter &painter, const QPointF &plug, bool dockedVisual) const
    {
        const QRectF bodyRect = plugRectAt(plug);
        const QRectF lipRect(bodyRect.left() + 4.0,
                             bodyRect.bottom() - 6.0,
                             bodyRect.width() - 8.0,
                             6.0);

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 0, 0, 105));
        painter.drawRoundedRect(bodyRect.translated(2.0, 3.0), 8, 8);

        painter.setPen(QPen(QColor("#070707"), 2));
        painter.setBrush(QColor(dockedVisual ? "#242424" : "#202020"));
        painter.drawRoundedRect(bodyRect, 7, 7);

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#303030"));
        painter.drawRoundedRect(lipRect, 3, 3);

        painter.setPen(QPen(QColor("#d8d8d8"), 2, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(QPointF(plug.x() - 13.0, plug.y() - 15.0),
                         QPointF(plug.x() - 13.0, plug.y() - 24.0));
        painter.drawLine(QPointF(plug.x() + 13.0, plug.y() - 15.0),
                         QPointF(plug.x() + 13.0, plug.y() - 24.0));

        painter.setPen(QPen(QColor("#525252"), 1));
        painter.drawLine(QPointF(bodyRect.left() + 8.0, bodyRect.top() + 4.0),
                         QPointF(bodyRect.right() - 8.0, bodyRect.top() + 4.0));
    }

    QRectF plugRect() const
    {
        return plugRectAt(plugPosition);
    }

    QRectF plugRectAt(const QPointF &position) const
    {
        return QRectF(position.x() - 26,
                      position.y() - 14,
                      52,
                      28);
    }

    QPointF basePosition() const
    {
        return QPointF(width() / 2.0, height() + 36.0);
    }

    QPointF targetPosition() const
    {
        return QPointF(width() / 2.0, 120.0);
    }

    QPointF restPosition() const
    {
        return QPointF(width() / 2.0, height() - 112.0);
    }

    double snapDistance() const
    {
        return 95.0;
    }

    double detachDistance() const
    {
        return 26.0;
    }

    std::function<void(bool)> connectionChanged;
    QPointF plugPosition;
    QPointF dragGrabOffset;
    bool dragging;
    bool dragStartedConnected;
    bool connected;
};

class WatchChargeCableDetachHotspot : public QWidget
{
public:
    explicit WatchChargeCableDetachHotspot(WatchChargeCableWidget *cableWidget,
                                           QWidget *parent = nullptr)
        : QWidget(parent),
          cableWidget(cableWidget),
          dragging(false)
    {
        setAttribute(Qt::WA_TranslucentBackground);
        setCursor(Qt::OpenHandCursor);
        setFocusPolicy(Qt::NoFocus);
    }

protected:
    void mousePressEvent(QMouseEvent *event) override
    {
        if (cableWidget == nullptr) {
            return;
        }

        dragging = true;
        setCursor(Qt::ClosedHandCursor);
        cableWidget->beginDragFromParentPosition(parentPositionFor(event));
        event->accept();
    }

    void mouseMoveEvent(QMouseEvent *event) override
    {
        if (!dragging || cableWidget == nullptr) {
            return;
        }

        cableWidget->dragToParentPosition(parentPositionFor(event));
        event->accept();
    }

    void mouseReleaseEvent(QMouseEvent *event) override
    {
        Q_UNUSED(event);

        if (!dragging || cableWidget == nullptr) {
            return;
        }

        dragging = false;
        setCursor(Qt::OpenHandCursor);
        cableWidget->finishDragFromParentPosition();
    }

private:
    QPointF parentPositionFor(QMouseEvent *event) const
    {
        return QPointF(mapToParent(event->position().toPoint()));
    }

    WatchChargeCableWidget *cableWidget;
    bool dragging;
};
MainWindow::MainWindow(simulation::PhysicsEngine *physicsEngine,
                       Laufkoordination *laufkoordination,
                       SystemController *kommController,
                       Herzfrequenzsensor *herzfrequenzsensor,
                       Ortung *laufsystemOrtung,
                       QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    physicsEngine(physicsEngine),
    hardwareSteuerung(nullptr),
    laufkoordination(laufkoordination),
    kommController(kommController),
    herzfrequenzsensor(herzfrequenzsensor),
    laufsystemOrtung(laufsystemOrtung),
    watchOrtung(this),
    watchShakeAnimation(this),
          watchLedRingOverlay(nullptr),
          watchChargeCableWidget(nullptr),
          watchChargeCableDetachHotspot(nullptr),
          watchChargeCableStatusLabel(nullptr),
    mapMinLat(0.0),
    mapMaxLat(0.0),
    mapMinLon(0.0),
    mapMaxLon(0.0),
    highlightedGpxIndex(0),
    mapLayerDirty(true),
    watchRunnerListInitialized(false),
    simulationStarted(false),
    simulationRunning(false),
    currentSpeedFactor(1.0f),
    watchLedRingAnimationStep(0),
    currentLap(1),
    lastBatteryDrainStep(0)
{
    ui->setupUi(this);

    hardwareSteuerung = new HardwareSteuerung(this);



    setConnectionLabelState(ui->wlanStatusLabel, false);
    setConnectionLabelState(ui->bluetoothStatusLabel, false);
    setConnectionLabelState(ui->antPlusStatusLabel, false);

    auto *originalBatteryGauge = ui->watchBatteryGauge;
    auto *paintedBatteryGauge =
        new WatchBatteryGauge(originalBatteryGauge->parentWidget());
    paintedBatteryGauge->setObjectName(originalBatteryGauge->objectName());
    paintedBatteryGauge->setGeometry(
        originalBatteryGauge->geometry().adjusted(0, 0, 6, 0));
    paintedBatteryGauge->setRange(originalBatteryGauge->minimum(),
                                  originalBatteryGauge->maximum());
    paintedBatteryGauge->setValue(originalBatteryGauge->value());
    paintedBatteryGauge->show();
    delete originalBatteryGauge;
    ui->watchBatteryGauge = paintedBatteryGauge;

    ui->watchBackgroundLabel->setPixmap(QPixmap(findDataFile("staffelstab_displayv2.png")));
    ui->wlanComboBox->installEventFilter(this);
    ui->bluetoothComboBox->installEventFilter(this);
    ui->antPlusComboBox->installEventFilter(this);

    watchLedRingOverlay = new WatchLedRingOverlay(ui->smartWatchFace);
    watchLedRingOverlay->setObjectName("watchLedRingOverlay");
    watchLedRingOverlay->setGeometry(58, 51, 274, 274);
    watchLedRingOverlay->hide();

    createWatchChargeCableUi();

    watchShakeAnimation.setTargetObject(ui->smartWatchFace);
    watchShakeAnimation.setPropertyName("pos");
    watchShakeAnimation.setDuration(140);

    watchLedRingOverlay->raise();
    ui->watchScreenLayer->raise();
    ui->watchModeStack->raise();
    ui->watchStartStopButton->raise();
    ui->watchRunnerNextButton->raise();
    ui->watchModeCycleButton->raise();

    loadMapPixmap();
    updateSpeedDisplay();
    updateSimulationStatusDisplay();
    setWatchMenuStateFromDisplay(sDisplay::WatchSite::Workout);

    // connect(this->physicsEngine, &simulation::PhysicsEngine::gpxPositionChanged,
    //             herzfrequenzsensor &MainWindow::onGpxPositionChanged);

    connect(ui->watchStartStopButton, &QPushButton::clicked,
            hardwareSteuerung, &HardwareSteuerung::startStopButtonGeclickt);

    connect(ui->watchModeCycleButton, &QPushButton::clicked,
            hardwareSteuerung, &HardwareSteuerung::menueButtonGeclickt);

    connect(ui->watchShakeButton, &QPushButton::clicked,
            hardwareSteuerung, &HardwareSteuerung::anfeuernButtonGeclickt);

    connect(&watchLedRingAnimationTimer, &QTimer::timeout,
            this, &MainWindow::updateWatchLedRingAnimation);

    connect(&watchChargeTimer, &QTimer::timeout,
            this, &MainWindow::onWatchChargeTimer);

    connect(ui->simulationControlButton, &QPushButton::clicked,
            this, &MainWindow::onStartSimulationClicked);
    connect(ui->watchRunnerRemoveButton1, &QPushButton::clicked,
            this, [this]() { removeSmartWatchRunner(0); });

    connect(ui->watchRunnerRemoveButton2, &QPushButton::clicked,
            this, [this]() { removeSmartWatchRunner(1); });

    connect(ui->watchRunnerRemoveButton3, &QPushButton::clicked,
            this, [this]() { removeSmartWatchRunner(2); });

    connect(ui->watchRunnerRemoveButton4, &QPushButton::clicked,
            this, [this]() { removeSmartWatchRunner(3); });

    connect(ui->watchRunnerRemoveButton5, &QPushButton::clicked,
            this, [this]() { removeSmartWatchRunner(4); });

    connect(ui->runnerAddButton, &QPushButton::clicked,
            this, &MainWindow::addSmartWatchRunner);

    connect(ui->endConnection, &QPushButton::clicked,
            kommController, &SystemController::disconnectAll);

    connect(ui->endConnection, &QPushButton::clicked,
            this, &MainWindow::setDisconnected);

    connect(ui->runnerNameLineEdit, &QLineEdit::returnPressed,
            this, &MainWindow::addSmartWatchRunner);

    connect(ui->watchRunnerNextButton, &QPushButton::clicked,
            this, &MainWindow::switchToNextSmartWatchRunner);

    connect(ui->simulationSpeedSlider, &QSlider::valueChanged,
            this, [this](int value) {
                currentSpeedFactor = speedFactorFromSlider(value);
                updateSpeedDisplay();
                emit simulationSpeedChanged(currentSpeedFactor);
            });


    connect(hardwareSteuerung,
            &HardwareSteuerung::startStopGeaendert,
            this,
            [this](bool aktiv) {
                // Beim ersten Start die Simulation zurücksetzen/initialisieren.
                if (aktiv && !simulationStarted) {
                    simulationStarted = true;
                    emit startSimulationClicked();
                }

                // MainWindow speichert nur eine Kopie für die Anzeige.
                setSimulationRunning(aktiv);

                // Den eindeutigen Zustand an main.cpp weitergeben.
                emit startStopZustandGeaendert(aktiv);
            });

    connect(hardwareSteuerung, &HardwareSteuerung::anfeuernAngefordert,
            this, &MainWindow::animateSmartWatchPress);

    connect(hardwareSteuerung, &HardwareSteuerung::colorGeaendert,
            this, &MainWindow::setWatchLedRingProgressColor);

    connect(hardwareSteuerung, &HardwareSteuerung::displaySeiteGeaendert,
            this, &MainWindow::setWatchMenuStateFromDisplay);

    connect(hardwareSteuerung, &HardwareSteuerung::displayWorkoutWerteGeaendert,
            this, &MainWindow::setSmartWatchValues);

    connect(hardwareSteuerung, &HardwareSteuerung::displayStatusWerteGeaendert,
            this, &MainWindow::setSmartWatchStatusValues);

    connect(hardwareSteuerung, &HardwareSteuerung::displayTimerWerteGeaendert,
            this, &MainWindow::setSmartWatchTimerValues);

    connect(hardwareSteuerung, &HardwareSteuerung::displayRunnerWerteGeaendert,
            this, &MainWindow::setSmartWatchRunnerValues);

    connect(hardwareSteuerung, &HardwareSteuerung::batterieProzentGeandert,
            this, [this](int prozent) {
                Q_UNUSED(prozent);
                updateWatchBatteryUi();
            });

    connect(hardwareSteuerung, &HardwareSteuerung::ladestatusGeaendert,
            this, [this](bool aktiv) {
                Q_UNUSED(aktiv);
                updateWatchBatteryUi();
                updateWatchChargeCableUi();
            });

    // emit signals to main.cpp that the buttons where clicked
    connect(ui->connectWlanButton, &QPushButton::clicked,
        this, &MainWindow::onWlanButtonClicked);
    connect(ui->connectBluetoothButton, &QPushButton::clicked,
        this, &MainWindow::onBluetoothButtonClicked);
    connect(ui->connectAntPlusButton, &QPushButton::clicked,
        this, &MainWindow::onAntPlusButtonClicked);

    if (this->physicsEngine != nullptr) {
        connect(this->physicsEngine, &simulation::PhysicsEngine::gpxPositionChanged,
                this, &MainWindow::onGpxPositionChanged);

        this->physicsEngine->loadGpxTrack();
        invalidateMapLayer();
        redrawMap();
    }

    hardwareSteuerung->setBatterieProzent(65);
    hardwareSteuerung->setLadestatus(false);
    syncBatteryDrainStepToCurrentSimulationTime();
    updateWatchBatteryUi();
    updateWatchChargeCableUi();


    syncSmartWatchRunnersFromLaufkoordination();
    refreshSmartWatchSensorValues();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::onStartSimulationClicked()
{
    currentLap = 1;
    lastBatteryDrainStep = 0;
    watchOrtung.resetPaceBerechnung();
    emit startSimulationClicked();
    refreshSmartWatchSensorValues();
    updateSimulationStatusDisplay();
}

void MainWindow::onGpxPositionChanged(double lat, double lon, int index, int count)
{
    Q_UNUSED(lat);
    Q_UNUSED(lon);

    highlightedGpxIndex = index;
    setWatchLedRingProgress(count > 1
                                ? static_cast<double>(index) / static_cast<double>(count - 1)
                                : 0.0);

    if (physicsEngine != nullptr) {
        herzfrequenzsensor->setHerzfrequenz(physicsEngine->getGpxPoint(index).heartRateBpm);
        updateSmartWatchFromGpxPoint(physicsEngine->getGpxPoint(index));
    }

    redrawMap();
}

void MainWindow::setSmartWatchValues(int heartRate,
                                     const QString &pace,
                                     const QString &distance,
                                     int lap,
                                     int runnerNumber)
{
    ui->watchHeartValueLabel->setText(QString::number(heartRate));
    ui->watchPaceValueLabel->setText(pace);
    ui->watchDistanceLabel->setText(distance);
    ui->watchLapValueLabel->setText(QString::number(lap));
    ui->watchRunnerValueLabel->setText(QString("%1").arg(runnerNumber, 2, 10, QChar('0')));
}

void MainWindow::setSmartWatchStatusValues(int batteryPercent,
                                           const QString &time,
                                           int runnerNumber,
                                           const QString &connectionText)
{
    Q_UNUSED(batteryPercent);

    ui->watchStatusTimeLabel->setText(time);
    updateWatchBatteryUi();
    ui->watchStatusRunnerValueLabel->setText(QString("%1").arg(runnerNumber, 2, 10, QChar('0')));
//    ui->watchWifiStatusLabel->setText(connectionText);
}

void MainWindow::setSmartWatchTimerValues(const QString &pace,
                                          const QString &elapsedTime,
                                          const QString &totalMinutes,
                                          const QString &distanceToGoal)
{
    ui->watchTimerPaceLabel->setText(pace);
    ui->watchTimerElapsedLabel->setText(elapsedTime);
    ui->watchTimerTotalLabel->setText(totalMinutes);
    ui->watchTimerGoalDistanceValueLabel->setText(distanceToGoal);
}

void MainWindow::setSmartWatchRunnerValues(const QStringList &runnerNames)
{
    currentWatchRunnerNames = runnerNames.mid(0, 5);
    watchRunnerListInitialized = true;
    updateSmartWatchRunnerListDisplay();
}

void MainWindow::setWatchMenuStateFromDisplay(sDisplay::WatchSite seite)
{
    switch (seite) {
    case sDisplay::WatchSite::Workout:
        ui->watchModeStack->setCurrentIndex(
            ui->watchModeStack->indexOf(ui->watchWorkoutPage)
            );
        break;

    case sDisplay::WatchSite::Timer:
        ui->watchModeStack->setCurrentIndex(
            ui->watchModeStack->indexOf(ui->watchTimerPage)
            );
        break;

    case sDisplay::WatchSite::Status:
        ui->watchModeStack->setCurrentIndex(
            ui->watchModeStack->indexOf(ui->watchStatusPage)
            );
        break;

    case sDisplay::WatchSite::Runner:
        ui->watchModeStack->setCurrentIndex(
            ui->watchModeStack->indexOf(ui->watchRunnerPage)
            );
        break;
    }
}

void MainWindow::refreshSmartWatchSensorValues()
{
    if (physicsEngine != nullptr && physicsEngine->getGpxPointCount() > 0) {
        updateSmartWatchFromGpxPoint(physicsEngine->getGpxPoint(highlightedGpxIndex));
    }
}

// QString MainWindow::setConnectionType(connectionType){ // for 838

// }


void MainWindow::updateSmartWatchFromGpxPoint(const simulation::GpxPoint &point)
{
    if (hardwareSteuerung == nullptr) {
        return;
    }

    const QString pace = watchOrtung.berechnePace(point.secondsFromStart,
                                                  point.distanceFromStartMeters,
                                                  point.speedMps);
    const QString distance = formatDistance(point.distanceFromStartMeters);
    const QString elapsedTime = formatElapsedTime(point.secondsFromStart);
    const int runnerNumber = currentRunnerNumber();
    const QString watchTime = point.timestamp.isValid()
        ? point.timestamp.toLocalTime().toString("HH:mm")
        : QString("--:--");

    updateBatteryFromSimulationTime(point.secondsFromStart);

    float totalSeconds = point.secondsFromStart;
    if (physicsEngine != nullptr && physicsEngine->getGpxPointCount() > 0) {
        totalSeconds = physicsEngine->getGpxPoint(physicsEngine->getGpxPointCount() - 1)
                           .secondsFromStart;
    }
    const QString distanceToGoal = laufsystemOrtung != nullptr
        ? formatDistance(laufsystemOrtung->getNearFieldDistance())
        : QString("--");

    hardwareSteuerung->setDisplayWorkoutWerte( herzfrequenzsensor->getHerzfrequenz(),
                                              pace,
                                              distance,
                                              currentLap,
                                              runnerNumber);

    hardwareSteuerung->setDisplayStatusWerte(hardwareSteuerung->getHardwareDaten().batterieProzent,
                                             watchTime,
                                             runnerNumber,
                                             "WBA"); // connectionType für verschiedene typen

    hardwareSteuerung->setDisplayTimerWerte(pace,
                                            elapsedTime,
                                            formatElapsedTime(totalSeconds),
                                            distanceToGoal);
}

void MainWindow::createWatchChargeCableUi()
{
    watchChargeCableWidget =
        new WatchChargeCableWidget(
            [this](bool connected) {
                setWatchChargeCableConnected(connected);
            },
            ui->watchAreaFrame);
    watchChargeCableWidget->setObjectName("watchChargeCableWidget");
    watchChargeCableWidget->setGeometry(0, 300, ui->watchAreaFrame->width(), 430);

    watchChargeCableDetachHotspot =
        new WatchChargeCableDetachHotspot(
            static_cast<WatchChargeCableWidget *>(watchChargeCableWidget),
            ui->watchAreaFrame);
    watchChargeCableDetachHotspot->setObjectName("watchChargeCableDetachHotspot");
    watchChargeCableDetachHotspot->setStyleSheet(
        "QWidget#watchChargeCableDetachHotspot {"
        "background: transparent;"
        "border: none;"
        "}"
        );
    watchChargeCableDetachHotspot->hide();

    watchChargeCableStatusLabel = new QLabel(ui->watchAreaFrame);
    watchChargeCableStatusLabel->setObjectName("watchChargeCableStatusLabel");
    watchChargeCableStatusLabel->setGeometry(50, 700, 300, 24);
    watchChargeCableStatusLabel->setAlignment(Qt::AlignCenter);
    watchChargeCableStatusLabel->setStyleSheet(
        "QLabel#watchChargeCableStatusLabel {"
        "color: #303030;"
        "font-size: 12px;"
        "}"
        );

    watchChargeCableWidget->stackUnder(ui->smartWatchFace);
    watchChargeCableDetachHotspot->raise();
    watchChargeCableStatusLabel->raise();
    watchChargeCableStatusLabel->hide();
}

void MainWindow::toggleWatchChargeCable()
{
    if (hardwareSteuerung == nullptr) {
        return;
    }

    const bool currentlyCharging =
        hardwareSteuerung->getHardwareDaten().ladeStatus;

    hardwareSteuerung->setLadestatus(!currentlyCharging);
}

void MainWindow::setWatchChargeCableConnected(bool connected)
{
    if (hardwareSteuerung == nullptr) {
        return;
    }

    syncBatteryDrainStepToCurrentSimulationTime();
    hardwareSteuerung->setLadestatus(connected);
}

void MainWindow::updateWatchChargeCableUi()
{
    if (hardwareSteuerung == nullptr ||
        watchChargeCableWidget == nullptr ||
        watchChargeCableStatusLabel == nullptr) {
        return;
    }

    const bool charging = hardwareSteuerung->getHardwareDaten().ladeStatus;

    auto *cableWidget =
        static_cast<WatchChargeCableWidget *>(watchChargeCableWidget);

    cableWidget->setConnectedVisual(charging);
    watchChargeCableWidget->stackUnder(ui->smartWatchFace);

    if (watchChargeCableDetachHotspot != nullptr) {
        watchChargeCableDetachHotspot->setGeometry(
            cableWidget->connectedHotspotInParent());
        watchChargeCableDetachHotspot->setVisible(charging);

        if (charging) {
            watchChargeCableDetachHotspot->raise();
        }
    }

    watchChargeCableStatusLabel->clear();

    if (charging && !watchChargeTimer.isActive()) {
        watchChargeTimer.start(1000);
    } else if (!charging) {
        watchChargeTimer.stop();
    }
}

void MainWindow::updateWatchBatteryUi()
{
    if (hardwareSteuerung == nullptr) {
        return;
    }

    const int percent =
        qBound(0, hardwareSteuerung->getHardwareDaten().batterieProzent, 100);
    const bool charging = hardwareSteuerung->getHardwareDaten().ladeStatus;
    const QString color = batteryColorForPercent(percent);

    ui->watchBatteryGauge->setValue(percent);
    static_cast<WatchBatteryGauge *>(ui->watchBatteryGauge)->setBatteryColor(color);
    ui->watchBatteryValueLabel->setText(charging
                                            ? QString("%1% Laden").arg(percent)
                                            : QString("%1%").arg(percent));
    ui->watchBatteryValueLabel->setStyleSheet(QString("color: %1;").arg(color));
}

QString MainWindow::batteryColorForPercent(int percent) const
{
    const int value = qBound(0, percent, 100);
    const QColor red("#ff2f2f");
    const QColor yellow("#e8f600");
    const QColor green("#39ff6a");

    const QColor from = value <= 50 ? red : yellow;
    const QColor to = value <= 50 ? yellow : green;
    const double t = value <= 50
        ? value / 50.0
        : (value - 50) / 50.0;

    const int r = qRound(from.red() + (to.red() - from.red()) * t);
    const int g = qRound(from.green() + (to.green() - from.green()) * t);
    const int b = qRound(from.blue() + (to.blue() - from.blue()) * t);

    return QColor(r, g, b).name();
}

void MainWindow::onWatchChargeTimer()
{
    if (hardwareSteuerung == nullptr ||
        !hardwareSteuerung->getHardwareDaten().ladeStatus) {
        return;
    }

    hardwareSteuerung->ladeBatterieProzent(1);
}

void MainWindow::updateBatteryFromSimulationTime(float secondsFromStart)
{
    if (hardwareSteuerung == nullptr) {
        return;
    }

    const int currentStep = qMax(0, qFloor(secondsFromStart / 60.0f));

    if (currentStep <= lastBatteryDrainStep) {
        return;
    }

    if (hardwareSteuerung->getHardwareDaten().ladeStatus) {
        lastBatteryDrainStep = currentStep;
        return;
    }

    const int stepsPassed = currentStep - lastBatteryDrainStep;
    lastBatteryDrainStep = currentStep;
    hardwareSteuerung->entladeBatterieProzent(stepsPassed);
}

int MainWindow::currentSimulationStep() const
{
    if (physicsEngine == nullptr || physicsEngine->getGpxPointCount() <= 0) {
        return 0;
    }

    const int index = qBound(0, highlightedGpxIndex, physicsEngine->getGpxPointCount() - 1);
    return qMax(0, qFloor(physicsEngine->getGpxPoint(index).secondsFromStart / 60.0f));
}

void MainWindow::syncBatteryDrainStepToCurrentSimulationTime()
{
    lastBatteryDrainStep = currentSimulationStep();
}

QString MainWindow::formatDistance(float distanceMeters) const
{
    const int centiKilometers = qMax(0, qRound(distanceMeters / 10.0f));
    return QString("%1,%2 km")
        .arg(centiKilometers / 100)
        .arg(centiKilometers % 100, 2, 10, QChar('0'));
}

QString MainWindow::formatElapsedTime(float seconds) const
{
    const int totalSeconds = qMax(0, qRound(seconds));
    const int hours = totalSeconds / 3600;
    const int minutes = (totalSeconds / 60) % 60;
    const int remainingSeconds = totalSeconds % 60;

    if (hours > 0) {
        return QString("%1:%2:%3")
            .arg(hours)
            .arg(minutes, 2, 10, QChar('0'))
            .arg(remainingSeconds, 2, 10, QChar('0'));
    }

    return QString("%1:%2")
        .arg(minutes)
        .arg(remainingSeconds, 2, 10, QChar('0'));
}

int MainWindow::currentRunnerNumber() const
{
    if (laufkoordination == nullptr) {
        return 0;
    }

    const int currentRunnerIndex = laufkoordination->getAktuellerLaeuferIndex();
    return currentRunnerIndex >= 0 ? currentRunnerIndex + 1 : 0;
}

void MainWindow::animateSmartWatchVibration()
{
    const QPoint startPosition = ui->smartWatchFace->pos();

    watchShakeAnimation.stop();
    watchShakeAnimation.setStartValue(startPosition);
    watchShakeAnimation.setKeyValueAt(0.25, startPosition + QPoint(3, 0));
    watchShakeAnimation.setKeyValueAt(0.50, startPosition + QPoint(-3, 0));
    watchShakeAnimation.setKeyValueAt(0.75, startPosition + QPoint(2, 0));
    watchShakeAnimation.setEndValue(startPosition);
    watchShakeAnimation.start();
}

void MainWindow::animateSmartWatchPress()
{
    animateSmartWatchVibration();
    startWatchLedRingAnimation();
}

void MainWindow::startWatchLedRingAnimation()
{
    watchLedRingAnimationStep = 0;
    watchLedRingAnimationTimer.stop();
    updateWatchLedRingAnimation();
    watchLedRingAnimationTimer.start(90);
}

void MainWindow::updateWatchLedRingAnimation()
{
    static const QStringList colors = {
        "#e8f600",
        "#00e5ff",
        "#ff2f92",
        "#39ff6a",
        "#ff9f1c",
        "#7c4dff"
    };

    const int animationLength = 20;

    if (watchLedRingAnimationStep >= animationLength) {
        watchLedRingAnimationTimer.stop();
        resetWatchLedRing();
        return;
    }

    const bool blinkOn = watchLedRingAnimationStep % 2 == 0;
    const QString color = colors.at(watchLedRingAnimationStep % colors.size());

    setWatchLedRingColor(blinkOn ? color : "#161616",
                         blinkOn ? 6 : 2);

    ++watchLedRingAnimationStep;
}

void MainWindow::setWatchLedRingColor(const QString &color, int borderWidth)
{
    static_cast<WatchLedRingOverlay *>(watchLedRingOverlay)->setRingColor(color, borderWidth);
}

void MainWindow::setWatchLedRingProgressColor(const QString &color)
{
    static_cast<WatchLedRingOverlay *>(watchLedRingOverlay)->setProgressColor(color);
}

void MainWindow::setWatchLedRingProgress(double progress)
{
    static_cast<WatchLedRingOverlay *>(watchLedRingOverlay)->setProgress(progress);
}

void MainWindow::resetWatchLedRing()
{
    static_cast<WatchLedRingOverlay *>(watchLedRingOverlay)->resetFlash();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    invalidateMapLayer();
    redrawMap();
}

void MainWindow::loadMapPixmap()
{
    mapPixmap.load(findDataFile("olympia_karte_better_res.png"));
    invalidateMapLayer();
}

QString MainWindow::findDataFile(const QString &fileName) const
{
    const QStringList searchDirs = {
        QCoreApplication::applicationDirPath() + "/data",
        QCoreApplication::applicationDirPath() + "/../app/data",
        QCoreApplication::applicationDirPath() + "/../../app/data",
        QCoreApplication::applicationDirPath() + "/../../../app/data",
        QDir::currentPath() + "/app/data",
        QDir::currentPath() + "/data"
    };

    for (const QString &dir : searchDirs) {
        const QString path = QDir(dir).filePath(fileName);

        if (QFile::exists(path)) {
            return path;
        }
    }

    return QString();
}

void MainWindow::redrawMap()
{
    ui->mapLabel->setPixmap(createMapPixmap(highlightedGpxIndex));
}

void MainWindow::invalidateMapLayer()
{
    mapLayerDirty = true;
}

float MainWindow::speedFactorFromSlider(int value) const
{
    if (value == 0) {
        return 1.0f;
    }

    const float magnitude = static_cast<float>(qPow(50.0, qAbs(value) / 100.0));
    return value > 0 ? magnitude : -magnitude;
}

void MainWindow::setSimulationRunning(bool running)
{
    simulationRunning = running;
    updateSimulationStatusDisplay();
}

void MainWindow::updateSpeedDisplay()
{
    ui->simulationSpeedValueLabel->setText(
        QString("Speed: %1x").arg(currentSpeedFactor, 0, 'f', 1)
        );
}

void MainWindow::updateSimulationStatusDisplay()
{
    QString statusText = "Simulation: not started";

    if (simulationStarted) {
        statusText = simulationRunning
                         ? "Simulation: running"
                         : "Simulation: paused";
    }

    ui->simulationStatusLabel->setText(statusText);
}

void MainWindow::updateSmartWatchRunnerListDisplay()
{
    const int runnerCount = currentWatchRunnerNames.size();
    const int currentRunnerIndex = laufkoordination != nullptr
        ? laufkoordination->getAktuellerLaeuferIndex()
        : -1;

    ui->watchRunnerNameLabel1->setText(currentWatchRunnerNames.value(0));
    ui->watchRunnerNameLabel2->setText(currentWatchRunnerNames.value(1));
    ui->watchRunnerNameLabel3->setText(currentWatchRunnerNames.value(2));
    ui->watchRunnerNameLabel4->setText(currentWatchRunnerNames.value(3));
    ui->watchRunnerNameLabel5->setText(currentWatchRunnerNames.value(4));

    ui->watchRunnerNameLabel1->setVisible(runnerCount > 0);
    ui->watchRunnerNameLabel2->setVisible(runnerCount > 1);
    ui->watchRunnerNameLabel3->setVisible(runnerCount > 2);
    ui->watchRunnerNameLabel4->setVisible(runnerCount > 3);
    ui->watchRunnerNameLabel5->setVisible(runnerCount > 4);

    ui->watchRunnerRemoveButton1->setVisible(runnerCount > 0);
    ui->watchRunnerRemoveButton2->setVisible(runnerCount > 1);
    ui->watchRunnerRemoveButton3->setVisible(runnerCount > 2);
    ui->watchRunnerRemoveButton4->setVisible(runnerCount > 3);
    ui->watchRunnerRemoveButton5->setVisible(runnerCount > 4);

    QLabel *runnerLabels[] = {
        ui->watchRunnerNameLabel1,
        ui->watchRunnerNameLabel2,
        ui->watchRunnerNameLabel3,
        ui->watchRunnerNameLabel4,
        ui->watchRunnerNameLabel5
    };

    for (int i = 0; i < 5; ++i) {
        runnerLabels[i]->setStyleSheet(
            i == currentRunnerIndex
                ? "color: #e8f600; font-weight: bold;"
                : "color: white;"
            );
    }

    const QString runnerNumberText =
        QString("%1").arg(currentRunnerNumber(), 2, 10, QChar('0'));

    ui->watchRunnerValueLabel->setText(runnerNumberText);
    ui->watchStatusRunnerValueLabel->setText(runnerNumberText);
}

void MainWindow::syncSmartWatchRunnersFromLaufkoordination()
{
    if (laufkoordination == nullptr) {
        currentWatchRunnerNames.clear();
    } else {
        currentWatchRunnerNames = laufkoordination->getLaeuferListe();
    }

    watchRunnerListInitialized = true;
    updateSmartWatchRunnerListDisplay();
    hardwareSteuerung->setDisplayRunnerWerte(currentWatchRunnerNames);
}

void MainWindow::addSmartWatchRunner()
{
    if (laufkoordination == nullptr) {
        return;
    }

    laufkoordination->addLaeufer(ui->runnerNameLineEdit->text());
    ui->runnerNameLineEdit->clear();
    syncSmartWatchRunnersFromLaufkoordination();
}

void MainWindow::switchToNextSmartWatchRunner()
{
    ++currentLap;

    if (laufkoordination == nullptr) {
        refreshSmartWatchSensorValues();
        return;
    }

    laufkoordination->naechsterLaeufer();
    syncSmartWatchRunnersFromLaufkoordination();
    refreshSmartWatchSensorValues();
}

void MainWindow::removeSmartWatchRunner(int runnerIndex)
{
    if (laufkoordination == nullptr ||
        runnerIndex < 0 ||
        runnerIndex >= currentWatchRunnerNames.size()) {
        return;
    }

    laufkoordination->removeLaeufer(runnerIndex);
    syncSmartWatchRunnersFromLaufkoordination();
}

void MainWindow::rebuildMapLayer()
{
    const int width = qMax(1, ui->mapLabel->width());
    const int height = qMax(1, ui->mapLabel->height());

    mapLayerPixmap = QPixmap(width, height);
    mapLayerPixmap.fill(Qt::transparent);
    mapLayerRect = QRect();

    QPainter painter(&mapLayerPixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    if (!mapPixmap.isNull()) {
        const QSize scaledSize =
            mapPixmap.size().scaled(mapLayerPixmap.size(), Qt::KeepAspectRatio);

        mapLayerRect = QRect((width - scaledSize.width()) / 2,
                             (height - scaledSize.height()) / 2,
                             scaledSize.width(),
                             scaledSize.height());

        painter.drawPixmap(mapLayerRect, mapPixmap);

        if (physicsEngine == nullptr || physicsEngine->getGpxPointCount() == 0) {
            mapLayerDirty = false;
            return;
        }

        const simulation::GpxPoint firstPoint = physicsEngine->getGpxPoint(0);

        mapMinLat = firstPoint.lat;
        mapMaxLat = firstPoint.lat;
        mapMinLon = firstPoint.lon;
        mapMaxLon = firstPoint.lon;

        for (int i = 1; i < physicsEngine->getGpxPointCount(); ++i) {
            const simulation::GpxPoint point = physicsEngine->getGpxPoint(i);

            mapMinLat = qMin(mapMinLat, point.lat);
            mapMaxLat = qMax(mapMaxLat, point.lat);
            mapMinLon = qMin(mapMinLon, point.lon);
            mapMaxLon = qMax(mapMaxLon, point.lon);
        }

        painter.setPen(QPen(QColor("#ff0000"), 2));

        for (int i = 1; i < physicsEngine->getGpxPointCount(); ++i) {
            painter.drawLine(mapPointForGpxPoint(physicsEngine->getGpxPoint(i - 1)),
                             mapPointForGpxPoint(physicsEngine->getGpxPoint(i)));
        }

        mapLayerDirty = false;
        return;
    }

    painter.setPen(QColor("#777777"));
    painter.drawText(mapLayerPixmap.rect(),
                     Qt::AlignCenter,
                     "Keine Karte gefunden");

    mapLayerDirty = false;
}

QPointF MainWindow::mapPointForGpxPoint(const simulation::GpxPoint &point) const
{
    constexpr double trackXScale = 0.45;
    constexpr double trackYScale = 0.895;
    constexpr double trackXOffset = -28.0;
    constexpr double trackYOffset = -2.0;

    const double lonRange = qMax(0.000001, mapMaxLon - mapMinLon);
    const double latRange = qMax(0.000001, mapMaxLat - mapMinLat);

    const double xRel = qBound(
        0.0,
        0.5 + (((point.lon - mapMinLon) / lonRange) - 0.5) * trackXScale,
        1.0
        );

    const double yRel = qBound(
        0.0,
        0.5 + (((mapMaxLat - point.lat) / latRange) - 0.5) * trackYScale,
        1.0
        );

    return QPointF(mapLayerRect.left() + xRel * mapLayerRect.width() + trackXOffset,
                   mapLayerRect.top() + yRel * mapLayerRect.height() + trackYOffset);
}

QPixmap MainWindow::createMapPixmap(int highlightIndex)
{
    const QSize labelSize(qMax(1, ui->mapLabel->width()),
                          qMax(1, ui->mapLabel->height()));
    constexpr qreal mapCornerRadius = 40.0;

    if (mapLayerDirty || mapLayerPixmap.size() != labelSize) {
        rebuildMapLayer();
    }

    QPixmap canvas = mapLayerPixmap;

    if (physicsEngine != nullptr &&
        physicsEngine->getGpxPointCount() > 0 &&
        !mapLayerRect.isEmpty()) {
        const int clampedIndex =
            qBound(0, highlightIndex, physicsEngine->getGpxPointCount() - 1);

        QPainter markerPainter(&canvas);
        markerPainter.setRenderHint(QPainter::Antialiasing);

        const QPointF marker =
            mapPointForGpxPoint(physicsEngine->getGpxPoint(clampedIndex));

        markerPainter.setBrush(QColor("#ffcc00"));
        markerPainter.setPen(QPen(Qt::white, 2));
        markerPainter.drawEllipse(marker, 9, 9);
    }

    QPixmap roundedCanvas(canvas.size());
    roundedCanvas.fill(Qt::transparent);

    QPainter roundedPainter(&roundedCanvas);
    roundedPainter.setRenderHint(QPainter::Antialiasing);

    const QRect roundedRect = mapLayerRect.isEmpty()
        ? canvas.rect()
        : mapLayerRect;

    QPainterPath clipPath;
    clipPath.addRoundedRect(QRectF(roundedRect),
                            mapCornerRadius,
                            mapCornerRadius);

    roundedPainter.setClipPath(clipPath);
    roundedPainter.drawPixmap(0, 0, canvas);

    return roundedCanvas;
}

void MainWindow::updateWlanComboBox(const QStringList& netze)
{
    ui->wlanComboBox->clear();

    for (const auto& name : netze)
    {
        ui->wlanComboBox->addItem(name);
    }

    ui->wlanComboBox->setCurrentIndex(-1);
}

void MainWindow::updateBluetoothComboBox(const QStringList& netze)
{
    ui->bluetoothComboBox->clear();

    for (const auto& name : netze)
    {
        ui->bluetoothComboBox->addItem(name);
    }

    ui->bluetoothComboBox->setCurrentIndex(-1);
}

void MainWindow::updateAntPlusComboBox(const QStringList& netze)
{
    ui->antPlusComboBox->clear();

    for (const auto& name : netze)
    {
        ui->antPlusComboBox->addItem(name);
    }

    ui->antPlusComboBox->setCurrentIndex(-1);
}

void MainWindow::setDisconnected()
{
    setConnectionLabelState(ui->wlanStatusLabel, false);
    setConnectionLabelState(ui->bluetoothStatusLabel, false);
    setConnectionLabelState(ui->antPlusStatusLabel, false);
}

void MainWindow::setWlanConnectionActive(bool active)
{
    setConnectionLabelState(ui->wlanStatusLabel, active);
}

void MainWindow::setBluetoothConnectionActive(bool active)
{
    setConnectionLabelState(ui->bluetoothStatusLabel, active);
}

void MainWindow::setAntPlusConnectionActive(bool active)
{
    setConnectionLabelState(ui->antPlusStatusLabel, active);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        if (obj == ui->wlanComboBox)
            {
                emit wlanComboBoxClicked();
            }
            else if (obj == ui->bluetoothComboBox)
            {
                emit bluetoothComboBoxClicked();
            }
            else if (obj == ui->antPlusComboBox)
            {
                emit antPlusComboBoxClicked();
            }
    }

    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::onWlanButtonClicked()
{
    if (ui->wlanComboBox->currentIndex() < 0 ||
        ui->wlanComboBox->currentText().isEmpty()) {
        setWlanConnectionActive(false);
        return;
    }

    QString ssid = ui->wlanComboBox->currentText();
    QString password = ui->lineEditPassword->text();

    emit connectWlanRequest(ssid, password);
}

void MainWindow::onBluetoothButtonClicked()
{
    if (ui->bluetoothComboBox->currentIndex() < 0 ||
        ui->bluetoothComboBox->currentText().isEmpty()) {
        setBluetoothConnectionActive(false);
        return;
    }

    QString name = ui->bluetoothComboBox->currentText();

    emit connectBluetoothRequest(name);
}

void MainWindow::onAntPlusButtonClicked()
{
    if (ui->antPlusComboBox->currentIndex() < 0 ||
        ui->antPlusComboBox->currentText().isEmpty()) {
        setAntPlusConnectionActive(false);
        return;
    }

    QString id = ui->antPlusComboBox->currentText();

    emit connectAntPlusRequest(id);
}


void MainWindow::setConnectionLabelState(QLabel *label, bool active)
{
    if (label == nullptr) {
        return;
    }

    const QString backgroundColor = active ? "#39ff6a" : "#555555";

    label->setAlignment(Qt::AlignCenter);
    label->setMinimumSize(24, 24);

    label->setStyleSheet(
        QString(
            "QLabel {"
            "color: white;"
            "background-color: %1;"
            "border-radius: 12px;"
            "font-weight: bold;"
            "padding: 4px;"
            "}"
            ).arg(backgroundColor)
        );
}
