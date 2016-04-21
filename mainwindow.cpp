#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("GPS Lap Timer Simulator V0.2");
    timerThread = new LapTimerThread(this);

    timer = new QTimer(this);
    ranSeed = 0;
    lapCount = 0;
    emitCount = 0;
    noiseSD = 0;

    xMax=30;
    xMin=-30;
    yMin=-30;
    yMax=30;

    ui->pushButton_2->setDisabled(true);
    ui->pushButton->setEnabled(true);
    connect(this,SIGNAL(currentCoordSender(QStringList)),timerThread, SLOT(onCurrentCoordReceived(QStringList)));
    connect(this,SIGNAL(currentSpeedSender(QString)),timerThread, SLOT(onCurrentSpeedReceived(QString)));
    connect(this,SIGNAL(terminateTimingThread()),timerThread, SLOT(onThreadTerminated()));
    connect(timerThread,SIGNAL(sectionInfoSender(QStringList)),this,SLOT(onSectionInfoReceived(QStringList)));
    connect(timerThread,SIGNAL(sendMsg(QString)),this,SLOT(displayMsg(QString)));


//    Sample tmp;

//    tmp.lat = "5256.35133";
//    tmp.lon = "-00111.46767";
//    tmp.time = "160002.530";
//    tmp.alt = "65";
//    tmp.speed = "60";
//    sampleList.append(tmp);

//    tmp.lat = "5256.34650";
//    tmp.lon = "-00111.45750";
//    tmp.time = "160003.402";
//    tmp.alt = "67";
//    tmp.speed = "67";
//    sampleList.append(tmp);

//    tmp.lat = "5256.12833";
//    tmp.lon = "-00111.67600";
//    tmp.time = "160048.563";
//    tmp.alt = "59";
//    tmp.speed = "48";
//    sampleList.append(tmp);

//    tmp.lat = "5256.12417";
//    tmp.lon = "-00111.68250";
//    tmp.time = "160049.501";
//    tmp.alt = "61";
//    tmp.speed = "55";
//    sampleList.append(tmp);

//    tmp.lat = "5256.39933";
//    tmp.lon = "-00111.91300";
//    tmp.time = "160118893";
//    tmp.alt = "65";
//    tmp.speed = "60";
//    sampleList.append(tmp);

//    tmp.lat = "5256.40200";
//    tmp.lon = "-00111.89500";
//    tmp.time = "160119.350";
//    tmp.alt = "60";
//    tmp.speed = "74";
//    sampleList.append(tmp);

//    tmp.lat = "5256.33800";
//    tmp.lon = "-00111.66183";
//    tmp.time = "160002.530";
//    tmp.alt = "75";
//    tmp.speed = "80";
//    sampleList.append(tmp);

//    tmp.lat = "5256.33283";
//    tmp.lon = "-00111.65867";
//    tmp.time = "160002.530";
//    tmp.alt = "68";
//    tmp.speed = "49";
//    sampleList.append(tmp);

//    count = 0;
    ui->lineEdit->setReadOnly(true);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete timerThread;
    delete timer;
}

void MainWindow::displayMsg(QString msg)
{
    ui->textBrowser->insertPlainText(msg);
}


void MainWindow::on_pushButton_clicked()
{
    if (sampleBuffer.length())
    {
        timerThread->start(QThread::HighPriority);
        connect(timer, SIGNAL(timeout()), this, SLOT(emitSample()));
        timer->start(100);
        if (timerThread->isRunning())
        {
            ui->pushButton->setDisabled(true);
            ui->pushButton_2->setEnabled(true);
        }
        ui->tableWidget_2->clearContents();
    }
    else
    {
        QMessageBox messageBox;
        messageBox.critical(0,"Error","Please load samples.");
        messageBox.setFixedSize(500,200);
    }
}

