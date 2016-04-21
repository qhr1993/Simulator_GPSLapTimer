#include "laptimerthread.h"
#include <QtMath>
#include <QTimer>
#include <QStringList>
#include <QDebug>
#include <QFile>
#include <QDir>

LapTimerThread::LapTimerThread(QObject *parent):QThread(parent)
{

}
void LapTimerThread::initiate(QStringList coord,QStringList waypoints,bool mod,QString map)
{
    mode = mod;//false-demo true-recording
    flag = false;
    mapName = map;
    currentLat=0.0;
    currentLon=0.0;
    lastLat = 0.0;
    lastLon = 0.0;
    currentSpeed=0.0;
    currentAlt=0.0;
    currentTime.setHMS(0,0,0,0);
    lastTime.setHMS(0,0,0,0);
    currentStart.setHMS(0,0,0,0);
    QStringList waypointFrontList;
    QStringList waypointHindList;
    QList<QString>::iterator i;
    for (i = waypoints.begin(); i != waypoints.end(); ++i)
    {
        QString temp;
        if ((i+1)==waypoints.end())
            waypointFrontList.append(coord.last());
        else
        waypointFrontList.append(coord.at(coord.indexOf(temp.append(i->split(",").at(0)).append(" ").append(i->split(",").at(1)).append(" ")
                                                               .append(i->split(",").at(2)))-1));
        temp.clear();
        waypointHindList.append(coord.at(coord.indexOf(temp.append(i->split(",").at(0)).append(" ").append(i->split(",").at(1)).append(" ")
                                                               .append(i->split(",").at(2)))+1));
    }

    refLon = waypoints.last().split(",").at(0).toDouble();
    refLat = waypoints.last().split(",").at(1).toDouble();
    latScale = 6378137/180*3.1415926;
    lonScale = latScale*qCos(qAbs(refLat)/180*3.1415926);

    for (i = waypoints.begin(); i != waypoints.end(); ++i)
    {
        waypointLonList.append(lonScale*((*i).split(",").at(0).toDouble()-refLon));
        waypointLatList.append(latScale*((*i).split(",").at(1).toDouble()-refLat));
    }
    for (i = waypointFrontList.begin(); i != waypointFrontList.end(); ++i)
    {
        waypointFrontLonList.append(lonScale*((*i).split(" ").at(0).toDouble()-refLon));
        waypointFrontLatList.append(latScale*((*i).split(" ").at(1).toDouble()-refLat));
    }
    for (i = waypointHindList.begin(); i != waypointHindList.end(); ++i)
    {
        waypointHindLonList.append(lonScale*((*i).split(" ").at(0).toDouble()-refLon));
        waypointHindLatList.append(latScale*((*i).split(" ").at(1).toDouble()-refLat));
    }

    for(int j=0;j!=waypointLatList.length();j++)
    {
        double d1,d2;
        d1 = qSqrt((waypointFrontLatList.at(j)-waypointLatList.at(j))*(waypointFrontLatList.at(j)-waypointLatList.at(j))
            + (waypointFrontLonList.at(j)-waypointLonList.at(j))*(waypointFrontLonList.at(j)-waypointLonList.at(j)));
        d2 = qSqrt((waypointHindLatList.at(j)-waypointLatList.at(j))*(waypointHindLatList.at(j)-waypointLatList.at(j))
            + (waypointHindLonList.at(j)-waypointLonList.at(j))*(waypointHindLonList.at(j)-waypointLonList.at(j)));
        double xt,yt;
        yt = (waypointHindLatList.at(j)-waypointFrontLatList.at(j))*d1/(d1+d2)+waypointFrontLatList.at(j);
        xt = (waypointHindLonList.at(j)-waypointFrontLonList.at(j))*d1/(d1+d2)+waypointFrontLonList.at(j);

        if (xt == waypointLonList.at(j))//Ax+By+C=0
        {
            coefA.append(1);
            coefB.append(0);
            coefC.append(0-xt);
        }
        else
        {
            coefB.append(1);
            coefA.append((yt-waypointLatList.at(j))/(waypointLonList.at(j)-xt));
            coefC.append(xt*((yt-waypointLatList.at(j))/(xt-waypointLonList.at(j)))-yt);
        }
        sectionFlag.append(0);
        sectionChange.append(0);
        best.append(QTime());
        current.append(QTime());
    }
    sectionExp = -1;


}

LapTimerThread::~LapTimerThread()
{

}

void LapTimerThread::run()
{
 //stopWatch = new QTime(0,0,0,0);
 //stopWatch->start();
    coordTmpFile.setFileName("tmp/tmp_coord.xml");
    coordTmpFile.open(QIODevice::ReadWrite | QIODevice::Text);
    bufferCoord.setDevice(&coordTmpFile);
    bufferCoord.setAutoFormatting(true);
    bufferCoord.writeStartDocument();
    bufferCoord.writeStartElement("Dummy");

    wayptTmpFile.setFileName("tmp/tmp_waypt.xml");
    wayptTmpFile.open(QIODevice::WriteOnly | QIODevice::Text);
    bufferWaypt.setDevice(&wayptTmpFile);
    bufferWaypt.setAutoFormatting(true);
    bufferWaypt.writeStartDocument();
    bufferWaypt.writeStartElement("Dummy");


    speedTmpFile.setFileName("tmp/tmp_speed.xml");
    speedTmpFile.open(QIODevice::WriteOnly | QIODevice::Text);
    bufferSpeed.setDevice(&speedTmpFile);
    bufferSpeed.setAutoFormatting(true);
    bufferSpeed.writeStartDocument();
    bufferSpeed.writeStartElement("Dummy");

    dirName = QDate::currentDate().toString("yyyyMMdd")+QTime::currentTime().toString("hhmmss");
    QDir dir("data/");
    dir.mkdir(dirName);
    infoFile.setFileName("data/"+dirName+"/log.txt");
    infoFile.open(QIODevice::WriteOnly | QIODevice::Text);

 exec();
}

