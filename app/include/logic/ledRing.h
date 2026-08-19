#pragma once

#include <QObject>
#include <QString>

class LEDRing : public QObject
{
    Q_OBJECT

private:
    bool status;
    QString color;

public:
    explicit LEDRing(QObject *parent = nullptr);

    bool getStatus() const;
    QString getColor() const;

public slots:
    void setStatus();
    void setColor(const QString &colorcode);

signals:
    void statusGeaendert(bool status);
    void colorGeaendert(const QString &color);
};