#ifndef DECKHANDLER_H
#define DECKHANDLER_H

#include "deckcard.h"
#include "ui_mainwindow.h"
#include <QObject>
#include <QMap>

#define MALORNE QString("GVG_035")


enum TabEnum { tabArena, tabDeck, tabEnemy, tabLog };

class DeckHandler : public QObject
{
    Q_OBJECT
public:
    DeckHandler(QObject *parent, QMap<QString, QJsonObject> *cardsJson, Ui::MainWindow *ui);
    ~DeckHandler();

//Variables
private:
    QList<DeckCard> deckCardList;
    int remainingCards;
    Ui::MainWindow *ui;
    QMap<QString, QJsonObject> *cardsJson;
    bool inGame;


//Metodos
private:
    void insertDeckCard(DeckCard &deckCard);

public:
    void reset();
    void redrawDownloadedCardImage(QString code);
    QList<DeckCard> * getDeckComplete();
    void showCount();

signals:
    void checkCardImage(QString code);
    void sendLog(QString line);
    void sendStatusBarMessage(QString message, int timeout=0);

public slots:
    void newDeckCard(QString card, int total=1);
    void showPlayerCardDraw(QString code);
    void enableDeckButtons();
    void cardTotalMin();
    void cardTotalPlus();
    void lockDeckInterface();
    void unlockDeckInterface();
};

#endif // DECKHANDLER_H