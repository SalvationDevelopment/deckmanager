#include "cardslist.h"
#include "limitcards.h"
#include "draghelper.h"
#include "range.h"
#include <QDebug>

CardsList::CardsList(QWidget *parent)
    : QWidget(parent), pos(0), cardSize(177 / 3.5, 254 / 3.5), cardsPerColumn(0),
      sb(nullptr), needRefreshId(false), current(-1)
{
    setMouseTracking(true);
    setAcceptDrops(true);
    setMinimumWidth(cardSize.width() + fontMetrics().width(tr("宽")) * 10);
    auto family = font().family();
    setFont(QFont(family, 11));
}

void CardsList::wheelEvent(QWheelEvent *event)
{
    int numDegrees = event->delta() / 8;
    int numSteps = numDegrees / 15;

    needRefreshId = true;
    point = event->pos();

    setPos(pos - numSteps);
    event->accept();
}

void CardsList::refresh()
{
    int max = ls.size() - cardsPerColumn;
    if(sb)
    {
        sb->setMaximum(max > 0 ? max : 0);
    }
}

void CardsList::setScrollBar(QScrollBar *_sb)
{
    sb = _sb;
    sb->setMaximum(ls.size());
    connect(sb, &QScrollBar::valueChanged, this, &CardsList::setPos);
}

QString CardsList::adToString(int ad)
{
    if(ad == -2)
    {
        return "?";
    }
    else
    {
        return std::move(QString::number(ad));
    }
}

void CardsList::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    if(!ls.empty())
    {
        int fmHeight = painter.fontMetrics().height();
        int h = height(), cardHeight = cardSize.height(),
                cardWidth = cardSize.width();

        cardsPerColumn = h / cardHeight;

        decltype(items) newItems;

        double varHeight = h - cardHeight * 1.0;

        for(int i : range(std::min(cardsPerColumn, ls.size() - pos)))
        {
            quint32 id = ls[i + pos];

            auto it = newItems.insert(i + pos, CardItem(id));

            auto &item = it.value();

            int y = varHeight * i / (cardsPerColumn - 1);


            current = itemAt(mapFromGlobal(QCursor::pos()));
            if(i + pos == current)
            {
                QBrush brush = painter.brush(), newBrush(Qt::lightGray);
                QPen pen = painter.pen();
                painter.setPen(Qt::transparent);
                QColor color(newBrush.color());
                color.setAlpha(160);
                newBrush.setColor(color);
                painter.setBrush(newBrush);
                painter.drawRect(QRect(QPoint(0,  y), QSize(width(), cardSize.height())));
                painter.setBrush(brush);
                painter.setPen(pen);
            }

            item.setPos(QPoint(0, y));

            painter.drawPixmap(0, y, cardWidth, cardHeight,
                               *item.getPixmap().data());

            int lim = limitCards->getLimit(it->getId());
            if(lim < 3)
            {
                auto data = limitCards->getPixmap(lim);
                if(data)
                {
                    painter.drawPixmap(0, y, 16, 16, *data.data());
                }
            }

            auto card = cardPool->getCard(id);

            if(!card)
            {
                continue;
            }

            painter.drawText(cardWidth + 5, y + fmHeight, card->name);
            QString ot;
            QString level = (card->type & Const::TYPE_XYZ) ? tr("R") : tr("L");
            level = "[" + level + QString::number(card->level) + "]";
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
                painter.drawText(cardWidth + 5, y + 5 + fmHeight * 2,
                                 card->cardRace() + tr("/") + card->cardAttr() + level);

                painter.drawText(cardWidth + 5, y + 10 + fmHeight * 3,
                                 adToString(card->atk) + tr("/") +
                                 adToString(card->def) + ot);
            }
            else if(card->type & (Const::TYPE_SPELL | Const::TYPE_TRAP))
            {
                painter.drawText(cardWidth + 5, y + 5 + fmHeight * 2,
                                 card->cardType());
                painter.drawText(cardWidth + 5, y + 10 + fmHeight * 3, ot);
            }
        }
        items.swap(newItems);
    }
    if(needRefreshId)
    {
        refreshCurrentId();
        needRefreshId = false;
    }
    refresh();
}

