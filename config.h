#ifndef CONFIG_H
#define CONFIG_H

#include "qlineedit.h"
#include "qvalidator.h"
#include <QDialog>
//#include <QButtonGroup>

namespace Ui {
class Config;
}

class Config : public QDialog
{
    Q_OBJECT

public:
    explicit Config(QWidget *parent = nullptr);
    ~Config();

    QString getPort() const;
    QString getBaudRate() const;
    QString getDataBits() const;
    QString getStopBits() const;
    QString getParity() const;
    QString getFlowControl() const;
    QString getOpMode() const;

    QString getIpaddr() const;
    quint16 getTcpPort() const;

    Ui::Config *getUi() const;
    QString getOperatingMode() const;
    bool getTCPActive() const;
    bool getSerialActvie() const;

    void setPort(const QString &newPort);
    void setIpaddr(const QString &newIpaddr);
    void setHostname(const QString &hostname);
    void setBaudRate(const QString &newBaudRate);
    void setDataBits(const QString &newDataBits);
    void setStopBits(const QString &newStopBits);
    void setParity(const QString &newParity);
    void setFlowControl(const QString &newFlowControl);
    void setOperatingMode(const QString &newOperatingMode);
    void setTCPActive(const bool newTCPActive);
    void setSerialActvie(const bool newSerialActvie);
    void setTCPPort(QString newTCPPort);

    bool getUdpActive() const;
    void setUdpActive(bool newUdpActive);
    bool portValidation(QLineEdit *str_portObj, qint16 &int_port);
    bool ipValidation(QLineEdit *str_ipaddr, QString &ipaddr);


    int getUseHostName() const;
    void setUseHostName(int newUseHostName);

    quint16 getUdpRemotePort() const;
    void setUdpRemotePort(quint16 newUdpRemotePort);

    quint16 getUdpLocalPort() const;
    void setUdpLocalPort(quint16 newUdpLocalPort);

    QString getUdpLocalIPAddr() const;
    void setUdpLocalIPAddr(const QString &newUdpLocalIPAddr);

    QString getUdpRemoteIPAddr() const;
    void setUdpRemoteIPAddr(const QString &newUdpRemoteIPAddr);

public slots:
    void setvalues(void);
    void hostNameCheckBoxStateChanged(int state);

private:
    Ui::Config *ui;

    QValidator *portValidator;
    QRegularExpressionValidator *ipValidator;

    QString serialPort;
    QString baudRate;
    QString dataBits;
    QString stopBits;
    QString parity;
    QString flowControl;
    QString operatingMode;
    QString tcpIPAddr, udpLocalIPAddr, udpRemoteIPAddr;
    qint16 tcpPort, udpLocalPort, udpRemotePort;
//    QButtonGroup *activeConnection;
    bool TCPActive = false;
    bool serialActvie = false;
    bool acceptedIP{false}, acceptedPort{false};
    bool useHostName {false};
    bool UDPActive;
    Q_PROPERTY(QString ipaddr READ getIpaddr CONSTANT)
    Q_PROPERTY(QString tcpPort READ getTcpPort CONSTANT)
};

#endif // CONFIG_H