void MainWindow::emitSample()
{
    //qWarning()<<sampleBuffer.length();
    if (sampleBuffer.length())
    {

        QStringList list;
        list.append(sampleBuffer.first().lat);
        list.append(sampleBuffer.first().lon);
        list.append(sampleBuffer.first().time);
        list.append(sampleBuffer.first().alt);
        emit currentSpeedSender(sampleBuffer.first().speed);
        emit currentCoordSender(list);
        sampleBuffer.removeFirst();
        emitCount++;
        ui->label_5->setText(QString::number(emitCount)+"/"+QString::number(sampleList.length()));
        displayMsg("Sample emitted: ("+QString::number(emitCount)+"/"+QString::number(sampleList.length())+")\n");
    }
    else
    {
        timer->stop();
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    if (timerThread->isRunning())
    {
        timer->stop();
        emit terminateTimingThread();
        QThread::msleep(1000);
        if(!timerThread->isRunning())
        {
            ui->pushButton_2->setDisabled(true);
            ui->pushButton->setEnabled(true);
        }
    }
}

//void MainWindow::on_pushButton_3_clicked()
//{
//    QStringList list;
//    list.append(ui->lineEdit->text());
//    list.append(ui->lineEdit_2->text());
//    list.append(ui->lineEdit_3->text());
//    list.append(ui->lineEdit_4->text());
//    emit currentSpeedSender(ui->lineEdit_5->text());
//    emit currentCoordSender(list);
//    if (count==7) count=0;
//    else count++;
//}

void MainWindow::onSectionInfoReceived(QStringList list)
{
   displayMsg("===============================\n");
    displayMsg("Waypt NO " + list.at(0)+"\n");
    displayMsg("Waypt Time " + list.at(1)+"\n");
    displayMsg("Best @this waypt " + list.at(2)+"\n");
   displayMsg("To best " + list.at(3)+"\n");
   displayMsg("===============================\n");

    ui->tableWidget_2->setRowCount(noLap);
    QString display;
    if (list.at(0)!="0")
    {
        display = list.at(1);
        QTableWidgetItem *newItem = new QTableWidgetItem(display);
        ui->tableWidget_2->setItem(lapCount, 3*(list.at(0).toInt())-3, newItem);

        display = list.at(2);
        newItem = new QTableWidgetItem(display);
        ui->tableWidget_2->setItem(lapCount, 3*(list.at(0).toInt())-2, newItem);

        display = list.at(3);
        newItem = new QTableWidgetItem(display);
        ui->tableWidget_2->setItem(lapCount, 3*(list.at(0).toInt())-1, newItem);

        display = list.at(4);
        newItem = new QTableWidgetItem(display);
        ui->tableWidget_2->setItem(lapCount, 12, newItem);
    }
    else
    {
            display = list.at(1);
            QTableWidgetItem *newItem = new QTableWidgetItem(display);
            ui->tableWidget_2->setItem(lapCount, 3*4-3, newItem);

            display = list.at(2);
            newItem = new QTableWidgetItem(display);
            ui->tableWidget_2->setItem(lapCount, 3*4-2, newItem);

            display = list.at(3);
            newItem = new QTableWidgetItem(display);
            ui->tableWidget_2->setItem(lapCount, 3*4-1, newItem);

            display = list.at(4);
            newItem = new QTableWidgetItem(display);
            ui->tableWidget_2->setItem(lapCount, 12, newItem);
        lapCount++;
    }

}

//void MainWindow::on_pushButton_4_clicked()
//{
//    ui->lineEdit->setText(sampleList.at(count).lat);
//    ui->lineEdit_2->setText(sampleList.at(count).lon);
//    ui->lineEdit_3->setText(QTime::currentTime().toString("hhmmss.zzz"));
//    ui->lineEdit_4->setText(sampleList.at(count).alt);
//    int random = qrand() % 50 + 50;
//    ui->lineEdit_5->setText(QString::number(random));
//}

void MainWindow::on_pushButton_3_clicked()
{
    QString fileN = QFileDialog::getOpenFileName(this, tr("Open File"),"",tr("KML Files (*.KML)"));
    ui->lineEdit->setText(fileN);
}

void MainWindow::on_pushButton_4_clicked()
{
    bool noLapFlag = false;
    noLap = ui->lineEdit_2->text().toInt(&noLapFlag,10);
    bool ranSeedFlag = false;
    ranSeed = ui->lineEdit_4->text().toInt(&ranSeedFlag,10);
    bool noiseSDFlag = false;
    noiseSD = ui->lineEdit_3->text().toInt(&noiseSDFlag,10);
    if (noLapFlag&&noiseSDFlag&&ranSeedFlag)
    {
    sampleList.clear();
    sampleBuffer.clear();
    ui->textBrowser->clear();
    testLoader.loadFile(ui->lineEdit->text());
    testLoader.parseFile();
    QStringList testCoord;
    QStringList testWaypoints;//0-start/end 1-waypoint1 ...
    QList <KMLData>::iterator i;
    QList <QString>::iterator j;
        qWarning()<<testLoader.contentHandler->KMLDataList.count();
    for (i=testLoader.contentHandler->KMLDataList.begin();
         i!=testLoader.contentHandler->KMLDataList.end();++i)
    {
        if (i->parentName == "gx:Track")
        {
            for (j=i->dataInfo.begin();j!=i->dataInfo.end();++j)
                //qWarning()<<(*j);
                testCoord.append(*j);
        }
        if (i->parentName == "Point")
        {
            for (j=i->dataInfo.begin();j!=i->dataInfo.end();++j)
                //qWarning()<<(*j);
                testWaypoints.append(*j);
        }
    }
qWarning()<<"pass1";
    timerThread->initiate(testCoord,testWaypoints,true,testLoader.file.fileName());

    QList <QString>::iterator k;
    QString currentTime = QTime::currentTime().toString("hhmmss.zzz");
    count = 0;

    qsrand(ranSeed);

    GaussianPair s;
    s = gaussianGenerator(-30,30,-30,30);
    xMax = s.sampleA/3*3+3;
    xMin = s.sampleA/3*3;
    yMax = s.sampleB/3*3+3;
    yMin = s.sampleB/3*3;

    ui->label_8->setText("{("+QString::number(xMin)+","+QString::number(xMax)+"),("+QString::number(yMin)+","+QString::number(yMax)+")}");

    for (k=testCoord.end()-50;k!=testCoord.end();++k)
    {

//        float uniRandA,uniRandB,normRandA,normRandB;
//        uniRandA = qrand()/32767.0;
//        uniRandB = qrand()/32767.0;
//        normRandA = sqrt(-2*qLn(uniRandA))*qCos(2*M_PI*uniRandB)*noiseSD*3/(6371000*M_PI/180*5);
//        normRandB = sqrt(-2*qLn(uniRandA))*qSin(2*M_PI*uniRandB)*noiseSD*3/(6371000*M_PI/180*5);

        GaussianPair g;
        g = gaussianGenerator(xMin,xMax,yMin,yMax);
        qWarning()<<count;


        Sample samplePoint;
        QString latStr = k->split(" ").at(1);
        QString lonStr = k->split(" ").at(0);
        samplePoint.lat = QString::number(((int)(latStr.toFloat())+((latStr.toFloat()-(int)(latStr.toFloat()))+(g.sampleA/(6371000*M_PI/180)))/5*3)*100,'f',5);
        samplePoint.lon = QString::number(((int)(lonStr.toFloat())+((lonStr.toFloat()-(int)(lonStr.toFloat()))+(g.sampleB/(6371000*M_PI/180))*qCos(latStr.toFloat()/180*M_PI))/5*3)*100,'f',5);
        samplePoint.latNoise = QString::number((g.sampleA/(6371000*M_PI/180)),'f',5);
        samplePoint.lonNoise = QString::number((g.sampleB/(6371000*M_PI/180))*qCos(latStr.toFloat()/180*M_PI),'f',5);
        samplePoint.time = QTime::fromString(currentTime,"hhmmss.zzz").addMSecs(count*200).toString("hhmmss.zzz");
        samplePoint.alt = k->split(" ").at(2);
        samplePoint.speed = QString::number((int)(qrand()/1024+40));
        sampleList.append(samplePoint);
        count++;

    }
    for (int s=0;s<noLap;s++)
    {
        for (k=testCoord.begin();k!=testCoord.end();++k)
        {
//            float uniRandA,uniRandB,normRandA,normRandB;
//            uniRandA = qrand()/32767.0;
//            uniRandB = qrand()/32767.0;
//            normRandA = sqrt(-2*qLn(uniRandA))*qCos(2*M_PI*uniRandB)*noiseSD*3/(6371000*M_PI/180*5);
//            normRandB = sqrt(-2*qLn(uniRandA))*qSin(2*M_PI*uniRandB)*noiseSD*3/(6371000*M_PI/180*5);
            GaussianPair g;
            g = gaussianGenerator(xMin,xMax,yMin,yMax);
            qWarning()<<count;



            Sample samplePoint;
            QString latStr = k->split(" ").at(1);
            QString lonStr = k->split(" ").at(0);
            samplePoint.lat = QString::number(((int)(latStr.toFloat())+((latStr.toFloat()-(int)(latStr.toFloat()))+(g.sampleA/(6371000*M_PI/180)))/5*3)*100,'f',5);
            samplePoint.lon = QString::number(((int)(lonStr.toFloat())+((lonStr.toFloat()-(int)(lonStr.toFloat()))+(g.sampleB/(6371000*M_PI/180))*qCos(latStr.toFloat()/180*M_PI))/5*3)*100,'f',5);
            samplePoint.latNoise = QString::number((g.sampleA/(6371000*M_PI/180)),'f',5);
            samplePoint.lonNoise = QString::number((g.sampleB/(6371000*M_PI/180))*qCos(latStr.toFloat()/180*M_PI),'f',5);
            samplePoint.time = QTime::fromString(currentTime,"hhmmss.zzz").addMSecs(count*200).toString("hhmmss.zzz");
            samplePoint.alt = k->split(" ").at(2);
            samplePoint.speed = QString::number((int)(qrand()/1024+40));
            sampleList.append(samplePoint);
            count++;
        }
    }

    for (k=testCoord.begin();k!=testCoord.begin()+50;++k)
    {
//        float uniRandA,uniRandB,normRandA,normRandB;
//        uniRandA = qrand()/32768.0;
//        uniRandB = qrand()/32768.0;
//        normRandA = sqrt(-2*qLn(uniRandA))*qCos(2*M_PI*uniRandB)*noiseSD*3/(6371000*M_PI/180*5);
//        normRandB = sqrt(-2*qLn(uniRandA))*qSin(2*M_PI*uniRandB)*noiseSD*3/(6371000*M_PI/180*5);

        GaussianPair g;
        g = gaussianGenerator(xMin,xMax,yMin,yMax);
        qWarning()<<count;



        Sample samplePoint;
        QString latStr = k->split(" ").at(1);
        QString lonStr = k->split(" ").at(0);
        samplePoint.lat = QString::number(((int)(latStr.toFloat())+((latStr.toFloat()-(int)(latStr.toFloat()))+(g.sampleA/(6371000*M_PI/180)))/5*3)*100,'f',5);
        samplePoint.lon = QString::number(((int)(lonStr.toFloat())+((lonStr.toFloat()-(int)(lonStr.toFloat()))+(g.sampleB/(6371000*M_PI/180))*qCos(latStr.toFloat()/180*M_PI))/5*3)*100,'f',5);
        samplePoint.latNoise = QString::number((g.sampleA/(6371000*M_PI/180)),'f',5);
        samplePoint.lonNoise = QString::number((g.sampleB/(6371000*M_PI/180))*qCos(latStr.toFloat()/180*M_PI),'f',5);
        samplePoint.time = QTime::fromString(currentTime,"hhmmss.zzz").addMSecs(count*200).toString("hhmmss.zzz");
        samplePoint.alt = k->split(" ").at(2);
        samplePoint.speed = QString::number((int)(qrand()/1024+40));
        sampleList.append(samplePoint);
        count++;
    }
    }
    else
    {
        QMessageBox messageBox;
        messageBox.critical(0,"Error","Please specify parameters correctly.");
        messageBox.setFixedSize(500,200);
    }

    QList <Sample>::iterator i;
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(sampleList.length());
    int c =0;
    for (i=sampleList.begin();i!=sampleList.end();++i)
    {
        //ui->textBrowser->clear();
        QString display;
//        display.append("Coordinate from Fake GPS Receiver in NMEA\n");
//        display.append("=============================\n");
//        display.append("Lat: ");
//        display.append(i->lat+"\n");
//        display.append("Lon: ");
//        display.append(i->lon+"\n");
//        display.append("Spd: ");
//        display.append(i->speed+"\n");
//        display.append("Alt: ");
//        display.append(i->alt+"\n");
//        display.append("Time: ");
//        display.append(i->time+"\n");
//        display.append("=============================\n");
//        display.append("\n\n");
        display = i->lat;
        QTableWidgetItem *newItem = new QTableWidgetItem(display);
        ui->tableWidget->setItem(c, 0, newItem);
        display = i->latNoise;
        newItem = new QTableWidgetItem(display);
        ui->tableWidget->setItem(c, 1, newItem);
        display = i->lon;
        newItem = new QTableWidgetItem(display);
        ui->tableWidget->setItem(c, 2, newItem);
        display = i->lonNoise;
        newItem = new QTableWidgetItem(display);
        ui->tableWidget->setItem(c, 3, newItem);
        display = i->alt;
        newItem = new QTableWidgetItem(display);
        ui->tableWidget->setItem(c, 4, newItem);
        display = i->speed;
        newItem = new QTableWidgetItem(display);
        ui->tableWidget->setItem(c, 5, newItem);
        display = i->time;
        newItem = new QTableWidgetItem(display);
        ui->tableWidget->setItem(c, 6, newItem);

        c++;
    }
    sampleBuffer = sampleList;
}

GaussianPair MainWindow::gaussianGenerator(int minX, int maxX, int minY, int maxY)
{
    float uniRandA,uniRandB,normRandA,normRandB;
    uniRandA = qrand()/32768.0;
    uniRandB = qrand()/32768.0;
    normRandA = sqrt(-2*qLn(uniRandA))*qCos(2*M_PI*uniRandB)*noiseSD;
    normRandB = sqrt(-2*qLn(uniRandA))*qSin(2*M_PI*uniRandB)*noiseSD;
        while ((normRandA<minX)|(normRandA>maxX)|(normRandB<minY)|(normRandB>maxY))
    {
        uniRandA = qrand()/32768.0;
        uniRandB = qrand()/32768.0;
        normRandA = sqrt(-2*qLn(uniRandA))*qCos(2*M_PI*uniRandB)*noiseSD;
        normRandB = sqrt(-2*qLn(uniRandA))*qSin(2*M_PI*uniRandB)*noiseSD;
//        if (!((normRandA<minX)|(normRandA>maxX)|(normRandB<minY)|(normRandB>maxY)))
//        {
//            qWarning()<<"success";
//            qWarning()<<normRandA;
//        }
    }
    GaussianPair p;
    p.sampleA = (int)normRandA;
    p.sampleB = (int)normRandB;
    return p;
}
