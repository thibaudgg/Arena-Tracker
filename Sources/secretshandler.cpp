#include "secretshandler.h"
#include <QtWidgets>

SecretsHandler::SecretsHandler(QObject *parent, Ui::MainWindow *ui) : QObject(parent)
{
    this->ui = ui;
    this->synchronized = false;
    this->secretsAnimating = false;

    completeUI();
}

SecretsHandler::~SecretsHandler()
{

}


void SecretsHandler::completeUI()
{
    ui->secretsTreeWidget->setHidden(true);

    ui->secretsTreeWidget->setColumnCount(1);
    ui->secretsTreeWidget->header()->close();
    ui->secretsTreeWidget->setIconSize(CARD_SIZE);
    ui->secretsTreeWidget->setStyleSheet("background-color: transparent;");
}


void SecretsHandler::setSynchronized()
{
    this->synchronized = true;
}


void SecretsHandler::adjustSize()
{
    if(secretsAnimating)
    {
        QTimer::singleShot(ANIMATION_TIME+50, this, SLOT(adjustSize()));
        return;
    }

    int rowHeight = ui->secretsTreeWidget->sizeHintForRow(0);
    int rows = 0;

    for(int i=0; i<activeSecretList.count(); i++)
    {
        rows += activeSecretList[i].children.count() + 1;
    }

    int height = rows*rowHeight + 2*ui->secretsTreeWidget->frameWidth();
    int maxHeight = (ui->secretsTreeWidget->height()+ui->enemyHandListWidget->height())*4/5;
    if(height>maxHeight)    height = maxHeight;

    QPropertyAnimation *animation = new QPropertyAnimation(ui->secretsTreeWidget, "minimumHeight");
    animation->setDuration(ANIMATION_TIME);
    animation->setStartValue(ui->secretsTreeWidget->minimumHeight());
    animation->setEndValue(height);
    animation->setEasingCurve(QEasingCurve::OutBounce);
    animation->start();

    QPropertyAnimation *animation2 = new QPropertyAnimation(ui->secretsTreeWidget, "maximumHeight");
    animation2->setDuration(ANIMATION_TIME);
    animation2->setStartValue(ui->secretsTreeWidget->maximumHeight());
    animation2->setEndValue(height);
    animation2->setEasingCurve(QEasingCurve::OutBounce);
    animation2->start();

    this->secretsAnimating = true;
    connect(animation, SIGNAL(finished()),
            this, SLOT(clearSecretsAnimating()));
}


void SecretsHandler::clearSecretsAnimating()
{
    this->secretsAnimating = false;
    if(activeSecretList.empty())    ui->secretsTreeWidget->setHidden(true);
}


void SecretsHandler::secretStealed(int id, QString code)
{
    ActiveSecret activeSecret;
    activeSecret.id = id;
    activeSecret.root.hero = unknown;

    activeSecret.root.treeItem = new QTreeWidgetItem(ui->secretsTreeWidget);
    activeSecret.root.treeItem->setExpanded(true);
    activeSecret.root.code = code;
    activeSecret.root.draw();
    emit checkCardImage(code);

    activeSecretList.append(activeSecret);

    ui->secretsTreeWidget->setHidden(false);
    if(synchronized) ui->tabWidget->setCurrentWidget(ui->tabEnemy);

    adjustSize();
}


