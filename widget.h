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
#include <QTcpSocket>
#include <QAbstractSocket>
#include <QMetaEnum>
#include <QTcpServer>
#include <QUdpSocket>
#include <QNetworkDatagram>
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
    void serialPortConfiguration();
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
    void serialReceiveData();
    void TCPServerConnection(QString host, quint16 port);
    ~Widget();

 signals:
    void updateEdit(QString message);

public slots:
    void showConfigDialog();
    void sendPWMFromSlider();
    void uiRestartLogging();
    void uiAdjustTable();
    void readSerialData();
    void classification();
    void TCPConnectToHost(QString host, quint16 port);
    void TCPDisconnect();
    void UDPreadyRead();

private slots:
    void TCPConnectionSuccessful();
    void TCPdisconnected();
    void TCPConnectionError(QAbstractSocket::SocketError socketError);
    void TCPStateChanged(QAbstractSocket::SocketState socketState);
    void TCPReceiveData();
    void connectionHandler(void);
    void serverHasNewConnection();
    void TCPServerHasNewData(QTcpSocket *newSereverSocket);
    void TCPServerSocketLostClient(QTcpSocket *newSereverSocket);


private:
    Ui::Widget *ui;
    QString tcpIPAddr, udpLocalIPAddr, udpRemoteIPAddr;
    quint16 tcpPort, udpLocalPort, udpRemotePort;
    QString serialPort;
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
    QString lastConnectionState;
    bool connected {false};
    bool logging{false};
    bool enableOutput {true};
    bool serialActive{false}, tcpActive{false}, udpActive{false};
    bool UDPConnected{false};
    QString buffer, lastcommand;
    bool settingsSaved{false};
    QString logfolderPath;
    QPair<QList<int>,QList<int>> resultofAutoGenAbnormal;
    QList<int> abnormalPWMs,abnormalTimes;
    QTimer NormalTimer, abnormalTimer, idealTimer, classificationTimer;
    int pIndex = 0;
    int logtime{50};

    QProcess *openExplorerProcess = new QProcess(this);

    QTcpSocket TCPSocket;
    QUdpSocket UDPSocket;
    QTcpServer *server ;

    struct Signal
    {
        QList<int> pwm;
        QList<int> timestamps;
        int basePWM;
    };


    Signal classSignal;
    Signal selectedClassSignal;
    QMap<QString, Signal> sigVals;
    QFuture<void> future;
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



    void uiConnectionSuccessful(QString msg);
    void uiDisconnectionSuccessful(QString msg);
    bool serialConnected{false}, TCPConnected {false};
    int useHostName {0};

};
#endif // WIDGET_H
