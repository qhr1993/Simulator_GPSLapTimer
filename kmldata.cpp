#include "kmldata.h"

KMLData::KMLData(QString parentName, KMLData *parent,int level)
{
    this->parentName = parentName;
    this->parent = parent;
    this->level = level;
    this->closedFlag = false;
}