void LapTimerThread::onCurrentSpeedReceived(QString speed)
{
    currentSpeed=speed;// km/h
}

void LapTimerThread::onCurrentCoordReceived(QStringList currentCoord)
{
    lastLat = currentLat;
    lastLon = currentLon;
    lastTime = currentTime;
    double kmlCurrentLat = ((int)(currentCoord.at(0).toDouble()/100)+(currentCoord.at(0).toDouble()/100-(int)(currentCoord.at(0).toDouble()/100))*1.6666667);
    double kmlCurrentLon = ((int)(currentCoord.at(1).toDouble()/100)+(currentCoord.at(1).toDouble()/100-(int)(currentCoord.at(1).toDouble()/100))*1.6666667);
    currentLat = (((int)(currentCoord.at(0).toDouble()/100)+(currentCoord.at(0).toDouble()/100-(int)(currentCoord.at(0).toDouble()/100))*1.6666667)- refLat)*latScale;
    currentLon = (((int)(currentCoord.at(1).toDouble()/100)+(currentCoord.at(1).toDouble()/100-(int)(currentCoord.at(1).toDouble()/100))*1.6666667)- refLon)*lonScale;

    currentTime =  QTime::fromString(currentCoord.at(2), "hhmmss.zzz");
    //qWarning()<<currentCoord.at(2);
    //qWarning()<<"qnmlgb: " + currentTime.toString("hh:mm:ss.zzz");
    QString alt = currentCoord.at(3);
    alt.remove("(/s|m)");
    currentAlt = alt;
    int ctr=0;// ctr indicates the number of changes of one coordinate
    for (int i=0;i<sectionFlag.length();i++)
    {
      int temp;
      temp = sectionFlag.at(i);
      sectionFlag.replace(i,(((coefA.at(i)*currentLon + coefB.at(i)*currentLat + coefC.at(i))>0)?1:-1));
      if (temp==0)
      {
          sectionChange.replace(i,0);//first coordinate
      }
      else if (temp!=sectionFlag.at(i)&& (dist(currentLat,waypointLatList.at(i),currentLon,waypointLonList.at(i))<30) && (dist(lastLat,waypointLatList.at(i),lastLon,waypointLonList.at(i))<30))
      {
          sectionChange.replace(i,1);
          ctr++;
      }
      else
          sectionChange.replace(i,0);
    }

    if (ctr==0)
    {
        //do nothing
    }
    else if ((ctr >0))
    {
        if (sectionExp==-1)// acquisition
        {
            if (sectionChange.last()==1)//if the acquisition happens at the finish line
            {
                sectionExp = 0;
                QList <double> sentList;
                sentList.append((double)sectionExp);
                sentList.append(distLine(currentLat,currentLon,coefA.last(),coefB.last(),coefC.last()));
                sentList.append(currentTime.hour());
                sentList.append(currentTime.minute());
                sentList.append(currentTime.second());
                sentList.append(currentTime.msec());
                sentList.append(distLine(lastLat,lastLon,coefA.last(),coefB.last(),coefC.last()));
                sentList.append(lastTime.hour());
                sentList.append(lastTime.minute());
                sentList.append(lastTime.second());
                sentList.append(lastTime.msec());
                sentList.append(0);
                sectionChangeSender(sentList);
                emit sendMsg("dist1="+QString::number(sentList.at(1),'f',5)+"m  dist2="+QString::number(sentList.at(1),'f',5)+"m  \n");
                emit sendMsg("**The timer has been initiated from state Idle.\n");
            }

        }
        else if (sectionExp == sectionFlag.length()-1) //cross finish point
        {
            if (sectionChange.indexOf(1)==sectionExp)
            {
                sectionExp=0;
            QList <double> sentList;
            sentList.append((double)sectionExp);
            sentList.append(distLine(currentLat,currentLon,coefA.last(),coefB.last(),coefC.last()));
            sentList.append(currentTime.hour());
            sentList.append(currentTime.minute());
            sentList.append(currentTime.second());
            sentList.append(currentTime.msec());
            sentList.append(distLine(lastLat,lastLon,coefA.last(),coefB.last(),coefC.last()));
            sentList.append(lastTime.hour());
            sentList.append(lastTime.minute());
            sentList.append(lastTime.second());
            sentList.append(lastTime.msec());
            sentList.append(1);
            bufferWaypt.writeTextElement("when",QDate::currentDate().toString("yyyy-MM-dd")+"T"+currentTime.toString("hh:mm:ss.zzz")+"Z");
            bufferWaypt.writeTextElement("waypt",QString::number(kmlCurrentLon,'f',8)+","+QString::number(kmlCurrentLat,'f',8)+","+alt);
            sectionChangeSender(sentList);
            emit sendMsg("dist1="+QString::number(sentList.at(1),'f',5)+"m  dist2="+QString::number(sentList.at(1),'f',5)+"m  \n");
            }
            else
                sectionExp = -1; //reset section
        }
        else //cross other waypoints
        {
            if ((sectionExp+1)==(sectionChange.indexOf(1)+1))
            {
                sectionExp++;
                QList <double> sentList;
                sentList.append((double)sectionExp);//0
                sentList.append(distLine(currentLat,currentLon,coefA.at(sectionExp-1),coefB.at(sectionExp-1),coefC.at(sectionExp-1)));//1
                sentList.append(currentTime.hour());//2
                sentList.append(currentTime.minute());//3
                sentList.append(currentTime.second());//4
                sentList.append(currentTime.msec());//5
                sentList.append(distLine(lastLat,lastLon,coefA.at(sectionExp-1),coefB.at(sectionExp-1),coefC.at(sectionExp-1)));//6
                sentList.append(lastTime.hour());//7
                sentList.append(lastTime.minute());//8
                sentList.append(lastTime.second());//9
                sentList.append(lastTime.msec());//10
                sentList.append(1);
                bufferWaypt.writeTextElement("when",QDate::currentDate().toString("yyyy-MM-dd")+"T"+currentTime.toString("hh:mm:ss.zzz")+"Z");
                bufferWaypt.writeTextElement("waypt",QString::number(kmlCurrentLon,'f',8)+","+QString::number(kmlCurrentLat,'f',8)+","+alt);
                sectionChangeSender(sentList);
                emit sendMsg("dist1="+QString::number(sentList.at(1),'f',5)+"m  dist2="+QString::number(sentList.at(1),'f',5)+"m  \n");
            }
            else
                sectionExp = -1;//reset section
        }


    }

    if (flag)
    {
    bufferCoord.writeTextElement("when",QDate::currentDate().toString("yyyy-MM-dd")+"T"+currentTime.toString("hh:mm:ss.zzz")+"Z");
    bufferCoord.writeTextElement("coord",QString::number(kmlCurrentLon,'f',8)+" "+QString::number(kmlCurrentLat,'f',8)+" "+alt);
    bufferSpeed.writeTextElement("speed",currentSpeed);
    }
//    else
//    {
//        sectionExp = -1;//reset section
//    }
//qWarning()<<currentLat<<currentLon;
//qWarning()<<"sectionChange: "<<sectionChange;
//qWarning()<<"sectionFlag: "<<sectionFlag;
    emit sendMsg("sectionChange: "+QString::number(sectionChange.at(0))+QString::number(sectionChange.at(1))+QString::number(sectionChange.at(2))+QString::number(sectionChange.at(3))+"\n");
    emit sendMsg("sectionFlag: "+QString::number(sectionFlag.at(0))+QString::number(sectionFlag.at(1))+QString::number(sectionFlag.at(2))+QString::number(sectionFlag.at(3))+"\n\n");


}

