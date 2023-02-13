#ifndef WIDGET_H
#define WIDGET_H

#include "qlabel.h"
#include "qplaintextedit.h"
#include <QWidget>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QSettings>
#include <QDir>
#include <QFileDialog>
#include <QRandomGenerator>
#include <QTimer>
#include <QFile>
#include <QIODevice>
#include <QDateTime>
#include <QProcess>
#include <QDesktopServices>
#include <QtConcurrent>
#include <QFuture>
#include <QInputDialog>
//#include <QEventLoop>
//#include <QFutureWatcher>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT
    friend void updateLabel(QPlainTextEdit *e);
public:
    Widget(QWidget *parent = nullptr);
    void signalSlotsConnections();
//    void connectSerial();
    void portConfiguration();
    void sendCommandLine();
    void saveSettings();
    bool loadSettings();
    void NormalSignalGenerator();
    void sendNormalPWMs(int seconds);
    void setCheckBtn(QLabel *obj);
    QFile *createLogFile(QString type);
    void idealSignalGenerator();
    void abnormalSignalGenerator(int seconds);
    void openFileInExplorer();
    void processRead();
    QFile *logfile;

    /////
    void selectClassificationClass();
    void sendCommand(QString cmd);
    void sendClassificationPattern(int seconds);
    void receiveData();
    ~Widget();

 signals:
    void updateEdit(QString message);

public slots:
    void showConfigDialog();
    void sendSliderPWM();
    void connectSerial();
    void restartLogging();
    void adjustTable();
    void readSerialData();
    void classification();


private:
    Ui::Widget *ui;
    QString port;
    QString mes;
    QString baudRate;
    QString dataBits;
    QString stopBits;
    QString parity;
    QString flowControl;
    QString verbosity;
    QString opMode;
    QSerialPort serial;
    QSerialPortInfo info;
    bool connected {false}, logging{false};
    bool enableOutput {true};
    QString buffer, lastcommand;
    bool settingsSaved{false};
    QString logfolderPath;
    QPair<QList<int>,QList<int>> resultofAutoGenAbnormal;
    QList<int> abnormalPWMs,abnormalTimes;
    QTimer NormalTimer, abnormalTimer, idealTimer, classificationTimer;
    int pIndex = 0;
    int logtime{50};

    QProcess *proc = new QProcess(this);

    struct Signal
    {
//        Signal(QList<int> p, QList<int> t) {
//            pwm = p;
//            timestamps = t;
//        }

        QList<int> pwm;
        QList<int> timestamps;
        int basePWM;
    };


    Signal classSignal;
    Signal selectedClassSignal;
    QMap<QString, Signal> sigVals;

    QString inActiveBtnStyle = R"(
            QToolButton{
                border:1px solid;
                border-radius:15px;
                border-color: rgb(255, 255, 255);
                background-color: transparent;
            }

            QToolButton:hover{
                background-color: rgb(0, 22, 18);
            }
        )";
    QString activeBtnStyle = R"(
            QToolButton{
                border:1px solid;
                border-radius:15px;
                border-color: rgb(255, 255, 255);
                background-color: rgb(0, 22, 20);
            }

            QToolButton:hover{
                background-color: rgb(0, 22, 18);
            }
        )";

    // ------------
    QFuture<void> future;

};
#endif // WIDGET_H
