#include "kmlhandler.h"

KMLHandler::KMLHandler() :
    QXmlDefaultHandler()
{
    nodeList<<"Document"<<"Folder"<<"name"<<"Placemark"
           <<"Description"<<"Point"<<"LineString"<<"LinearRing"<<"Polygon"
          <<"gx:MultiTrack"<<"gx:Track"<<"gx:coord"<<"coordinates"<<"kml";
    metKMLTag = false;
    lastRead = "";
    currentLevel = -1;
    holdNode= "";

}

KMLHandler::~KMLHandler()
{

}

KMLData *KMLHandler::creatKMLData(KMLData *parent,QString node, int currentLevel)
{
    currentLevel++;
    KMLData tmpData(node,parent,currentLevel);
    KMLDataList.append(tmpData);
    return &(KMLDataList.last());
}

bool KMLHandler::startElement(const QString & /* namespaceURI */,
                               const QString & /* localName */,
                               const QString &qName,
                               const QXmlAttributes &attributes)
{
    if (!metKMLTag)
    {
        if(qName!="kml")
        {
        errorStr = QObject::tr("The file is not a KML file.");
        return false;
        }
        else
        {
         KMLRoot = this->creatKMLData(0,qName,currentLevel);
         currentNode = KMLRoot;
         currentNode->parent = currentNode;
         lastRead = qName;
         metKMLTag = true;
         //qWarning()<<qName;
         return true;
        }
    }
    if (!holdNode.isEmpty())
        return true;

    if (nodeList.indexOf(qName)>=0)
    {
        KMLData *ptr = this->creatKMLData(currentNode,qName,currentLevel);
        currentNode->dataTitle.append(qName);
        currentNode->dataChildPtr.append(ptr);
        currentNode->dataInfo.append("");
        QStringList tmpList1,tmpList2;
        for (int i=0;i<attributes.count();i++)
        {
            tmpList1.append(attributes.value(i));
            tmpList2.append(attributes.qName(i));
        }
        currentNode->dataAttrInfoList.append(tmpList1);
        currentNode->dataAttrNameList.append(tmpList2);
        currentNode = ptr;
        lastRead = qName;
        //qWarning()<<qName;

        metKMLTag = true;
    }
    else if (nodeList.indexOf(qName)==-1)
    {
        holdNode=qName;
        //qWarning()<<qName;

    }

    currentText.clear();
    return true;
}

bool KMLHandler::endElement(const QString & /* namespaceURI */,
                             const QString & /* localName */,
                             const QString &qName)
{
    if (nodeList.indexOf(qName)>=0)
    {

        if (!holdNode.isEmpty())
        {

        }
        else if (qName == lastRead)
        {
            currentNode = currentNode->parent;
            KMLDataList.removeLast();
            currentNode->dataChildPtr.last() = 0;
            currentNode->dataInfo.last() = currentText;
            //qWarning()<<qName;
        }
        else
        {
            //qWarning()<<qName;
            currentNode->closedFlag = true;
            currentNode = currentNode->parent;

        }
        currentLevel--;
    }
    else if (nodeList.indexOf(qName)==-1)
    {
      if (!holdNode.isEmpty())
      {
          if (holdNode==qName)
              holdNode.clear();
      }
      else
          this->errorStr= "Error when reading.";
    }
    return true;
}

bool KMLHandler::characters(const QString &str)
{
   if (holdNode.isEmpty())
    currentText += str;
   //qWarning()<<currentText;
    return true;
}

bool KMLHandler::fatalError(const QXmlParseException &exception)
{

    qWarning()<<"XML Error line: "<<exception.lineNumber()<<
                             " column: "<<exception.columnNumber()<<
                             " message: "<<exception.message();
    return false;
}

QString KMLHandler::errorString() const
{
    return errorStr;
}
