#ifndef CONFIG_H
#define CONFIG_H

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

  public slots:
    void setvalues(void);

private:
    Ui::Config *ui;
    QString port;
    QString baudRate;
    QString dataBits;
    QString stopBits;
    QString parity;
    QString flowControl;
    QString operatingMode;
};

#endif // CONFIG_H
