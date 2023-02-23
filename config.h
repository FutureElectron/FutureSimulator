#ifndef CONFIG_H
#define CONFIG_H

#include "qvalidator.h"
#include <QDialog>

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
    bool configValidation();

    void setPort(const QString &newPort);
    void setIpaddr(const QString &newIpaddr);
    void setBaudRate(const QString &newBaudRate);
    void setDataBits(const QString &newDataBits);
    void setStopBits(const QString &newStopBits);
    void setParity(const QString &newParity);
    void setFlowControl(const QString &newFlowControl);
    void setOperatingMode(const QString &newOperatingMode);
    void setTCPActive(const bool newTCPActive);
    void setSerialActvie(const bool newSerialActvie);
    void setTCPPort(QString newTCPPort);

public slots:
    void setvalues(void);

private:
    Ui::Config *ui;
    QString serialPort;
    QString baudRate;
    QString dataBits;
    QString stopBits;
    QString parity;
    QString flowControl;
    QString operatingMode;
    QString ipaddr;
    quint16 tcpPort;
    bool TCPActive = false;
    bool serialActvie = false;
    bool acceptedIP{false}, acceptedPort{false};

    Q_PROPERTY(QString ipaddr READ getIpaddr CONSTANT)
    Q_PROPERTY(QString tcpPort READ getTcpPort CONSTANT)
};

#endif // CONFIG_H
