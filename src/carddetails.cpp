#include "carddetails.h"
#include "config.h"
#include <QVBoxLayout>

CardDetails::CardDetails(QWidget *parent)
    : QWidget(parent), offset(3), currentId(0)
{
    cp = new CardPicture;
    effect = new QPlainTextEdit;
    effect->setReadOnly(true);

    vbox = new QVBoxLayout;
    vbox->addWidget(cp);
    vbox->addWidget(effect, 1);
    setLayout(vbox);
}

void CardDetails::mouseDoubleClickEvent(QMouseEvent *event)
{
    emit clickId(currentId);
    QWidget::mouseDoubleClickEvent(event);
}

void CardDetails::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    setId(currentId);
}

void CardDetails::setId(quint32 id)
{
    QStringList str;
    QSharedPointer<Card> card = cardPool->getCard(id);
    if(!card)
    {
        return;
    }
    str << card->name + "[" + QString::number(id) + "]";
    currentId = id;

    QString ot;
    if((card->ot & 0x3) == 1)
    {
        ot = tr("[OCG]");
    }
    else if((card->ot & 0x3) == 2)
    {
        ot = tr("[TCG]");
    }

    if(card->type & Const::TYPE_MONSTER)
    {
        str << ("[" + card->cardType() + "]"
                      + card->cardRace() + "/" + card->cardAttr());
        QString level = (card->type & Const::TYPE_XYZ) ? tr("R") : tr("L");
        level = "[" + level + QString::number(card->level) + "]";
        str << (level + card->cardAD() + ot);
    }
    else
    {
        str << ("[" + card->cardType() + "]");
        str << (ot);
    }

    quint64 setcode = card->setcode;
    QStringList setcodeStr;
    while(setcode)
    {
        QString cur;
        cur.setNum(setcode & 0xffff, 16);
        setcodeStr << cur;
        setcode = setcode >> 16;
    }
    str << (config->getStr("label", "setcode", "系列") + ":[" + setcodeStr.join(tr("|")) + "]");
    str << card->effect;
    effect->clear();
    effect->insertPlainText(str.join('\n'));
    cp->setId(id, effect->width());
    vbox->removeWidget(effect);
    vbox->addWidget(effect, 1);
    updateGeometry();
}

CardDetails::~CardDetails()
{

}

