#pragma once


#include <QObject>

class Ladehub : public QObject
{
    Q_OBJECT

private:
    bool ladestatus;
    int batterieProzent;

public:
    explicit Ladehub(QObject *parent = nullptr);
    int getBatterieProzent() const;
    bool getLadeStatus() const;

public slots:
    void setBatterieProzent(int prozent);
    void setLadestatus(bool aktiv);
    void ladeProzent(int prozent = 1);
    void entladeProzent(int prozent = 1);


signals:
    void batterieProzentGeandert(int prozent);
    void ladeStatusGeandert(bool aktiv);
};
