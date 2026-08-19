#pragma once

#include <QObject>

class MenueButton : public QObject
{
    Q_OBJECT

private:
    bool clicked = false;
    int klickAnzahl = 0;

public:
    explicit MenueButton(QObject *parent = nullptr);

    bool getClicked() const;
    int getKlickAnzahl() const;

public slots:
    void setClicked();

signals:
    void clickedZustand(bool clicked);
    void menueButtonGeklickt();
    void klickAnzahlGeaendert(int klickAnzahl);
};