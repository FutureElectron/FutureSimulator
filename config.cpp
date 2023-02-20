#include "config.h"
#include "ui_config.h"
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QMessageBox>

Config::Config(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Config)
{
    QSerialPortInfo serialInfo;

//    QValidator *validator = new QIntValidator(1,65535,this);
//    ui->tcp_port->setValidator(validator);

    QList<QSerialPortInfo> ports = serialInfo.availablePorts();
    QStringList stringPorts;
        for(int i = 0 ; i < ports.size() ; i++){
            stringPorts.append(ports.at(i).portName());
        }

    ui->setupUi(this);
    ui->portCombo->addItems(stringPorts);


    connect(ui->cancelBtn,&QPushButton::clicked,this, [=](){
        reject();
    });
    connect(ui->okBtn, &QPushButton::clicked,this, &Config::setvalues);
    }

Config::~Config()
{
    delete ui;
}

QString Config::getPort() const
{
    return port;
}

QString Config::getBaudRate() const
{
    return baudRate;
}

QString Config::getDataBits() const
{
    return dataBits;
}

QString Config::getStopBits() const
{
    return stopBits;
}

QString Config::getParity() const
{
    return parity;
}

QString Config::getFlowControl() const
{
    return flowControl;
}

QString Config::getOpMode() const
{
    return operatingMode;
}

void Config::setvalues()
{
    bool ok;
    port = ui->portCombo->currentText();
    baudRate = ui->baudrateCombo->currentText();
    dataBits = ui->baudrateCombo->currentText();
    stopBits = ui->stopbitsCombo->currentText();
    parity = ui->parityCombo->currentText();
    flowControl = ui->flowcontrolCombo->currentText();
    operatingMode = ui->modeCombo->currentText();
    ipaddr = ui->ipaddr->text();
    tcpPort = ui->tcp_port->text().toInt(&ok);
    if (!ok)
    {
        ui->tcp_port->setText("");
       QMessageBox::warning(this,"Invalid Port Address", "Port must be integer valued! "
                                   "Please try again ...",QMessageBox::Ok);
       setvalues();
    }

    TCPActive = ui->setTCPActive->isChecked();
    serialActvie = ui->setSerialActive->isChecked();
    accept();
}

Ui::Config *Config::getUi() const
{
    return ui;
}

QString Config::getOperatingMode() const
{
    return operatingMode;
}

bool Config::getTCPActive() const
{
    return TCPActive;
}

bool Config::getSerialActvie() const
{

    return serialActvie;
}

QString Config::getIpaddr() const
{

    return ipaddr;
}

quint16 Config::getTcpPort() const
{
    return tcpPort;
}