double LapTimerThread::dist(double lat1,double lat2,double lon1, double lon2,bool AFlag /*= false*/)
{
    if (!AFlag)
    return qSqrt((lat2-lat1)*(lat2-lat1)+(lon2-lon1)*(lon2-lon1));
    else
    {
       return qSqrt((lat2-lat1)*latScale*(lat2-lat1)*latScale+(lon2-lon1)*lonScale*(lon2-lon1)*lonScale);
    }
}
double LapTimerThread::distLine(double lat,double lon,double A, double B, double C)
{
    return qAbs((A*lon+B*lat+C)/qSqrt(A*A+B*B));
}

QTime LapTimerThread::qTimeGap(const QTime *start, const QTime *end)
{
    int msecs = qAbs(start->msecsTo(*end));
    int hours = msecs/(1000*60*60);
    int minutes = (msecs-(hours*1000*60*60))/(1000*60);
    int seconds = (msecs-(minutes*1000*60)-(hours*1000*60*60))/1000;
    int milliseconds = msecs-(seconds*1000)-(minutes*1000*60)-(hours*1000*60*60);
    return QTime(hours,minutes,seconds,milliseconds);


}

void LapTimerThread::sectionChangeSender(QList<double> list)
{
    //display
    QTime latter(list.at(2),list.at(3),list.at(4),list.at(5));
    QTime former(list.at(7),list.at(8),list.at(9),list.at(10));
    double dis = former.msecsTo(latter);
    QString dev = "";
    double scale = list.at(6)/(list.at(6)+list.at(1));
    QTime atWaypt = former.addMSecs((int)(scale*dis));
    if (list.first()==0)//cross finish line
    {
        if (list.last()==1)//normally finish a lap
        {
            current.replace(current.length()-1,qTimeGap(&currentStart,&atWaypt));
            dev = QString::number(qTimeGap(&(best.at(list.last())),&(current.at(list.last()))).msecsSinceStartOfDay()/1000)
                    .append(".").append(qTimeGap(&(best.at(list.last())),&(current.at(list.last()))).toString("zzz"));
            dev.prepend((best.last().msecsTo(current.last())>=0)?"+":"-");
            if ((best.last().msecsTo(current.last())<0)|best.last().isNull())
                best.replace(current.length()-1,current.last());
            QStringList tmpList;
            tmpList.append(QString::number((int)list.first()));
            tmpList.append(current.last().toString("mm:ss.zzz"));
            tmpList.append(best.last().toString("mm:ss.zzz"));
            tmpList.append(dev);
            tmpList.append("");
            emit sectionInfoSender(tmpList);

        }
        else if (list.last()==0)//restart from reset
        {
            //do something
        }
            currentStart = former.addMSecs((int)(scale*dis));
            //qWarning()<<"Former Time:"<<former.toString("hh:mm:ss.zzz");
           // qWarning()<<scale;
            //qWarning()<<dis;
            //qWarning()<<"currentStart: "<<currentStart.toString("hh:mm:ss.zzz");
            emit sendMsg("Start time of current Lap: "+currentStart.toString("hh:mm:ss.zzz")+"\n\n");

        if (flag)
        {
            bufferWaypt.writeTextElement("time",current.last().toString("mm:ss.zzz"));
            bufferWaypt.writeTextElement("best",best.last().toString("hh:mm:ss.zzz"));
            bufferWaypt.writeTextElement("dev",dev);

            bufferCoord.writeEndElement();
            bufferWaypt.writeEndElement();
            bufferSpeed.writeEndElement();
        }



            bufferCoord.writeStartElement("lap");
            bufferCoord.writeAttribute("start",currentStart.toString("hh:mm:ss.zzz"));
            bufferWaypt.writeStartElement("lap");
            bufferSpeed.writeStartElement("lap");
            flag = true;

    }
    else //cross other section
    {
        current.replace(list.first()-1,qTimeGap(&currentStart,&atWaypt));
        dev = QString::number(qTimeGap(&(best.at(list.first()-1)),&(current.at(list.first()-1))).msecsSinceStartOfDay()/1000)
                .append(".").append(qTimeGap(&(best.at(list.first()-1)),&(current.at(list.first()-1))).toString("zzz"));
        dev.prepend((best.at(list.first()-1).msecsTo(current.at(list.first()-1))>=0)?"+":"-");

        if ((best.at(list.first()-1).msecsTo(current.at(list.first()-1))<0) | best.at(list.first()-1).isNull() )
            best.replace(list.first()-1,current.at(list.first()-1));
        QStringList tmpList;
        tmpList.append(QString::number((int)list.first()));//0 - waypt no
        tmpList.append(current.at(list.first()-1).toString("mm:ss.zzz"));// 1 - waypt time
        tmpList.append(best.at(list.first()-1).toString("mm:ss.zzz"));//2 - best @ this waypt
        tmpList.append(dev);//3 - waypt dev
        tmpList.append("");//4 - info
        emit sectionInfoSender(tmpList);

        bufferWaypt.writeTextElement("time",current.at(list.first()-1).toString("mm:ss.zzz"));
        bufferWaypt.writeTextElement("best",best.at(list.first()-1).toString("hh:mm:ss.zzz"));
        bufferWaypt.writeTextElement("dev",dev);
    }
}