void CardsList::mousePressEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        startPos = event->pos();
    }
    else if(event->buttons() & Qt::MiddleButton)
    {
        int index = itemAt(event->pos());
        if(index >= 0)
        {
            emit details(ls[index]);
        }
    }
    QWidget::mousePressEvent(event);
}

int CardsList::itemAt(const QPoint &_pos)
{
    for(int i: range(cardsPerColumn))
    {
        if(i + pos >= ls.size())
        {
            break;
        }
        int index = i +  pos;

        auto &item = items.find(index).value();

        if(_pos.y() >= item.getPos().y() &&
                _pos.y() <= item.getPos().y() + cardSize.height() &&
                _pos.x() >= 0 && _pos.x() < width())
        {
            return index;
        }
    }
    return -1;
}

void CardsList::refreshCurrentId()
{
    int index = itemAt(point);
    if(index != -1)
    {
        quint32 id = ls[index];
        if(currentCardId != id)
        {
            currentCardId = id;
            emit currentIdChanged(ls[index]);
        }
    }
}

void CardsList::mouseMoveEvent(QMouseEvent *event)
{
    int index = itemAt(event->pos());
    if(index != -1)
    {
        quint32 id = ls[index];
        if(currentCardId != id)
        {
            currentCardId = id;
            emit currentIdChanged(ls[index]);
        }
        needRefreshId = false;
    }

    if(event->buttons() & Qt::LeftButton)
    {
        int dist = (event->pos() - startPos).manhattanLength();
        if(dist >= QApplication::startDragDistance() && index != -1)
        {
            current = -1;
            startDrag(index);
        }
    }
    else if(current != index)
    {
        current = index;
        update();
    }
    QWidget::mouseMoveEvent(event);
}

void CardsList::startDrag(int index)
{
    if(index >= ls.size())
    {
        return;
    }
    QMimeData *mimedata = new QMimeData;
    quint32 id = ls[index];
    mimedata->setText(QString::number(id));
    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimedata);
    auto &item = items.find(index).value();
    drag->setPixmap(item.getPixmap()->scaled(cardSize));
    drag->setHotSpot(QPoint(drag->pixmap().width() / 2,
                            drag->pixmap().height() / 2));
    dragHelper.moved = false;
    drag->exec(Qt::MoveAction);
}

void CardsList::dragEnterEvent(QDragEnterEvent *event)
{
    QObject *src = event->source();

    if(src)
    {
        event->accept();
    }
}

void CardsList::dragMoveEvent(QDragMoveEvent *event)
{
    QObject *src = event->source();
    if(src)
    {
        event->accept();
    }
}

void CardsList::dropEvent(QDropEvent *event)
{
    dragHelper.moved = true;
    event->accept();
}

void CardsList::setPos(int _pos)
{
    int max = ls.size() - cardsPerColumn;
    max = max > 0 ? max : 0;
    if(pos > max)
    {
        pos = max;
    }
    if(_pos >= 0 && _pos <= max)
    {
        pos = _pos;
        update();
    }
    if(sb)
    {
        sb->setValue(pos);
    }
}

void CardsList::setCards(Type::DeckP cards)
{
    ls.swap(*cards.data());
    ls.squeeze();

    setPos(0);

    emit sizeChanged(ls.size());
    refresh();
}

void CardsList::checkLeave()
{
    int i = itemAt(mapFromGlobal(QCursor::pos()));
    if(i != current)
    {
        current = i;
        update();
    }
}

CardsList::~CardsList()
{

}

CardsListView::CardsListView(QWidget *parent)
    : QWidget(parent)
{
    auto hbox = new QHBoxLayout;
    auto vbox = new QVBoxLayout;

    cl = new CardsList(nullptr);
    auto sb = new QScrollBar;

    auto label = new DeckSizeLabel(config->getStr("label", "number", "数目"));
    label->setAlignment(Qt::AlignRight);

    cl->setScrollBar(sb);

    connect(cl, &CardsList::currentIdChanged, this, &CardsListView::changeId);
    connect(cl, &CardsList::sizeChanged, label, &DeckSizeLabel::changeSize);
    connect(cl, &CardsList::clickId, this, &CardsListView::clickId);
    connect(cl, &CardsList::details, this, &CardsListView::details);

    hbox->addWidget(cl);
    hbox->addWidget(sb);
    vbox->addWidget(label);
    vbox->addLayout(hbox, 1);
    setLayout(vbox);
}

