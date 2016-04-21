#ifndef LAPTIMERTHREAD_H
#define LAPTIMERTHREAD_H
#include <QThread>
#include <QObject>
#include <QString>
#include <QTime>
#include <QStringList>
#include <QXmlStreamWriter>
#include <QFile>

class LapTimerThread : public QThread
{
    Q_OBJECT
public:
    explicit LapTimerThread(QObject *parent);
    void initiate(QStringList coord,QStringList waypoints,bool mod,QString map);
    ~LapTimerThread();
    void run();
    double currentLon,currentLat;
    QString currentSpeed,currentAlt;
    double lastLon,lastLat;
    QList <double> waypointLonList,waypointLatList;
    QList <double> waypointFrontLonList, waypointHindLonList,waypointFrontLatList, waypointHindLatList;
    QList <double> coefA,coefB,coefC;
    QList <int> sectionFlag,sectionChange;
    QList <QTime> best,current;
    int sectionExp;
    double refLon,refLat;
    double latScale,lonScale;
    QTime currentTime,lastTime,currentStart;
    bool mode;
    QString mapName;
public slots:
    void onCurrentCoordReceived(QStringList currentCoord);
    void onCurrentSpeedReceived(QString speed);
    void onThreadTerminated();
signals:
    void sectionInfoSender(QStringList list);
    void sendMsg(QString msg);
private:
    double dist(double lat1,double lat2,double lon1,double lon2,bool AFlag = false);
    double distLine(double lat,double lon,double A, double B, double C);
    QTime qTimeGap(const QTime *start, const QTime *end);
    void sectionChangeSender(QList <double>);
    QXmlStreamWriter bufferCoord,bufferWaypt,bufferSpeed;
    QFile coordTmpFile,wayptTmpFile,speedTmpFile,infoFile;
    QStringList xmlTextBuffer;
    void writeXml();
    void writeTxt();
    bool flag;
    QString dirName;


};

#endif // LAPTIMERTHREAD_H