void LapTimerThread::onThreadTerminated()
{
    bufferCoord.writeEndElement();
    bufferCoord.writeEndElement();
    bufferCoord.writeEndDocument();

    bufferWaypt.writeEndElement();
    bufferWaypt.writeEndElement();
    bufferWaypt.writeEndDocument();

    bufferSpeed.writeEndElement();
    bufferSpeed.writeEndElement();
    bufferSpeed.writeEndDocument();
    speedTmpFile.close();
    coordTmpFile.close();
    wayptTmpFile.close();

    writeTxt();
    infoFile.close();
    if (mode) writeXml();


    quit();
}

void LapTimerThread::writeXml()
{
    QXmlStreamReader xmlReader;
    coordTmpFile.open(QIODevice::ReadOnly | QIODevice::Text);
    xmlReader.setDevice(&coordTmpFile);
    xmlReader.readNext();
    int lapNum=0;
    //Reading from the file
    xmlReader.readNext();
    while (!xmlReader.isEndDocument())
    {
        if (xmlReader.isStartElement())
        {
            QString name = xmlReader.name().toString();
            if (name == "lap")
            {
                lapNum++;
            }

        }
        else if (xmlReader.isEndElement())
        {

        }
        xmlReader.readNext();
    }
    coordTmpFile.close();
    for (int i=0;i<lapNum-1;i++)// loop to generate kml file for each lap
    {

    QFile xmlKmlFile;
    QXmlStreamWriter bufferKml;

    xmlKmlFile.setFileName("data/"+dirName+"/"+dirName+"-"+QString::number(i+1)+".kml");
    xmlKmlFile.open(QIODevice::WriteOnly | QIODevice::Text);
    bufferKml.setDevice(&xmlKmlFile);
    bufferKml.setAutoFormatting(true);
    bufferKml.writeStartDocument();

    bufferKml.writeStartElement("kml");
    QXmlStreamAttributes atrr;
    atrr.append("xmlns","http://www.opengis.net/kml/2.2");
    atrr.append("xmlns:gx","http://www.google.com/kml/ext/2.2");
    atrr.append("xmlns:atom","http://www.w3.org/2005/Atom");
    bufferKml.writeAttributes(atrr);
    bufferKml.writeStartElement("Document");
    bufferKml.writeTextElement("open","1");
    bufferKml.writeTextElement("visibility","1");
    bufferKml.writeStartElement("name");
    bufferKml.writeCDATA("GPS Lap Timer - KML Log - "+QDate::currentDate().toString("yyyyMMdd")+QTime::currentTime().toString("hhmmss"));
    bufferKml.writeEndElement();
    //style - track
    bufferKml.writeStartElement("Style");
    bufferKml.writeAttribute(QXmlStreamAttribute("id","track"));
    bufferKml.writeStartElement("LineStyle");
    bufferKml.writeTextElement("color","7f0000ff");
    bufferKml.writeTextElement("width","4");
    bufferKml.writeEndElement();
    bufferKml.writeStartElement("IconStyle");
    bufferKml.writeTextElement("scale","1.3");
    bufferKml.writeStartElement("Icon");
    bufferKml.writeTextElement("href","http://earth.google.com/images/kml-icons/track-directional/track-0.png");
    bufferKml.writeEndElement();
    bufferKml.writeEndElement();
    bufferKml.writeEndElement();
    //style - start
    bufferKml.writeStartElement("Style");
    bufferKml.writeAttribute(QXmlStreamAttribute("id","start"));
    bufferKml.writeStartElement("IconStyle");
    bufferKml.writeTextElement("scale","1.3");
    bufferKml.writeStartElement("Icon");
    bufferKml.writeTextElement("href","http://maps.google.com/mapfiles/kml/paddle/grn-circle.png");
    bufferKml.writeEndElement();
    bufferKml.writeEmptyElement("hotSpot");
    atrr.clear();
    atrr.append("x","32");
    atrr.append("y","1");
    atrr.append("xunits","pixels");
    atrr.append("yunits","pixels");
    bufferKml.writeAttributes(atrr);
    bufferKml.writeEndElement();
    bufferKml.writeEndElement();
    // style - end
    bufferKml.writeStartElement("Style");
    bufferKml.writeAttribute(QXmlStreamAttribute("id","end"));
    bufferKml.writeStartElement("IconStyle");
    bufferKml.writeTextElement("scale","1.3");
    bufferKml.writeStartElement("Icon");
    bufferKml.writeTextElement("href","http://maps.google.com/mapfiles/kml/paddle/red-circle.png");
    bufferKml.writeEndElement();
    bufferKml.writeEmptyElement("hotSpot");
    bufferKml.writeAttributes(atrr);
    bufferKml.writeEndElement();
    bufferKml.writeEndElement();
    // style - statistics
    bufferKml.writeStartElement("Style");
    bufferKml.writeAttribute(QXmlStreamAttribute("id","statistics"));
    bufferKml.writeStartElement("IconStyle");
    bufferKml.writeTextElement("scale","1.3");
    bufferKml.writeStartElement("Icon");
    bufferKml.writeTextElement("href","http://maps.google.com/mapfiles/kml/pushpin/ylw-pushpin.png");
    bufferKml.writeEndElement();
    bufferKml.writeEmptyElement("hotSpot");
    atrr.clear();
    atrr.append("x","20");
    atrr.append("y","2");
    atrr.append("xunits","pixels");
    atrr.append("yunits","pixels");
    bufferKml.writeAttributes(atrr);
    bufferKml.writeEndElement();
    bufferKml.writeEndElement();
    // style - waypoint
    bufferKml.writeStartElement("Style");
    bufferKml.writeAttribute(QXmlStreamAttribute("id","waypoint"));
    bufferKml.writeStartElement("IconStyle");
    bufferKml.writeTextElement("scale","1.3");
    bufferKml.writeStartElement("Icon");
    bufferKml.writeTextElement("href","http://maps.google.com/mapfiles/kml/pushpin/blue-pushpin.png");
    bufferKml.writeEndElement();
    bufferKml.writeEmptyElement("hotSpot");
    bufferKml.writeAttributes(atrr);
    bufferKml.writeEndElement();
    bufferKml.writeEndElement();

    QStringList xmlTmpTextBuffer;
    int lapTmpNum=0;
    bool lapNumFlag=false;

    bufferKml.writeStartElement("Folder");
    bufferKml.writeStartElement("name");
    bufferKml.writeCDATA("Lap #"+QString::number(i+1));
    bufferKml.writeEndElement();
    bufferKml.writeTextElement("open","1");

    //waypoints - load
    xmlReader.clear();
    wayptTmpFile.open(QIODevice::ReadOnly | QIODevice::Text);
    xmlReader.setDevice(&wayptTmpFile);
    xmlReader.readNext();
    xmlTmpTextBuffer.clear();
    xmlTextBuffer.clear();
    lapTmpNum=0;
    lapNumFlag=false;
    QStringList bestBuffer,devBuffer,timeBuffer;
    while (!xmlReader.isEndDocument())
    {
        if (xmlReader.isStartElement())
        {
            QString name = xmlReader.name().toString();

            if (name == "when" && lapNumFlag)//read time of each element
            {
               xmlTextBuffer.append(xmlReader.readElementText());
            }
            else if (name == "waypt" && lapNumFlag)
            {
                xmlTmpTextBuffer.append(xmlReader.readElementText());
            }
            else if (name == "best" && lapNumFlag)
            {
                bestBuffer.append(xmlReader.readElementText());
            }
            else if (name == "time" && lapNumFlag)
            {
                timeBuffer.append(xmlReader.readElementText());
            }
            else if (name == "dev" && lapNumFlag)
            {
                devBuffer.append(xmlReader.readElementText());
            }
            else if (name == "lap")
            {
                lapNumFlag=(lapTmpNum==i)?true:false;
                lapTmpNum++;
            }
        }
        else if (xmlReader.isEndElement())
        {

        }
        xmlReader.readNext();
    }
    wayptTmpFile.close();
    //waypoint - write
    for (int j=0;j<xmlTextBuffer.length();j++)
    {
        bufferKml.writeStartElement("Placemark");
        bufferKml.writeStartElement("name");
        if ((j+1)==xmlTextBuffer.length())
        bufferKml.writeCDATA("Finish");
        else
        bufferKml.writeCDATA("Waypoint"+QString::number(j+1));
        bufferKml.writeEndElement();
        bufferKml.writeStartElement("description");
        bufferKml.writeCDATA("Time: "+timeBuffer.at(j)+"; To best: "+devBuffer.at(j)+"; Best: "+bestBuffer.at(j));
        bufferKml.writeEndElement();
        bufferKml.writeStartElement("TimeStamp");
        bufferKml.writeTextElement("when",xmlTextBuffer.at(j));
        bufferKml.writeEndElement();
        if (j==xmlTextBuffer.length()-1)
        bufferKml.writeTextElement("styleUrl","#end");
        else
        bufferKml.writeTextElement("styleUrl","#waypoint");
        bufferKml.writeStartElement("Point");
        bufferKml.writeTextElement("coordinates",xmlTmpTextBuffer.at(j));
        bufferKml.writeEndElement();
        bufferKml.writeEndElement();
    }

    //track - load
    xmlReader.clear();
    coordTmpFile.open(QIODevice::ReadOnly | QIODevice::Text);
    xmlReader.setDevice(&coordTmpFile);
    xmlReader.readNext();
    xmlTmpTextBuffer.clear();
    xmlTextBuffer.clear();
    lapTmpNum=0;
    lapNumFlag=false;
    while (!xmlReader.isEndDocument())
    {
        if (xmlReader.isStartElement())
        {
            QString name = xmlReader.name().toString();

            if (name == "when" && lapNumFlag)//read time of each element
            {
               xmlTextBuffer.append(xmlReader.readElementText());
            }
            else if (name == "coord" && lapNumFlag)
            {
                xmlTmpTextBuffer.append(xmlReader.readElementText());
            }
            else if (name == "lap")
            {
                lapNumFlag=(lapTmpNum==i)?true:false;
                                lapTmpNum++;
            }
        }
        else if (xmlReader.isEndElement())
        {

        }
        xmlReader.readNext();
    }
    coordTmpFile.close();
    //track - write
    bufferKml.writeStartElement("Placemark");
    bufferKml.writeAttribute(QXmlStreamAttribute("id","tour"));
    bufferKml.writeStartElement("name");
    bufferKml.writeCDATA("track");
    bufferKml.writeEndElement();
    bufferKml.writeStartElement("description");
    bufferKml.writeCDATA("");
    bufferKml.writeEndElement();
    bufferKml.writeTextElement("styleUrl","#track");
    bufferKml.writeStartElement("gx:MultiTrack");
    bufferKml.writeTextElement("altitudeMode","absolute");
    bufferKml.writeTextElement("gx:interpolate","1");
    bufferKml.writeStartElement("gx:Track");
    for (int j=0;j<xmlTextBuffer.length();j++)
    {
        bufferKml.writeTextElement("when",xmlTextBuffer.at(j));
        bufferKml.writeTextElement("gx:coord",xmlTmpTextBuffer.at(j));
    }
    //data - alt - write
    bufferKml.writeStartElement("ExtendedData");
    bufferKml.writeEmptyElement("SchemaData");
    bufferKml.writeAttribute(QXmlStreamAttribute("schemaUrl","#schema"));
    bufferKml.writeStartElement("gx:SimpleArrayData");
    bufferKml.writeAttribute(QXmlStreamAttribute("name","elevation"));
    for (int j=0;j<xmlTextBuffer.length();j++)
    {
        bufferKml.writeTextElement("gx:value",QString(xmlTmpTextBuffer.at(j)).section(" ",2,2));
    }
    bufferKml.writeEndElement();
    //data - speed - load
    xmlReader.clear();
    speedTmpFile.open(QIODevice::ReadOnly | QIODevice::Text);
    xmlReader.setDevice(&speedTmpFile);
    xmlReader.readNext();
    xmlTmpTextBuffer.clear();
    xmlTextBuffer.clear();
    lapTmpNum=0;
    lapNumFlag=false;
    while (!xmlReader.isEndDocument())
    {
        if (xmlReader.isStartElement())
        {
            QString name = xmlReader.name().toString();

            if (name == "speed" && lapNumFlag)//read time of each element
            {
               xmlTextBuffer.append(xmlReader.readElementText());
            }
            else if (name == "lap")
            {
                lapNumFlag=(lapTmpNum==i)?true:false;
                lapTmpNum++;
            }
        }
        else if (xmlReader.isEndElement())
        {

        }
        xmlReader.readNext();
    }
    speedTmpFile.close();
    //data - speed - write
    bufferKml.writeStartElement("gx:SimpleArrayData");
    bufferKml.writeAttribute(QXmlStreamAttribute("name","speed"));
    for (int j=0;j<xmlTextBuffer.length();j++)
    {
        bufferKml.writeTextElement("gx:value",xmlTextBuffer.at(j));
    }
    bufferKml.writeEndElement();

    // close all opened element
    bufferKml.writeEndElement();
    bufferKml.writeEndElement();
    bufferKml.writeEndElement();
    bufferKml.writeEndElement();
    bufferKml.writeEndElement();
    bufferKml.writeEndElement();
    bufferKml.writeEndElement();

    bufferKml.writeEndDocument();
    xmlKmlFile.close();
  }
}

