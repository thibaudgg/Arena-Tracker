#include "enemyhandhandler.h"
#include <QtWidgets>

EnemyHandHandler::EnemyHandHandler(QObject *parent, Ui::MainWindow *ui) : QObject(parent)
{
    this->ui = ui;
    this->inGame = false;
    this->transparency = Never;

    completeUI();
}


EnemyHandHandler::~EnemyHandHandler()
{
    ui->enemyHandListWidget->clear();
    enemyHandList.clear();
}


void EnemyHandHandler::reset()
{
    ui->enemyHandListWidget->clear();
    enemyHandList.clear();
}


void EnemyHandHandler::completeUI()
{
    ui->enemyHandListWidget->setIconSize(CARD_SIZE);
    ui->enemyHandListWidget->setMinimumHeight(0);
}


void EnemyHandHandler::showEnemyCardDraw(int id, int turn, bool special, QString code)
{
    HandCard handCard(code);
    handCard.id = id;
    handCard.turn = turn;
    handCard.special = special;
    handCard.listItem = new QListWidgetItem();
    ui->enemyHandListWidget->addItem(handCard.listItem);

    handCard.draw();
    enemyHandList.append(handCard);

    if(code != "")      emit checkCardImage(code);
}


void EnemyHandHandler::lastHandCardIsCoin()
{
    if(enemyHandList.empty())   return;//En modo practica el mulligan enemigo termina antes de robar las cartas
    enemyHandList.last().code = COIN;
    enemyHandList.last().cost = 0;
    enemyHandList.last().draw();

    emit checkCardImage(COIN);
}


void EnemyHandHandler::showEnemyCardPlayed(int id, QString code)
{
    (void) code;

    int i=0;
    for (QList<HandCard>::iterator it = enemyHandList.begin(); it != enemyHandList.end(); it++, i++)
    {
        if(it->id == id)
        {
            delete it->listItem;
            enemyHandList.removeAt(i);
            return;
        }
    }
}


void EnemyHandHandler::redrawDownloadedCardImage(QString code)
{
    for (QList<HandCard>::iterator it = enemyHandList.begin(); it != enemyHandList.end(); it++)
    {
        if(it->code == code)    it->draw();
    }
}


void EnemyHandHandler::lockEnemyInterface()
{
    this->inGame = true;
    updateTransparency();

    reset();
}


void EnemyHandHandler::unlockEnemyInterface()
{
    this->inGame = false;
    updateTransparency();
}


void EnemyHandHandler::updateTransparency()
{
    if(transparency==Always || (inGame && transparency==Auto))
    {
        ui->enemyHandListWidget->setStyleSheet("background-color: transparent;");
        ui->tabEnemy->setAttribute(Qt::WA_NoBackground);
        ui->tabEnemy->repaint();
    }
    else
    {
        ui->enemyHandListWidget->setStyleSheet("");
        ui->tabEnemy->setAttribute(Qt::WA_NoBackground, false);
        ui->tabEnemy->repaint();
    }
}


void EnemyHandHandler::setTransparency(Transparency value)
{
    this->transparency = value;
    updateTransparency();
}