void SecretsHandler::secretPlayed(int id, SecretHero hero)
{
    ActiveSecret activeSecret;
    activeSecret.id = id;
    activeSecret.root.hero = hero;

    activeSecret.root.treeItem = new QTreeWidgetItem(ui->secretsTreeWidget);
    activeSecret.root.treeItem->setExpanded(true);
    activeSecret.root.draw();

    switch(hero)
    {
        case paladin:
            activeSecret.children.append(SecretCard(AVENGE));
            activeSecret.children.append(SecretCard(NOBLE_SACRIFICE));
            activeSecret.children.append(SecretCard(REPENTANCE));
            activeSecret.children.append(SecretCard(REDEMPTION));
            activeSecret.children.append(SecretCard(EYE_FOR_AN_EYE));
            activeSecret.children.append(SecretCard(COMPETITIVE_SPIRIT));
        break;

        case hunter:
            activeSecret.children.append(SecretCard(FREEZING_TRAP));
            activeSecret.children.append(SecretCard(EXPLOSIVE_TRAP));
            activeSecret.children.append(SecretCard(BEAR_TRAP));
            activeSecret.children.append(SecretCard(SNIPE));
            activeSecret.children.append(SecretCard(MISDIRECTION));
            activeSecret.children.append(SecretCard(SNAKE_TRAP));
        break;

        case mage:
            activeSecret.children.append(SecretCard(MIRROR_ENTITY));
            activeSecret.children.append(SecretCard(DDUPLICATE));
            activeSecret.children.append(SecretCard(ICE_BARRIER));
            activeSecret.children.append(SecretCard(EFFIGY));
            activeSecret.children.append(SecretCard(VAPORIZE));
            activeSecret.children.append(SecretCard(COUNTERSPELL));
            activeSecret.children.append(SecretCard(SPELLBENDER));
            activeSecret.children.append(SecretCard(ICE_BLOCK));
        break;

        case unknown:
        break;
    }

    for(QList<SecretCard>::iterator it = activeSecret.children.begin(); it != activeSecret.children.end(); it++)
    {
        it->treeItem = new QTreeWidgetItem(activeSecret.root.treeItem);
        it->draw();
        emit checkCardImage(it->code);
    }

    activeSecretList.append(activeSecret);

    ui->secretsTreeWidget->setHidden(false);
    if(synchronized) ui->tabWidget->setCurrentWidget(ui->tabEnemy);

    adjustSize();

    emit pDebug("Secret played. Hero: " + QString::number(hero));
}


void SecretsHandler::redrawDownloadedCardImage(QString code)
{
    for(QList<ActiveSecret>::iterator it = activeSecretList.begin(); it != activeSecretList.end(); it++)
    {
        if(it->root.code == code)    it->root.draw();
        for(QList<SecretCard>::iterator it2 = it->children.begin(); it2 != it->children.end(); it2++)
        {
            if(it2->code == code)    it2->draw();
        }
    }
}


void SecretsHandler::resetSecretsInterface()
{
    ui->secretsTreeWidget->setHidden(true);
    ui->secretsTreeWidget->clear();
    activeSecretList.clear();
    secretTests.clear();
}


void SecretsHandler::secretRevealed(int id, QString code)
{
    for(int i=0; i<activeSecretList.count(); i++)
    {
        if(activeSecretList[i].id == id)
        {
            ui->secretsTreeWidget->takeTopLevelItem(i);
            delete activeSecretList[i].root.treeItem;
            activeSecretList.removeAt(i);
            break;
        }
    }

    for(int i=0; i<secretTests.count(); i++)
    {
        secretTests[i].secretRevealedLastSecond = true;
    }
    adjustSize();

    //No puede haber dos secretos iguales
    discardSecretOptionNow(code);

    emit pDebug("Secret revealed: " + code);
}


void SecretsHandler::discardSecretOptionDelay()
{
    if(secretTests.isEmpty())   return;

    SecretTest secretTest = secretTests.dequeue();
    if(secretTest.secretRevealedLastSecond)
    {
        emit pDebug("Option not discarded: " + secretTest.code + " (A secret revealed)");
        return;
    }

    discardSecretOptionNow(secretTest.code);
}


void SecretsHandler::discardSecretOptionNow(QString code)
{
    for(QList<ActiveSecret>::iterator it = activeSecretList.begin(); it != activeSecretList.end(); it++)
    {
        for(int i=0; i<it->children.count(); i++)
        {
            if(it->children[i].code == code)
            {
                emit pDebug("Option discarded: " + code);
                it->root.treeItem->removeChild(it->children[i].treeItem);
                it->children.removeAt(i);
                QTimer::singleShot(10, this, SLOT(adjustSize()));

                //Comprobar unica posibilidad
                checkLastSecretOption(*it);
                break;
            }
        }
    }
}


void SecretsHandler::discardSecretOption(QString code, int delay)
{
    if(activeSecretList.isEmpty())  return;

    SecretTest secretTest;
    secretTest.code = code;
    secretTest.secretRevealedLastSecond = false;
    secretTests.enqueue(secretTest);

    QTimer::singleShot(delay, this, SLOT(discardSecretOptionDelay()));
}


