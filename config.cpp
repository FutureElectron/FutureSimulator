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
    return serialPort;
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
        serialPort = ui->portCombo->currentText();
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
        TCPActive = ui->setTCPActive->isChecked();
        QString IpRange = "(?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])";
        QRegularExpression IpRegex ("^" + IpRange
                                    + "(\\." + IpRange + ")"
                                    + "(\\." + IpRange + ")"
                                    + "(\\." + IpRange + ")$");
        QRegularExpressionValidator *ipValidator = new QRegularExpressionValidator(IpRegex, this);
        ui->ipaddr->setValidator(ipValidator);
        int pos{0};
        QString tmp_ipaddr = ui->ipaddr->text();


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
            acceptedPort = true;
        }
        else
        {
            QMessageBox::warning(this,"Invalid Port Address", "Port must be between 1 and 65535! "
                                                              "Please try again ...",QMessageBox::Ok);

            acceptedPort= false;
            ui->tcp_port->setText("");
        }

    }
    if (configValidation())
        accept();
}

void Config::setPort(const QString &newPort)
{
    serialPort = newPort;
    ui->portCombo->setCurrentText(newPort);
}

void Config::setIpaddr(const QString &newIpaddr)
{
    ipaddr = newIpaddr;
    ui->ipaddr->setText(ipaddr);
}

void Config::setBaudRate(const QString &newBaudRate)
{
    baudRate = newBaudRate;
    ui->baudrateCombo->setCurrentText(newBaudRate);
}

void Config::setDataBits(const QString &newDataBits)
{
    dataBits = newDataBits;
    ui->databitsCombo->setCurrentText(newDataBits);
}

void Config::setStopBits(const QString &newStopBits)
{
    stopBits = newStopBits;
    ui->stopbitsCombo->setCurrentText(newStopBits);
}

void Config::setParity(const QString &newParity)
{
    parity = newParity;
    ui->parityCombo->setCurrentText(newParity);
}

void Config::setFlowControl(const QString &newFlowControl)
{
    flowControl = newFlowControl;
    ui->flowcontrolCombo->setCurrentText(newFlowControl);
}

void Config::setOperatingMode(const QString &newOperatingMode)
{
    operatingMode = newOperatingMode;
    ui->modeCombo->setCurrentText(newOperatingMode);
}

void Config::setTCPActive(bool newTCPActive)
{
    TCPActive = newTCPActive;
    ui->setTCPActive->setChecked(newTCPActive);
}

void Config::setSerialActvie(const bool newSerialActvie)
{
    serialActvie = newSerialActvie;
    ui->setSerialActive->setChecked(newSerialActvie);
}

void Config::setTCPPort(const QString newTCPPort)
{
//    tcpPort = newTCPPort;
    ui->tcp_port->setText(newTCPPort);
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

bool Config::configValidation()
{
    // First check if any option is selected already and return if false
    if (!ui->setSerialActive->isChecked() && !serialActvie)
    {
        QMessageBox::warning(this,"Protocol Error", "No Protocol is selected. "
                                                    "Please select a vailid protocol to proceed...",QMessageBox::Ok);
        return false;
    }
    return true;
//    bool ipvalidation {false};
//    if (TCPActive)
//        if (acceptedIP && acceptedPort) ipvalidation = true;


//    if (ipvalidation && (TCPActive || serialActvie))
//        return true;
//    else
//        return false;
}

QString Config::getIpaddr() const
{

    return ipaddr;
}

quint16 Config::getTcpPort() const
{
    return tcpPort;
}
