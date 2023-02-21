#include "config.h"
#include "qbuttongroup.h"
#include "ui_config.h"
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QMessageBox>
#include <QButtonGroup>

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

    QButtonGroup *activeConnection = new QButtonGroup(ui->tabWidget);
    activeConnection->addButton(ui->setSerialActive);
    activeConnection->addButton(ui->setTCPActive);
    activeConnection->setExclusive(true);

    QValidator *validator = new QIntValidator(1,65535,this);
    ui->tcp_port->setValidator(validator);
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

    if(ui->setSerialActive->isChecked())
    {
        port = ui->portCombo->currentText();
        baudRate = ui->baudrateCombo->currentText();
        dataBits = ui->baudrateCombo->currentText();
        stopBits = ui->stopbitsCombo->currentText();
        parity = ui->parityCombo->currentText();
        flowControl = ui->flowcontrolCombo->currentText();
        operatingMode = ui->modeCombo->currentText();
        ipaddr = ui->ipaddr->text();
        serialActvie = ui->setSerialActive->isChecked();
        accept();
    }

    if (ui->setTCPActive->isChecked())
    {
        QString IpRange = "(?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])";
        QRegularExpression IpRegex ("^" + IpRange
                                    + "(\\." + IpRange + ")"
                                    + "(\\." + IpRange + ")"
                                    + "(\\." + IpRange + ")$");
        QRegularExpressionValidator *ipValidator = new QRegularExpressionValidator(IpRegex, this);
        ui->ipaddr->setValidator(ipValidator);
        int pos{0};
        QString tmp_ipaddr = ui->ipaddr->text();

        bool acceptedIP{false}, acceptedPort{false};
        if (ipValidator->validate(tmp_ipaddr,pos) !=2)
        {
            QMessageBox::warning(this,"Invalid IP Address", "Please enter a valid IP address! "
                                                              "Please try again ...",QMessageBox::Ok);
            ui->ipaddr->setText(""); 
            acceptedIP = false;

        }
        else
        {
            acceptedIP = true;
            ipaddr = tmp_ipaddr;
        }

        // Validate TCP port entries
        QString tmp_tcpport =  ui->tcp_port->text();
        QValidator *portValidator = new QIntValidator(1,65535,this);

        if(portValidator->validate(tmp_tcpport,pos)==2)
        {
            tcpPort = tmp_tcpport.toInt();
            TCPActive = ui->setTCPActive->isChecked();
            acceptedPort = true;
        }
        else
        {
            QMessageBox::warning(this,"Invalid Port Address", "Port must be between 1 and 65535! "
                                                              "Please try again ...",QMessageBox::Ok);

            acceptedPort= false;
            ui->tcp_port->setText("");
        }

        if(acceptedIP && acceptedPort)
            accept();
    }
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
