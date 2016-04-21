#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "kmlmaploader.h"
#include "laptimerthread.h"

namespace Ui {
class MainWindow;
}

struct GaussianPair{
    int sampleA;
    int sampleB;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    KMLMapLoader testLoader;
    LapTimerThread *timerThread;
    struct Sample{
        QString lat;
        QString lon;
        QString latNoise;
        QString lonNoise;
        QString time;
        QString alt;
        QString speed;
    };
    QList <Sample> sampleList;
    QList <Sample> sampleBuffer;
    int count,lapCount,noLap,ranSeed,noiseSD,emitCount;
    int xMin,xMax,yMin,yMax;
    QTimer *timer;
    GaussianPair gaussianGenerator(int minX,int maxX,int minY,int maxY);

signals:
   void currentCoordSender(QStringList list);
   void currentSpeedSender(QString speed);
   void terminateTimingThread();


private slots:
   void on_pushButton_clicked();

   void on_pushButton_2_clicked();

   void on_pushButton_3_clicked();

   void onSectionInfoReceived(QStringList list);

   void on_pushButton_4_clicked();

   void emitSample();

   void displayMsg(QString msg);

private:
        Ui::MainWindow *ui;
    };

    #endif // MAINWINDOW_H