void SecretsHandler::checkLastSecretOption(ActiveSecret activeSecret)
{
    if(activeSecret.children.count() == 1)
    {
        activeSecret.root.code = activeSecret.children.first().code;
        activeSecret.root.draw();
        activeSecret.root.treeItem->removeChild(activeSecret.children.first().treeItem);
        activeSecret.children.clear();

        //No puede haber dos secretos iguales
        discardSecretOptionNow(activeSecret.root.code);
    }
}


void SecretsHandler::playerSpellPlayed()
{
    discardSecretOptionNow(COUNTERSPELL);
}


/*
 * COUNTERSPELL no crea la primera linea en el log al desvelarse, solo la segunda que aparece cuando la animacion se completa
 * Eso hace que el caso de comprobacion de SPELLBENDER tenga que ser de mas delay.
 * No queremos que SPELLBENDER se descarte de un segundo secreto cuando al lanzar un hechizo el primer secreto
 * desvela COUNTERSPELL
 */

void SecretsHandler::playerSpellObjPlayed()
{
    discardSecretOption(SPELLBENDER, 7000);//Ocultado por COUNTERSPELL
}


void SecretsHandler::playerMinionPlayed()
{
    discardSecretOptionNow(MIRROR_ENTITY);
    discardSecretOptionNow(SNIPE);
    discardSecretOptionNow(REPENTANCE);
}


void SecretsHandler::enemyMinionDead()
{
    discardSecretOptionNow(DDUPLICATE);
    discardSecretOptionNow(EFFIGY);
    discardSecretOptionNow(REDEMPTION);
}


void SecretsHandler::avengeTested()
{
    discardSecretOptionNow(AVENGE);
}


void SecretsHandler::cSpiritTested()
{
    discardSecretOptionNow(COMPETITIVE_SPIRIT);
}


/*
 * http://hearthstone.gamepedia.com/Secret
 *
 * If a Secret removes the specific target for another Secret which was already triggered, the second Secret will not take effect,
 * since it now lacks a target. For example, if Freezing Trap removes the minion which would have been the target of Misdirection,
 * the Misdirection will not trigger, since it no longer has a target.
 *
 * Note that this rule only applies for Secrets which require specific targets; Secrets such as Explosive Trap and Snake Trap do not require targets,
 * and will always take effect once triggered, even if the original trigger minion has been removed from play.
 */
void SecretsHandler::playerAttack(bool isHeroFrom, bool isHeroTo)
{
    if(isHeroFrom)
    {
        //Hero -> hero
        if(isHeroTo)
        {
            discardSecretOptionNow(ICE_BARRIER);
            discardSecretOptionNow(EXPLOSIVE_TRAP);
            discardSecretOptionNow(BEAR_TRAP);
            discardSecretOption(MISDIRECTION);//Ocultado por EXPLOSIVE_TRAP
            discardSecretOption(EYE_FOR_AN_EYE);//Ocultado por NOBLE_SACRIFICE
            discardSecretOptionNow(NOBLE_SACRIFICE);
        }
        //Hero -> minion
        else
        {
            discardSecretOptionNow(SNAKE_TRAP);
            discardSecretOptionNow(NOBLE_SACRIFICE);
        }
    }
    else
    {
        //Minion -> hero
        if(isHeroTo)
        {
            discardSecretOptionNow(VAPORIZE);
            discardSecretOptionNow(ICE_BARRIER);
            discardSecretOptionNow(EXPLOSIVE_TRAP);
            discardSecretOptionNow(BEAR_TRAP);
            discardSecretOption(FREEZING_TRAP);//Ocultado por EXPLOSIVE_TRAP
            discardSecretOption(MISDIRECTION);//Ocultado por FREEZING_TRAP y EXPLOSIVE_TRAP
            discardSecretOption(EYE_FOR_AN_EYE);//Ocultado por NOBLE_SACRIFICE
            discardSecretOptionNow(NOBLE_SACRIFICE);
        }
        //Minion -> minion
        else
        {
            discardSecretOptionNow(FREEZING_TRAP);
            discardSecretOptionNow(SNAKE_TRAP);
            discardSecretOptionNow(NOBLE_SACRIFICE);
        }
    }
}

















