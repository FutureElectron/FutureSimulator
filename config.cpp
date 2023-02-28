#include "config.h"
#include "ui_config.h"
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QMessageBox>
#include <QButtonGroup>
#include <QFile>

Config::Config(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Config)
{


    ui->setupUi(this);

    // fill the comport combo with available ports
    QSerialPortInfo serialInfo;

    QList<QSerialPortInfo> ports = serialInfo.availablePorts();
    QStringList stringPorts;
        for(int i = 0 ; i < ports.size() ; i++){
            stringPorts.append(ports.at(i).portName());
        }

    ui->portCombo->addItems(stringPorts);



    connect(ui->cancelBtn,&QPushButton::clicked,this, [=](){
        reject();
    });

    connect(ui->okBtn, &QPushButton::clicked,this, &Config::setvalues);



    connect(ui->useHostnameCheckBtn, &QCheckBox::stateChanged,this, &Config::hostNameCheckBoxStateChanged);
    connect(ui->protocolSelCombo, &QComboBox::currentTextChanged,this,[=](){
        if(ui->protocolSelCombo->currentText()=="Serial")
        {
            ui->stackedWidget->setCurrentIndex(0);
            serialActvie = true;
            TCPActive =false;
            UDPActive =false;
        }
        if(ui->protocolSelCombo->currentText()=="TCPIP")
        {
            ui->stackedWidget->setCurrentIndex(1);
            serialActvie = false;
            TCPActive =true;
            UDPActive =false;
        }
        if(ui->protocolSelCombo->currentText()=="UDP")
        {
            ui->stackedWidget->setCurrentIndex(1);
            serialActvie = false;
            TCPActive =false;
            UDPActive =true;
        }
    });

    // ensure that the default state of checkbox is checked
    //ui->useHostnameCheckBtn->setChecked(true);

    // set stylesheet
    //extract the settings and apply to application
    QString filename = ":/stylesheets/style/configDialogStyle.css";

    QFile style(filename);
    if(style.open(QIODevice::ReadOnly)){
        QString sheetStyle = style.readAll();
        style.close();
        setStyleSheet(sheetStyle);
    }

    // Validators
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

    if(serialActvie)
    {
        serialPort = ui->portCombo->currentText();
        baudRate = ui->baudrateCombo->currentText();
        dataBits = ui->baudrateCombo->currentText();
        stopBits = ui->stopbitsCombo->currentText();
        parity = ui->parityCombo->currentText();
        flowControl = ui->flowcontrolCombo->currentText();
        operatingMode = ui->modeCombo->currentText();
        ipaddr = ui->ipaddr->text();
        accept();
    }

    if (TCPActive || UDPActive)
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

        if (!ui->useHostnameCheckBtn->isChecked())

        {
            if (ipValidator->validate(tmp_ipaddr,pos) !=2 )
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
                useHostName = 0;
            }
        }
        else
        {
            ipaddr = ui->hostNameLEdit->text();
            acceptedIP = true; 
            useHostName = 1;
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
    if (acceptedIP && acceptedPort)
        accept();
}

bool Config::getUdpActive() const
{
    return UDPActive;
}

void Config::setUdpActive(bool newUdpActive)
{
    UDPActive = newUdpActive;
    if(newUdpActive)
    {
        ui->stackedWidget->setCurrentIndex(0);
        ui->protocolSelCombo->setCurrentText("UDP");
    }

}

void Config::hostNameCheckBoxStateChanged(int state)
{

    if (state == 2)
    {
        ui->ipaddr->setEnabled(false);
        ui->hostNameLEdit->setEnabled(true);
    }

    else
    {
        ui->ipaddr->setEnabled(true);
        ui->hostNameLEdit->setEnabled(false);
    }

}

int Config::getUseHostName() const
{
    return useHostName;
}

void Config::setUseHostName(int newUseHostName)
{
    useHostName = newUseHostName;
    ui->useHostnameCheckBtn->setChecked(newUseHostName);
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

void Config::setHostname(const QString &hostname)
{
    ipaddr = hostname;
    ui->hostNameLEdit->setText(hostname);
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
    if (newTCPActive)
    {
        ui->protocolSelCombo->setCurrentText("TCPIP");
        ui->stackedWidget->setCurrentIndex(1);
    }
}

void Config::setSerialActvie(const bool newSerialActvie)
{
    serialActvie = newSerialActvie;
    if (newSerialActvie)
    {
        ui->protocolSelCombo->setCurrentText("Serial");
        ui->stackedWidget->setCurrentIndex(0);
    }
}

void Config::setTCPPort(const QString newTCPPort)
{
    tcpPort = newTCPPort.toInt();
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


QString Config::getIpaddr() const
{

    return ipaddr;
}

quint16 Config::getTcpPort() const
{
    return tcpPort;
}
