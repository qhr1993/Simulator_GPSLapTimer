#ifndef KMLHANDLER_H
#define KMLHANDLER_H

#include <QXmlDefaultHandler>
#include <QTreeWidget>
#include "kmldata.h"

class KMLHandler : public QXmlDefaultHandler
{

public:
    KMLHandler();
    ~KMLHandler();
        bool startElement(const QString &namespaceURI, const QString &localName,
                          const QString &qName, const QXmlAttributes &attributes);
        bool endElement(const QString &namespaceURI, const QString &localName,
                        const QString &qName);
        bool characters(const QString &str);
        bool fatalError(const QXmlParseException &exception);
    QString errorString() const;
    KMLData *creatKMLData(KMLData *parent,QString node,int currentLevel);
    QString currentText;
    KMLData *KMLRoot;
    QList <KMLData> KMLDataList;
    QStringList nodeList;
    QString holdNode;
    QString errorStr;
    bool metKMLTag;
    KMLData *currentNode;
    QString lastRead;
    int currentLevel;

};

#endif // KMLHANDLER_H
