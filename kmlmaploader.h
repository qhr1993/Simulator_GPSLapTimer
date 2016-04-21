#ifndef KMLMAPLOADER_H
#define KMLMAPLOADER_H
#include <QXmlSimpleReader>
#include <QtCore>
#include "kmlhandler.h"

class KMLMapLoader
{
public:
    KMLMapLoader();
    ~KMLMapLoader();
    void loadFile(QString fileName);
    void parseFile();
    QString msg;
    QFile file;
    QXmlSimpleReader reader;
    QDir *targetDir;
    QXmlInputSource *kmlInputSource;
    KMLHandler *contentHandler;

};

#endif // KMLMAPLOADER_H
