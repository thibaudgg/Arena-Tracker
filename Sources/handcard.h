#ifndef HANDCARD_H
#define HANDCARD_H

#include "deckcard.h"


class HandCard : public DeckCard
{
public:
    HandCard(QString code);
    ~HandCard();

//Variables
public:
    int id;
    int turn;
    bool special;

//Metodos
private:

public:
    void draw() Q_DECL_OVERRIDE;
};
#endif // HANDCARD_H
