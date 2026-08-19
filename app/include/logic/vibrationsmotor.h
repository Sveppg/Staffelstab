#pragma once

#include <QObject>

class Vibrationsmotor : public QObject
{
    Q_OBJECT

private:
    int rotationsspeed = 0;

public:
    explicit Vibrationsmotor(QObject *parent = nullptr);

    int getRotationsspeed() const;

public slots:
    void setRotationsspeed(int speed);

signals:
    void rotationsspeedGeaendert(int speed);
};