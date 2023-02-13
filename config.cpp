#include "config.h"
#include "ui_config.h"
#include <QSerialPortInfo>
#include <QSerialPort>

Config::Config(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Config)
{
    QSerialPortInfo serialInfo;
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
    port = ui->portCombo->currentText();
    baudRate = ui->baudrateCombo->currentText();
    dataBits = ui->baudrateCombo->currentText();
    stopBits = ui->stopbitsCombo->currentText();
    parity = ui->parityCombo->currentText();
    flowControl = ui->flowcontrolCombo->currentText();
    operatingMode = ui->modeCombo->currentText();
    accept();
}
