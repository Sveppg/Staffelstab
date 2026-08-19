#pragma once
#include <QObject>

class Herzfrequenzsensor : public QObject
{
    Q_OBJECT

private:
    int Herzfrequenz;

public:
    explicit Herzfrequenzsensor(QObject* parent = nullptr);

    int getHerzfrequenz() const;
    void setHerzfrequenz(int herzfrequenz);

signals:
    void herzfrequenzGeandert();
};