void LapTimerThread::writeTxt()
{
    QTextStream tmp(&infoFile);
    QXmlStreamReader xmlReader;
    qWarning()<<coordTmpFile.open(QIODevice::ReadOnly | QIODevice::Text);
    xmlReader.setDevice(&coordTmpFile);
    xmlReader.readNext();
    int lapNum=0;
    //Reading from the file
    xmlReader.readNext();
    while (!xmlReader.isEndDocument())
    {
        if (xmlReader.isStartElement())
        {
            QString name = xmlReader.name().toString();
            if (name == "lap")
            {
                lapNum++;
            }

        }
        else if (xmlReader.isEndElement())
        {

        }
        xmlReader.readNext();
    }
    qWarning()<<lapNum;
    coordTmpFile.close();
    for (int s=0;s<(lapNum-1);s++)
    {
    qWarning()<<"entering lap "<<s;
    tmp<<"-**********************************************\r\n";
    tmp<< "Map: " <<mapName<<"\r\n";
    qWarning()<<"header";
    xmlReader.clear();
    coordTmpFile.open(QIODevice::ReadOnly | QIODevice::Text);
    xmlReader.setDevice(&coordTmpFile);
    xmlReader.readNext();
    int lapTmpNum=0;
    bool lapNumFlag=false;
    xmlTextBuffer.clear();
    //Reading from the file
    QString start;
    while (!xmlReader.isEndDocument())
    {
        if (xmlReader.isStartElement())
        {
            QString name = xmlReader.name().toString();
            if ((name == "when")&& lapNumFlag)//read time of each element
            {
               xmlTextBuffer.append(xmlReader.readElementText());
            }
            else if (name == "lap")
            {
                lapNumFlag=(lapTmpNum==s)?true:false;
                if (lapNumFlag)
                    start = xmlReader.attributes().value("start").toString();
                lapTmpNum++;
            }
        }
        else if (xmlReader.isEndElement())
        {

        }
        xmlReader.readNext();
    }
    coordTmpFile.close();
        tmp<< "Start Time: "<< QString(start).replace(QRegExp("[TZ]")," ")<<"\r\n";
        //tmp<< "End Time: "<< QString(xmlTextBuffer.last()).replace(QRegExp("[TZ]")," ")<<"\r\n";
        //tmp<< "Duration: "<< QTime::fromMSecsSinceStartOfDay(QTime::fromString(QString(xmlTextBuffer.first()).section(QRegExp("[TZ]"),1,1),"hh:mm:ss.zzz")
              //.msecsTo(QTime::fromString(QString(xmlTextBuffer.last()).section(QRegExp("[TZ]"),1,1),"hh:mm:ss.zzz"))).toString("hh:mm:ss.zzz")<<"\r\n";
        xmlReader.clear();
        coordTmpFile.open(QIODevice::ReadOnly | QIODevice::Text);
        xmlReader.setDevice(&coordTmpFile);
        xmlReader.readNext();
        lapTmpNum=0;
        lapNumFlag=false;
        xmlTextBuffer.clear();
        //Reading from the file
        while (!xmlReader.isEndDocument())
        {
            if (xmlReader.isStartElement())
            {
                QString name = xmlReader.name().toString();
                if ((name == "coord")&& lapNumFlag)//read coord of each element
                {
                   xmlTextBuffer.append(xmlReader.readElementText());
                }
                else if (name == "lap")
                {
                    lapNumFlag=(lapTmpNum==s)?true:false;
                    lapTmpNum++;
                }
            }
            else if (xmlReader.isEndElement())
            {

            }
            xmlReader.readNext();
        }
        coordTmpFile.close();


        double distance = 0;
        double maxAlt = 0, minAlt = 10000;
        QList <QString>::iterator i;
        for (i=xmlTextBuffer.begin();i!=xmlTextBuffer.end();++i)
        {
            if ((i+1) == xmlTextBuffer.end())
            {
                QList <QString>::iterator ii = xmlTextBuffer.begin();
                distance +=dist(i->split(" ").at(1).toDouble(),ii->split(" ").at(1).toDouble(),i->split(" ").at(0).toDouble(),ii->split(" ").at(0).toDouble(),true);
            }
            else
            {
                distance += dist(i->split(" ").at(1).toDouble(),(i+1)->split(" ").at(1).toDouble(),i->split(" ").at(0).toDouble(),(i+1)->split(" ").at(0).toDouble(),true);
            }
                minAlt = (i->split(" ").at(2).toDouble()<minAlt)?(i->split(" ").at(2).toDouble()):minAlt;
                maxAlt = (i->split(" ").at(2).toDouble()>maxAlt)?(i->split(" ").at(2).toDouble()):maxAlt;
        }
        tmp<< "Distance: "<< QString::number((int)distance)<<" m"<<"\r\n";
        tmp<< "Max Alt: "<< QString::number(maxAlt,'f',4)<<" m"<<"\r\n";
        tmp<< "Min Alt: "<< QString::number(minAlt,'f',4)<<" m"<<"\r\n";
        tmp<< "Elevation Variance:" << QString::number(maxAlt-minAlt,'f',4)<<" m"<<"\r\n";

        xmlReader.clear();
        speedTmpFile.open(QIODevice::ReadOnly | QIODevice::Text);
        xmlReader.setDevice(&speedTmpFile);
        xmlReader.readNext();
        lapTmpNum=0;
        lapNumFlag=false;
        xmlTextBuffer.clear();
        //Reading from the file
        while (!xmlReader.isEndDocument())
        {
            if (xmlReader.isStartElement())
            {
                QString name = xmlReader.name().toString();
                if ((name == "speed")&& lapNumFlag)//read speed of each element
                {
                   xmlTextBuffer.append(xmlReader.readElementText());
                }
                else if (name == "lap")
                {
                    lapNumFlag=(lapTmpNum==s)?true:false;
                    lapTmpNum++;
                }
            }
            else if (xmlReader.isEndElement())
            {

            }
            xmlReader.readNext();
        }
        speedTmpFile.close();

        double maxSpeed = 0, minSpeed = 10000, speedSum = 0;
        QList <QString>::iterator j;
        for (j=xmlTextBuffer.begin();j!=xmlTextBuffer.end();++j)
        {
                minSpeed = (j->toDouble()<minSpeed)?(j->toDouble()):minSpeed;
                maxSpeed = (j->toDouble()>maxSpeed)?(j->toDouble()):maxSpeed;
                speedSum += j->toDouble();
        }

        tmp<< "Max Speed: "<< QString::number(maxSpeed,'f',4)<<" km/h"<<"\r\n";
        tmp<< "Min Alt: "<< QString::number(minSpeed,'f',4)<<" km/h"<<"\r\n";
        tmp<< "Average Speed **: "<< QString::number(speedSum/(xmlTextBuffer.size()),'f',4)<<" km/h"<<"\r\n";

        xmlReader.clear();
        wayptTmpFile.open(QIODevice::ReadOnly | QIODevice::Text);
        xmlReader.setDevice(&wayptTmpFile);
        xmlReader.readNext();
        lapTmpNum=0;
        lapNumFlag=false;
        xmlTextBuffer.clear();
        //Reading from the file
        QStringList tmpWhen,tmpBest,tmpDev;
        while (!xmlReader.isEndDocument())
        {
            if (xmlReader.isStartElement())
            {
                QString name = xmlReader.name().toString();
                if ((name == "time")&& lapNumFlag)//read time of each element
                {
                   xmlTextBuffer.append(xmlReader.readElementText());
                }
                else if ((name == "best")&& lapNumFlag)//read best of each element
                {
                   tmpBest.append(xmlReader.readElementText());
                }
                else if ((name == "when")&& lapNumFlag)//read when of each element
                {
                   tmpWhen.append(xmlReader.readElementText());
                }
                else if ((name == "dev")&& lapNumFlag)//read dev of each element
                {
                   tmpDev.append(xmlReader.readElementText());
                }
                else if (name == "lap")
                {
                    lapNumFlag=(lapTmpNum==s)?true:false;
                    lapTmpNum++;
                }
            }
            else if (xmlReader.isEndElement())
            {

            }
            xmlReader.readNext();
        }
        wayptTmpFile.close();
        for (int k=0;k<xmlTextBuffer.length();k++)
        {
            if (k==(xmlTextBuffer.length()-1))
            {
                tmp<<"Finish Point: "<<"\r\n        Time: "
                               <<xmlTextBuffer.at(k)<<"\r\n        Best: "
                              <<tmpBest.at(k)<<"\r\n        Time To Best: "
                             <<tmpDev.at(k)<<"\r\n";
            }
            else
            {
                tmp<<"Waypoint "<<QString::number(k+1)<<": "<<"\r\n        Time: "
                               <<xmlTextBuffer.at(k)<<"\r\n        Best: "
                              <<tmpBest.at(k)<<"\r\n        Time To Best: "
                             <<tmpDev.at(k)<<"\r\n";
            }
        }
    tmp<< "End Time: "<< QString(tmpWhen.last()).replace(QRegExp("[TZ]")," ")<<"\r\n";
    tmp<< "Average Speed *: "<< QString::number
              ((distance/1000.0/(QTime::fromString(start,"hh:mm:ss.zzz").msecsTo(
                              QTime::fromString(QString(tmpWhen.last()).section(QRegExp("[TZ]"),1,1),"hh:mm:ss.zzz"))))*3.6,'f',4)
       <<"\r\n";
    tmp<< "\r\n";
}
    qWarning()<<"Success";
}






