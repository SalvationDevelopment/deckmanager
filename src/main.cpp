#include "mainwindow.h"
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include "yrp.h"
#include "expansions.h"
#include "limitcards.h"
#include "config.h"
#include "card.h"
#include "carditem.h"


int main(int argc, char *argv[])
{
    int ret = 0;
    QApplication a(argc, argv);
    try
    {
        Config configR;
        config = &configR;

        QDir base("."), ext("expansions");
        QStringList paths, filters;
        filters << "*.cdb";
        foreach(auto info, ext.entryInfoList(filters))
        {
            paths << info.filePath();
        }
        foreach(auto info, base.entryInfoList(filters))
        {
            paths << info.filePath();
        }

        QStringList zipFilters, zipPaths;
        zipFilters << "*.zip";
        foreach(auto info, ext.entryInfoList(zipFilters))
        {
            zipPaths << info.filePath();
        }

        Expansions expansionsR(zipPaths);
        expansions = &expansionsR;

        CardPool cardPoolR(paths);
        cardPool = &cardPoolR;

        LimitCards limitCardsR;
        limitCards = &limitCardsR;

        MainWindow w;
        w.show();
        cardPool->loadNames();
        ret =  a.exec();
    }
    catch(...)
    {

    }
    return ret;
}
