#include "kmlmaploader.h"

KMLMapLoader::KMLMapLoader()
{
    contentHandler = new KMLHandler();
    reader.setContentHandler(contentHandler);
    reader.setErrorHandler(contentHandler);
    targetDir = new QDir("KmlFiles");
    if(!targetDir->exists())
    {
        targetDir->cdUp();
        targetDir->mkdir("KmlFiles");
    }

}

KMLMapLoader::~KMLMapLoader()
{
    delete contentHandler;
    delete targetDir;
    delete kmlInputSource;
}

void KMLMapLoader::loadFile(QString fileName)
{
   file.setFileName(fileName);
   msg = fileName;
   //qWarning()<<fileName;
   kmlInputSource = new QXmlInputSource(&file);
}

void KMLMapLoader::parseFile()
{
   if(!file.open(QFile::ReadOnly | QFile::Text))
   {
       msg = "KML file loading error.";
   }
   else
   {
       contentHandler->KMLDataList.clear();
       reader.parse(kmlInputSource);
   }
   file.close();
}
