#ifndef KMLDATA_H
#define KMLDATA_H
#include <QtCore>

class KMLData
{
public:
    explicit KMLData(QString parentName, KMLData *parent,int level);
    QString parentName;
    KMLData *parent;
    int level;
    QList <QString> dataTitle;
    QList <KMLData *> dataChildPtr;
    QList <QString> dataInfo;
    QList <QStringList> dataAttrNameList;
    QList <QStringList> dataAttrInfoList;
    bool closedFlag;


};

#endif // KMLDATA_H
