#include "pref.h"
#include "limitcards.h"
#include "config.h"
#include "iconbutton.h"
#include <QCheckBox>

Pref::Pref(QWidget *parent) : QWidget(parent)
{
    auto vbox = new QVBoxLayout;
    auto lfbox = new QHBoxLayout;
    auto lf = new QLabel(config->getStr("label", "limit", "禁卡表"));
    auto getButton = new IconButton(":/icons/right.png");
    lfcombo = new QComboBox();

    auto tables = LimitCards::getTables();

    {
        auto it = tables.get().begin();
        int index = 0;
        for(; it != tables.get().end(); ++it, ++index)
        {
            lfcombo->addItem(it->first, index);
        }
        lfcombo->addItem(config->getStr("label", "noupperbound", "无上限"), -1);
    }
    connect(lfcombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setLflist(int)));
    connect(getButton, SIGNAL(clicked()), this, SLOT(openLfList()));

    lfbox->addWidget(lf);
    lfbox->addWidget(lfcombo);
    lfbox->addWidget(getButton);

    vbox->addLayout(lfbox);

    auto waitC = new QCheckBox(config->getStr("label", "passwait", "卡密缺失等待"));

    auto convertC = new QCheckBox(config->getStr("label", "passconvert", "先行/正式卡密转换"));
    waitC->setChecked(config->waitForPass);
    convertC->setChecked(config->convertPass);
    lfcombo->setCurrentIndex(lfcombo->count() >= config->limit ? config->limit : 0);

    connect(waitC, SIGNAL(toggled(bool)), config, SLOT(setWaitForPass(bool)));
    connect(convertC, SIGNAL(toggled(bool)), config, SLOT(setConvertPass(bool)));

    vbox->addWidget(waitC);
    vbox->addWidget(convertC);
    setLayout(vbox);
}


void Pref::setLflist(int index)
{
    config->setLimit(index);
    emit lflistChanged();
}

void Pref::openLfList()
{
    emit lfList(LimitCards::getCards(lfcombo->currentIndex()));
}