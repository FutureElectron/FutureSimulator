#include "widget.h"
#include "./ui_widget.h"
#include "config.h"
#include <QMessageBox>
#include <QDebug>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    signalSlotsConnections();
    QIcon icon(":/16x16/futureelectronics.jfif");
    ui->connectBtn->setEnabled(false);
    this->setWindowIcon(icon);
    setWindowTitle("Future Simulator");
    if (loadSettings()) ui->connectBtn->setEnabled(true);
    ui->loggerBaseFrame->setEnabled(false);
    ui->homepageframe->setEnabled(false);
    ui->stackedWidget->setCurrentIndex(0);
    ui->homeBtn->setStyleSheet(activeBtnStyle);
    ui->NormalSignalTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    if (lastConnectionState == "Serial") {serialActive=true; tcpActive=false; udpActive=false;}
    if (lastConnectionState == "TCP") {serialActive=false; tcpActive=true; udpActive=false;}
    if (lastConnectionState == "UDP") {serialActive=false; tcpActive=false; udpActive=true;}

    //TCP Server
    server = new QTcpServer(this);
    connect(server, SIGNAL(newConnection()), this, SLOT(serverHasNewConnection()));
    TCPServerConnection("127.0.0.1", 1234);
}

void Widget::signalSlotsConnections()
{
    connect(ui->horizontalSlider, &QSlider::sliderMoved, this, &Widget::sendPWMFromSlider);
    connect(&serial, &QSerialPort::readyRead, this, &Widget::readSerialData,  Qt::QueuedConnection);

    // ------------------------------Button connections-------------------------------------------//

    //--->>> Logging start buttons - Generate normal, ideal and abnormal signals
    connect(ui->sendRegularSignalBtn, &QToolButton::clicked, this, [=](){
       static bool status {false};
       status = !status;
       ui->NormalSignalTable->setEnabled(status);
       ui->repeatNormal->setEnabled(status);



       if(status)
       {
           //Check if the values should be saved for n-classification
           if(ui->saveForClassificationCheck->isChecked())
           {
               classSignal.pwm = abnormalPWMs;
               classSignal.timestamps = abnormalTimes;
               classSignal.basePWM = ui->normalBasePWM_2->value();
               sigVals[ui->NormalSignalFile->text()] = classSignal;
               //qDebug()<<classSignal.pwm;
           }

           ui->stepThreeCheck->clear();
           int ibasePWM = ui->normalBasePWM_2->value();
           QString sbasePWM = "4"+ QString::number(ibasePWM);
           if(serial.isOpen()) serial.write(sbasePWM.toUtf8(),qstrlen(sbasePWM.toUtf8()));

           NormalTimer.start(1000);
           if(ui->NormalSignalFile->text() =="")
               logfile = createLogFile("Regular_");
           else
               logfile = createLogFile(ui->NormalSignalFile->text());

           sendCommand("10");
           logging = true;
           qDebug()<<"Logging has been set to"<<logging;
           ui->normalGroupBox->setEnabled(false);
           ui->idealGroupBox->setEnabled(false);
       }
       else
       {
           NormalTimer.stop();
           logging =false;
       }
    });
    connect(ui->startIdealGenerationBtn, &QToolButton::clicked, this, [=](){
       static bool status {false};
       status = !status;
       if(status)
       {
           ui->stepTwoCheck->clear();
           int ibasePWM = ui->idealBasePWM->value();
           QString sbasePWM = "4"+ QString::number(ibasePWM);
           if(serial.isOpen()) serial.write(sbasePWM.toUtf8(),qstrlen(sbasePWM.toUtf8()));

           ui->startIdealGenerationBtn->setText("Stop");
           if(ui->idealFileName->text() =="")
               logfile = createLogFile("Ideal_");
           else
               logfile = createLogFile(ui->idealFileName->text());
           logging = true;
           sendCommand("10");
           idealTimer.start(1000);
           ui->normalGroupBox->setEnabled(false);
           ui->abnormalGroupBox->setEnabled(false);
       }
       else
       {
           idealTimer.stop();
           logging =false;
           ui->startIdealGenerationBtn->setText("Start");

       }
    });

    connect(ui->startSendAbnormalSignalBtn, &QToolButton::clicked, this, [=](){
       static bool status {false};
       status = !status;
       if(status)
       {
           ui->stepFourCheck->clear();
           int ibasePWM = ui->baseAnomalyPWM->value();
           QString sbasePWM = "4"+ QString::number(ibasePWM);
           if(serial.isOpen()) serial.write(sbasePWM.toUtf8(),qstrlen(sbasePWM.toUtf8()));

           ui->startSendAbnormalSignalBtn->setText("Stop");
           if(ui->abnormalSignalFile->text() =="")
               logfile = createLogFile("Anomaly_");
           else
               logfile = createLogFile(ui->abnormalSignalFile->text());
           logging = true;

           abnormalTimer.start(1000);
           sendCommand("10");
           ui->normalGroupBox->setEnabled(false);
           ui->idealGroupBox->setEnabled(false);
       }
       else
       {
           abnormalTimer.stop();
           logging =false;
           ui->startIdealGenerationBtn->setText("Start");

       }
    });
    // ------------------------------ End of logging buttons-------------------------------------//

    // --------------------------------Other control buttons------------------------------------//
    connect(ui->adjustTableBtn,&QPushButton::clicked, this, &Widget::uiAdjustTable);
    connect(ui->resetLogginBtn,&QPushButton::clicked, this, &Widget::uiRestartLogging);
    connect(ui->settingsBtn,&QToolButton::clicked,this,&Widget::showConfigDialog);
    connect(ui->connectBtn,&QToolButton::clicked,this, &Widget::connectionHandler);
    connect(ui->clearOutputBtn, &QToolButton::clicked, this, [=](){
        ui->plainTextEdit->clear();
    });
    connect(ui->disableOutputBtn, &QToolButton::clicked, this, [=](){
        enableOutput=!enableOutput;
        if (!enableOutput) ui->plainTextEdit->appendPlainText("Output Disabled");
    });
    connect(ui->learnCmdBtn, &QToolButton::clicked, this,&Widget::selectClassificationClass);
    connect(ui->inferCmdBtn, &QToolButton::clicked, this,[=](){
        sendCommand("30");
    });
    connect(ui->idleCmdBtn, &QToolButton::clicked, this,[=](){
        sendCommand("00");
        classificationTimer.stop();
    });
    connect(ui->logCmdBtn, &QToolButton::clicked, this,[=](){
        sendCommand("10");
    });
    connect(ui->stackedWidget, &QStackedWidget::currentChanged,this,[=](int index){
        if(index==0)
        {
            //qDebug()<<"current page:"<<index;

            ui->homeBtn->setStyleSheet(activeBtnStyle);
            ui->logBtn->setStyleSheet(inActiveBtnStyle);
        }
        if(index==1)
        {
            //qDebug()<<"current page:"<<index;

            ui->homeBtn->setStyleSheet(inActiveBtnStyle);
            ui->logBtn->setStyleSheet(activeBtnStyle);
        }
    });

    connect(ui->verbosityCmdBtn,&QPushButton::clicked, this,[=](){
        static bool verbose {false};
        verbose = !verbose;
        if (verbose) sendCommand("52");
        else sendCommand("50");

    });
    connect(ui->sendCmdBtn, &QToolButton::clicked,this, &Widget::sendCommandLine, Qt::QueuedConnection);
    connect(ui->homeBtn, &QToolButton::clicked, this, [=](){
       ui->stackedWidget->setCurrentIndex(0);
    });
    connect(ui->logBtn, &QToolButton::clicked, this, [=](){
       ui->stackedWidget->setCurrentIndex(1);

    });
    connect(ui->selectFolderButton, &QPushButton::clicked, this, [=](){
        logfolderPath = QFileDialog::getExistingDirectory(this,tr("Open Folder"),
                                                               "C:/Documents",QFileDialog::ShowDirsOnly);
        ui->folderPathLabel->setText(logfolderPath);
        if(logfolderPath != "") setCheckBtn(ui->stepOneCheck);
    });
    connect(ui->repeatNormal, &QToolButton::clicked, this, [=](){
       NormalSignalGenerator();
    });

    connect(ui->classificationBtn, &QToolButton::clicked, this, &Widget::classification);

    connect(ui->fileFolders, &QToolButton::clicked, this, [=](){
       openFileInExplorer();
    });

    connect(ui->documentationBtn, &QToolButton::clicked, this, [=](){
        QString filename = QDir::currentPath() + QDir::separator() + "Documentation.pdf";
        QDesktopServices::openUrl(QUrl::fromLocalFile(QDir::toNativeSeparators
                                                      (filename)));
        //  R"(C:\Users\Stanley.Ezeh\Documents\STM32MP1 AI project\Nano\16x16\Documentation.pdf)"))
    });

    connect(ui->logfileBtn, &QToolButton::clicked, this, [=](){
        QString filepath = QDir::currentPath() + QDir::separator() + "log.txt";
        QStringList args;
        args << QDir::toNativeSeparators(filepath);
        //qDebug()<<filepath;
        QProcess *process = new QProcess(this);
        process->start("notepad.exe", args);
    });

    // ------------------------------End of Button connections-------------------------------------//

    //--->>> Processes
    connect(openExplorerProcess,&QProcess::readyRead, this, &Widget::processRead);
    connect(openExplorerProcess,&QProcess::readyReadStandardOutput, this, &Widget::processRead);
    connect(openExplorerProcess,&QProcess::readyReadStandardError, this, &Widget::processRead);

    //--------------------------------- Logging timers---------------------------------------------//
    connect(&NormalTimer, &QTimer::timeout,this,[=](){
        static int seconds {0}, mins{0};
        sendNormalPWMs(seconds);
        seconds++;
        ui->timerLabel->setText(QString::number(seconds));

        if (seconds == logtime)
        {
            seconds=0;
            mins += 1;
            pIndex = 0;

        }
        if (mins==2)
        {
            NormalTimer.stop();
            //sendCommand("00");
            setCheckBtn(ui->stepThreeCheck);
            if(logfile->isOpen())logfile->close();
            ui->normalGroupBox->setEnabled(true);
            ui->idealGroupBox->setEnabled(true);
            mins =0;
            ui->timerLabel->clear();
        }
    });

    connect(&classificationTimer, &QTimer::timeout,this,[=](){
        static int seconds {0};
        sendClassificationPattern(seconds);
        seconds++;

        if (seconds == logtime)
        {
            seconds=0;
            pIndex = 0;
        }
    });

    connect(&idealTimer, &QTimer::timeout,this,[=](){
        static int seconds {0}, mins{1};
        idealSignalGenerator();

        ui->idealSignalProgressBar->setMaximum(logtime*2);
        ui->idealSignalProgressBar->setValue(seconds*mins);
        seconds++;
        ui->timerLabel->setText(QString::number(seconds));
        if (seconds == logtime)
        {
            seconds=0;
            mins +=1;
        }
        if(mins==3)
        {
            idealTimer.stop();
            sendCommand("00");
            setCheckBtn(ui->stepTwoCheck);
            ui->idealSignalProgressBar->setValue(seconds);
            ui->startIdealGenerationBtn->setText("Start");
            ui->normalGroupBox->setEnabled(true);
            ui->abnormalGroupBox->setEnabled(true);
            if(logfile->isOpen())logfile->close();
            mins =0;
            ui->timerLabel->clear();
        }
    });
    connect(&abnormalTimer, &QTimer::timeout,this,[=](){
        static int seconds {0}, mins{1};
        abnormalSignalGenerator(seconds);
        ui->startSendAbnormalSignalBtn->setText("Stop");
        ui->abnormalSignalProgressBar->setMaximum(logtime);
        ui->abnormalSignalProgressBar->setValue(seconds);
        seconds++;
        ui->timerLabel->setText(QString::number(seconds));
        if (seconds == logtime)
        {
            seconds=0;
            mins +=1;
        }

        if(mins==3){
            abnormalTimer.stop();
            sendCommand("00");
            setCheckBtn(ui->stepFourCheck);
            ui->abnormalSignalProgressBar->setValue(seconds);
            ui->startSendAbnormalSignalBtn->setText("Start");
            ui->abnormalGroupBox->setEnabled(true);
            ui->idealGroupBox->setEnabled(true);
            if(logfile->isOpen())logfile->close();
            mins =0;
            ui->timerLabel->clear();
        }
    });

    //--------------------------------- End of logging timers---------------------------------------//

    // ---------------------------------TCP/IP Signals and slots ----------------------------------//
    connect(&TCPSocket,&QTcpSocket::connected,this,&Widget::TCPConnectionSuccessful);
    connect(&TCPSocket,&QTcpSocket::disconnected,this,&Widget::TCPdisconnected);
    connect(&TCPSocket,&QTcpSocket::stateChanged,this,&Widget::TCPStateChanged);
    connect(&TCPSocket,&QTcpSocket::readyRead,this,&Widget::TCPReceiveData);
    connect(&TCPSocket,&QTcpSocket::errorOccurred,this,&Widget::TCPConnectionError);

    //-------------------------------------UDP Connections----------------------------------------//
    connect(&UDPSocket,&QUdpSocket::readyRead,this,&Widget::UDPreadyRead);

}

void Widget::uiRestartLogging()
{
    ui->folderPathLabel->clear();
    ui->stepOneCheck->clear();
    ui->stepTwoCheck->clear();
    ui->stepThreeCheck->clear();
    ui->stepFourCheck->clear();
    ui->idealBasePWM->setValue(0);
    ui->baseAnomalyPWM->setValue(0);
    ui->normalBasePWM_2->setValue(0);
    ui->NormalSignalTable->clear();
}

void Widget::uiAdjustTable()
{
    int numberOfSpuriousSignals = ui->numberAbSpuriousSignal->value();
    int currentNumberOfColumns = ui->NormalSignalTable->columnCount();

    if(numberOfSpuriousSignals > currentNumberOfColumns)
        for (int col{currentNumberOfColumns};col<numberOfSpuriousSignals;col++)
            ui->NormalSignalTable->insertColumn(col);
    else
    {
        int c{currentNumberOfColumns};
        while(ui->NormalSignalTable->columnCount() > numberOfSpuriousSignals)
        {
            ui->NormalSignalTable->removeColumn(c);
            c--;
        }
        ui->NormalSignalTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }

}

void Widget::serialPortConfiguration()
{
    // set port name
    serial.setPortName(port);

    // databits
    if(dataBits == "5 Bits") {
           serial.setDataBits(QSerialPort::Data5);
    }
    else if((dataBits == "6 Bits")) {
       serial.setDataBits(QSerialPort::Data6);
    }
    else if(dataBits == "7 Bits") {
       serial.setDataBits(QSerialPort::Data7);
    }
    else if(dataBits == "8 Bits"){
       serial.setDataBits(QSerialPort::Data8);
        }

    //baudrate
    serial.setBaudRate(baudRate.toLong());

    //-> Stop bits
    if(stopBits == "1 Bit") {
     serial.setStopBits(QSerialPort::OneStop);
    }
    else if(stopBits == "1,5 Bits") {
     serial.setStopBits(QSerialPort::OneAndHalfStop);
    }
    else if(stopBits == "2 Bits") {
     serial.setStopBits(QSerialPort::TwoStop);
    }
    //-> parity
    if(parity == "No Parity"){
      serial.setParity(QSerialPort::NoParity);
    }
    else if(parity == "Even Parity"){
      serial.setParity(QSerialPort::EvenParity);
    }
    else if(parity == "Odd Parity"){
      serial.setParity(QSerialPort::OddParity);
    }
    else if(parity == "Mark Parity"){
      serial.setParity(QSerialPort::MarkParity);
    }
    else if(parity == "Space Parity") {
        serial.setParity(QSerialPort::SpaceParity);
    }

    //-> flow control
    if(flowControl == "No Flow Control") {
      serial.setFlowControl(QSerialPort::NoFlowControl);
    }
    else if(flowControl == "Hardware Flow Control") {
      serial.setFlowControl(QSerialPort::HardwareControl);
    }
    else if(flowControl == "Software Flow Control") {
      serial.setFlowControl(QSerialPort::SoftwareControl);
    }
}

void Widget::sendCommandLine()
{
    QByteArray cmd = ui->lineEdit->text().toUtf8();
    if (serialActive) serial.write(cmd,qstrlen(cmd));
    if (udpActive) {
       QNetworkDatagram datagram(cmd,QHostAddress::Broadcast,1234);
       //QNetworkDatagram datagram(cmd,QHostAddress(ipaddress),tcpPort);
       qInfo() << "Sending: " << cmd << "via UDP";
       UDPSocket.writeDatagram(datagram);
    }
    if (tcpActive) {
        TCPSocket.write(cmd);
        qInfo() << "Sending: " << cmd << "via TCP";
    }
    ui->lineEdit->clear();
}

void Widget::saveSettings()
{
    QString filename = QCoreApplication::applicationDirPath() + "/settings.ini";
    qInfo()<<filename;
    QSettings settings(filename, QSettings::Format::IniFormat,this);

    settings.setValue("port",QVariant(port));
    settings.setValue("baudrate",QVariant(baudRate));
    settings.setValue("stopbits",QVariant(stopBits));
    settings.setValue("parity",QVariant(parity));
    settings.setValue("flowcontrol",QVariant(flowControl));
    settings.setValue("opmode", QVariant(opMode));
    settings.setValue("lastConnectionState", QVariant(lastConnectionState));
    settings.setValue("tcpPort", QVariant(tcpPort));
    settings.setValue("ipadrress", QVariant(ipaddress));
    settings.setValue("useHostAddress", QVariant(useHostName));

    int result = settings.status();
    if (result ==QSettings::NoError && !settingsSaved )
        settingsSaved = true;
}

bool Widget::loadSettings()
{
    /*
        Checks if there are any settings saved already.
        returns: True if at five values are saved setting is saved;
    */
    QMap<QString, QString> returnValue;
    QString filename = QCoreApplication::applicationDirPath() + "/settings.ini";
    QSettings set(filename, QSettings::Format::IniFormat,this);
    // retrieve the keys
     QStringList keys=set.allKeys();
     if (keys.size()<5)
         return false;
     foreach(auto key,keys)
         returnValue[key]=set.value(key).toString();
     port = returnValue["port"];
     baudRate = returnValue["baudrate"];
     stopBits = returnValue["stopbits"];
     parity = returnValue["parity"];
     flowControl = returnValue["flowcontrol"];
     opMode = returnValue["opmode"];
     lastConnectionState = returnValue["lastConnectionState"];
     tcpPort = returnValue["tcpPort"].toInt();
     ipaddress = returnValue["ipadrress"];
     useHostName = returnValue["useHostAddress"].toInt();
     //qInfo()<<lastConnectionState;
     if(opMode=="Anomaly Detection")
     {
         ui->classificationBtn->setEnabled(false);
         ui->learnCmdBtn->setEnabled(true);
         ui->inferCmdBtn->setEnabled(true);
     }
     else
     {
        ui->classificationBtn->setEnabled(true);
        ui->learnCmdBtn->setEnabled(false);
        ui->inferCmdBtn->setEnabled(false);
     }

     return true;

}

void Widget::NormalSignalGenerator()
{

    /*
        Generate a random array of irregular PWMs between 1 and 9
        Each PWM will be sent at a specific randomly generated time
        Time values are unique and sorted in ascending order to ensure
        that at each increasing time, t+tn, a new pwm is transmitted
        in the function sendAbnormal PWMs.

        Function sets both the instance attributes
        abnormalPWMs and abnormalTimes to the randomly generated values.
    */

    QList<int> k,v;
    QSet<int>set;

    while(set.size()<ui->NormalSignalTable->columnCount())
        set.insert(QRandomGenerator::global()->bounded(logtime));

    k = QList(set.begin(),set.end());
    std::sort(k.begin(),k.end());

    while(v.size()<ui->NormalSignalTable->columnCount())
        v.append(QRandomGenerator::global()->bounded(9));


    abnormalPWMs = v;
    abnormalTimes = k;

    // update the table rows
    for (int col{0}; col<ui->NormalSignalTable->columnCount();col++)
    {
        QTableWidgetItem *val = new QTableWidgetItem();
        QVariant temp (QString::number(k.at(col))+"s");
        val->setData(Qt::DisplayRole,temp);
        ui->NormalSignalTable->setItem(0,col, val);
    }

    for (int col{0}; col<ui->NormalSignalTable->columnCount();col++)
    {
        QTableWidgetItem *val = new QTableWidgetItem();
        QVariant temp (QString::number(v.at(col)*10)+"%");
        val->setData(Qt::DisplayRole,temp);
        ui->NormalSignalTable->setItem(1,col, val);
    }
}

void Widget::sendNormalPWMs(int seconds)
{
    /*
     * @PARAMS:
     *        n : current value of the seconds variable. see Lambda connection for
     *        abnormalTimer for static declaration.
     *
     * @DESCRIPTION
              Increment the index, pIndex of the abnormalPWMs list if the time in seconds is found
              Select the current cell containing the value sent.
    */
    //Capture the base PWM
    int ibasePWM = ui->normalBasePWM_2->value();
    QString sbasePWM = "4"+ QString::number(ibasePWM);
    sendCommand(sbasePWM);

    if(abnormalTimes.contains(seconds))
    {

        QString pwm = "4"+ QString::number(abnormalPWMs.at(pIndex));
        qDebug()<<"sending Normal pwm: "<<pwm;
        if(serial.isOpen()) serial.write(pwm.toUtf8(),qstrlen(pwm.toUtf8()));
        ui->NormalSignalTable->selectColumn(pIndex);
        pIndex++;
        //qDebug()<<"n:"<<seconds;

        // reset to base signal after 500 miliseconds
        QTimer::singleShot(500, this, [=](){
            //qDebug()<<"returning to base Ideal pwm: "<<sbasePWM;
            sendCommand(sbasePWM);
        });
    }


}

void Widget::setCheckBtn(QLabel *obj)
{
    /*
        Sets the check icon to the label
    */
    QPixmap icon {":/16x16/check-mark_1.png"};
    obj->setPixmap(icon);
}

QFile *Widget::createLogFile(QString type)
{
    /*
        @PARAMS
            QString type: type of log file to create
        @DES
            Returns a pointer to a new file handle created and opened for storing logs
            type is appended to name of the file to distinguish log type
    */
    QString currentDate = QDate::currentDate().toString(Qt::ISODate);
    QString currentTime = QTime::currentTime().toString("hh_mm");
    QString filename = logfolderPath+"/"+type+"log_"+currentDate+"_"+currentTime+".csv";

    qInfo()<<"Created: "+filename;

    QFile *file=new QFile (filename);

    if(!file->open(QIODevice::WriteOnly)) {
            QMessageBox::warning(this,"Warning","Unable to create log file. "
                                                "Signal generated will not be logged!",
                                 QMessageBox::StandardButton::Close);

    }

    //  set the state of the microcontroller to log state
    QByteArray cmd {"10"};
    serial.write(cmd,qstrlen(cmd));

    return file;
}

void Widget::idealSignalGenerator()
{
    /*
        @PARAMS
                none;
        @DES
            Sends the base pwm to the MCU
    */
    QByteArray cmd {"10"};
    if(serial.isOpen()) serial.write(cmd,qstrlen(cmd));
    int val = ui->idealBasePWM->value();
    QString pwm = "4"+ QString::number(val);
    //qDebug()<<"sending base Ideal pwm: "<<pwm;
    if(serial.isOpen()) serial.write(pwm.toUtf8(),qstrlen(pwm.toUtf8()));
}

void Widget::abnormalSignalGenerator(int seconds)
{
    /*
        @PARAMS
            QString seconds: Current value of the static variable, seconds in the
            lambda function normalTimer
        @DES
            Generate a random PWM dutycycle at random times to be sent to the MCU.After
            one second, the disturbance (spurious signal) clears and the dutycycle returns
            to base pwm.
    */
    int ibasePWM = ui->baseAnomalyPWM->value();
    QString sbasePWM = "4"+ QString::number(ibasePWM);
    if(serial.isOpen()) serial.write(sbasePWM.toUtf8(),qstrlen(sbasePWM.toUtf8()));

    // Generate and send a different anomaly signal each time its called
    int rand = QRandomGenerator::global()->bounded(2,9);
    QString pwm = "4"+ QString::number(rand);
    qDebug()<<"sending spurious pwm: "<<pwm;
    if(serial.isOpen()) serial.write(pwm.toUtf8(),qstrlen(pwm.toUtf8()));

    // reset to base signal after 500 miliseconds
    QTimer::singleShot(500, this, [=](){
        //qDebug()<<"returning to base Ideal pwm: "<<sbasePWM;
        if(serial.isOpen()) serial.write(sbasePWM.toUtf8(),qstrlen(sbasePWM.toUtf8()));
    });

}

void Widget::sendPWMFromSlider()
{
    // send the horizontal slider value as a duty cycle to the controller
    QString val = "PWM **%";
    int sliderval = ui->horizontalSlider->value();
    val.replace("**", QString::number(sliderval*10));
    QByteArray pwm = ("4"+QString::number(sliderval)).toUtf8();
    //qDebug()<<pwm;

    if (serialActive) serial.write(pwm,qstrlen(pwm));
    if (udpActive) {
       QNetworkDatagram datagram(pwm,QHostAddress::Broadcast,1234);
       //QNetworkDatagram datagram(cmd,QHostAddress(ipaddress),tcpPort);
       qInfo() << "Sending: " << pwm << "via UDP";
       UDPSocket.writeDatagram(datagram);
    }
    if (tcpActive) {
        TCPSocket.write(pwm);
        qInfo() << "Sending: " << pwm << "via TCP";
    }
    ui->pwmLabel->setText(val);
}

void Widget::showConfigDialog()
{
    // Get the configuration parameters from the user
    Config *c = new Config(this);

    c->setUseHostName(useHostName);
    if (useHostName ==1) c->setHostname(ipaddress); else c->setIpaddr(ipaddress);
    c->setPort(port);
    c->setBaudRate(baudRate);
    c->setDataBits(dataBits);
    c->setStopBits(stopBits);
    c->setFlowControl(flowControl);
    c->setOperatingMode(opMode);
    c->setSerialActvie(serialActive);
    c->setTCPActive(tcpActive);
    c->setParity(parity);
    c->setTCPPort(QString::number(tcpPort));
    c->setUdpActive(udpActive);
    int ret = c->exec();
    if(ret==QDialog::Accepted)
    {
        port = c->getPort();
        baudRate = c->getBaudRate();
        dataBits = c->getDataBits();
        stopBits = c->getStopBits();
        parity = c->getParity();
        flowControl = c->getFlowControl();
        opMode = c->getOpMode();
        ipaddress = c->getIpaddr();
        tcpPort = c->getTcpPort();
        tcpActive = c->getTCPActive();
        qInfo()<<"tcp: "<<tcpActive;
        qInfo() <<"serial: "<<serialActive;
        serialActive = c->getSerialActvie();
        tcpActive = c->getTCPActive();
        udpActive = c->getUdpActive();
        useHostName = c->getUseHostName();
        //Enable the connect button now
        ui->connectBtn->setEnabled(true);
    }
    else
        QMessageBox::warning(this,"COMM Port Settings","Settings not saved");

    if(opMode == "n-Classification")
    {
        ui->learnCmdBtn->setEnabled(false);
        ui->inferCmdBtn->setEnabled(false);
        ui->classificationBtn->setEnabled(true);
    }
    if(opMode == "Anomaly Detection")
    {
        ui->classificationBtn->setEnabled(false);
        ui->learnCmdBtn->setEnabled(true);
        ui->inferCmdBtn->setEnabled(true);
    }
    if(tcpActive || udpActive)
    {
        ui->frame_10->setEnabled(false);
    }
    else
        ui->frame_10->setEnabled(true);
}

void Widget::openFileInExplorer()
{
   QStringList args;
   args << "/open," << QDir::toNativeSeparators(logfolderPath+"/");
   openExplorerProcess = new QProcess(this);
   openExplorerProcess->start("explorer.exe", args);
}

void Widget::processRead()
{
    qint64 value = 0;
    do
    {
        QByteArray line = openExplorerProcess->readAll();
        qInfo() << line.trimmed();
        value = line.length();
    } while(value > 0);
}

void Widget::selectClassificationClass()
{
    QMessageBox::StandardButton reply;

    // User can use the already saved pattern to train the generated model from Nano.
    // If reply is YES, then the NormalTimer is started which in turn calls the sendNormalPWM function
    if (!abnormalPWMs.isEmpty())
    {
        reply = QMessageBox::information(this, "Model Relearning","Logged Model available. Do you want to use this ?",QMessageBox::Yes|QMessageBox::No);
        if(reply==QMessageBox::Yes)
            NormalTimer.start(1000);
    }

    QString cmd = "20";
    QByteArray c = cmd.toUtf8();
    if (serial.isOpen()) serial.write(c, qstrlen(c));
    //qDebug()<<c;
    return ;
}

void Widget::sendCommand(QString cmd)
{
    QByteArray c = cmd.toUtf8();
    qDebug()<<"sending command: "<<cmd;
    if (serial.isOpen()) serial.write(c, qstrlen(c));
}

void Widget::sendClassificationPattern(int seconds)
{
    //qDebug()<<selectedClassSignal.pwm;
    if(selectedClassSignal.timestamps.contains(seconds))
    {
        QString pwm = "4"+ QString::number(selectedClassSignal.pwm.at(pIndex));
        qDebug()<<"sending pwm: "<<pwm;
        if(serial.isOpen()) serial.write(pwm.toUtf8(),qstrlen(pwm.toUtf8()));

        // do not increment the pIndex value if it is greater than or equal to the
        // length of the PWM array. -Introduced to avoid out of range error
        if(pIndex < selectedClassSignal.pwm.length()) pIndex++;
        else pIndex=0;

        //qDebug()<<"n:"<<seconds;

        // reset to base signal after 500 miliseconds
        QTimer::singleShot(500, this, [=](){
            //qDebug()<<"returning to base Ideal pwm: "<<selectedClassSignal.basePWM;
            sendCommand("4"+QString::number(selectedClassSignal.basePWM));
        });
    }
}

void Widget::readSerialData()
{

    if(!future.isRunning())
        future = QtConcurrent::run(&Widget::serialReceiveData, this);
    future.waitForFinished();
}

void Widget::classification()
{
    QStringList items;

    if(!sigVals.isEmpty())
    {
        foreach(auto item, sigVals.keys())
            items.append(item);
    }

    bool ok;
    QString item = QInputDialog::getItem(this, tr("Select Class Signal"),
                                         tr("Class:"), items, 0, false, &ok);
    if (ok && !item.isEmpty())
        selectedClassSignal = sigVals[item];
    sendCommand("60");
    if(!selectedClassSignal.pwm.isEmpty()) classificationTimer.start(1000);

}

void Widget::TCPConnectToHost(QString host, quint16 port)
{
    // modify GUI file before doing this
    if(TCPSocket.isOpen()) TCPSocket.close();
    qInfo() << "Connecting to: " << host << " on port " << port;
    TCPSocket.connectToHost(host,port);
}

void Widget::TCPDisconnect()
{

    if (TCPSocket.isOpen())
    {
        TCPSocket.close();
        TCPSocket.waitForDisconnected();
    }
}

void Widget::UDPreadyRead()
{
    while(UDPSocket.hasPendingDatagrams())
    {
        QNetworkDatagram datagram = UDPSocket.receiveDatagram();
        qInfo() << "Received via UDP: " << datagram.data() << " from " << datagram.senderAddress() << ":" << datagram.senderPort();
        ui->plainTextEdit->appendPlainText(datagram.data());
    }
}

void Widget::TCPConnectionSuccessful()
{
    qInfo()<<"Connected to "<<ipaddress<<"on port: "<<tcpPort;
    TCPConnected = true;
    QString msg = QString("TCP Connection established to %1"
                          " and port: %2").arg(ipaddress).arg(TCPSocket.peerPort());
    lastConnectionState="TCP";


    uiConnectionSuccessful(msg);
}

void Widget::TCPdisconnected()
{
    TCPConnected = false;
    qInfo()<<"TCP Disconnected";
    uiDisconnectionSuccessful("TCP disconnected. Configure valid connection or click on the connect button");
}

void Widget::TCPConnectionError(QAbstractSocket::SocketError socketError)
{
    qWarning() << "Error:" <<" " << TCPSocket.errorString();
    TCPConnected = false;
    ui->connectionLabel->setText(QString("Error: %1 ").arg(TCPSocket.errorString()));
}

void Widget::TCPStateChanged(QAbstractSocket::SocketState socketState)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QAbstractSocket::SocketState>();
    qInfo() << "State:" << metaEnum.valueToKey(socketState);
    ui->connectionLabel->setText(QString("State: %1").arg(metaEnum.valueToKey(socketState)));
}

void Widget::TCPReceiveData()
{
    //Will only be used if we want to receive a feedback
    ui->plainTextEdit->setPlainText(TCPSocket.readAll());
}

void Widget::connectionHandler()
{
    static bool attemptToConnect{false};

    // only toggle when there has been a previous successful connection
    if (TCPConnected || serialConnected || UDPConnected)
        attemptToConnect = !attemptToConnect;
    else attemptToConnect = true;

    if(attemptToConnect)
    {
        if (tcpActive)
        {
            /*
             * TCP server has to listen first
            TCP Connection is asynchronous and uses signals and slots so there is no need for 
            deciding to update UI or handle connection parameters. Check TCPConnected or TCPDisconnected Slots
            as these handle events regarding change of states.
            */
            //Listen on the specified port

            TCPConnectToHost(ipaddress,tcpPort);
            //check for successful connection

        }
        else if(udpActive)
        {
            QString msg= "UDP socket is bound to port %1. Heartbeat signals will be sent to host confirm alive";
            QMessageBox::information(this,"Connection Message", msg);
            if (!UDPSocket.bind(QHostAddress::LocalHost,1234))
            {
                QMessageBox::information(this,"Connection Error","Failed to bind UDP socket to port. Please try again");
                UDPConnected =false;

            }
            else{
            uiConnectionSuccessful(msg.arg(UDPSocket.peerPort()));
            lastConnectionState = "UDP";
            saveSettings();
            UDPConnected = true;
            // to implement heartbeat --- timebased
            }
        }

        // serial connection
        else if (serialActive)
        {
            serialPortConfiguration();
            serial.open(QIODevice::ReadWrite);
            if(serial.isOpen())
            {
                serialConnected = true;
                QString msg = "Connected to %1   Baudrate:%2 bps Datbits:%6  Parity: %3   Stop Bits: %4   Mode: %5";
                msg = msg.arg(port,baudRate,parity,stopBits,opMode,dataBits);
                uiConnectionSuccessful(msg);
                lastConnectionState = "Serial";
                saveSettings();
            }
            else
            {
                serialConnected = false;
                QMessageBox::information(this,"Connection Error","Connect to one of the "
                                                                 "available ports or Host by clicking on the settings button");
                uiDisconnectionSuccessful("Not connected. Click on the settings button to configure connection");
            }
        }

        // Fresh run, no configuration has been done in the past
        if(lastConnectionState =="")
        {
            QMessageBox::information(this,"Connection Error","Connect to one of the "
                                                             "available ports or Host by clicking on the settings button");
        }


    }
    else
    {
        // Disconnect all open connections
        if(serialConnected) serial.close();
        if(TCPConnected) TCPDisconnect();
        if(UDPConnected) {UDPSocket.close(); UDPConnected=false;}
        uiDisconnectionSuccessful("No active Connection. Click on Settings to configure");
    }
}

void Widget::serverHasNewConnection()
{
    //Getting Client Connection
    QTcpSocket *soc = server->nextPendingConnection();
    soc->setObjectName("NewSocket");
    //Connect the signal slot of the QTcpSocket to read the new data
    QObject::connect(soc, &QTcpSocket::readyRead, this, [=](){TCPServerHasNewData(soc);});
    QObject::connect(soc, &QTcpSocket::disconnected, this, [=](){TCPServerSocketLostClient(soc);});
    soc->write("Hello client, you just connected to me \r\n");
    soc->flush();
    soc->waitForBytesWritten(3000);
}

void Widget::TCPServerHasNewData(QTcpSocket *newSereverSocket)
{
    QByteArray data = newSereverSocket->readAll();
    ui->plainTextEdit->appendPlainText(data);
    qInfo()<<"Received data from"<<newSereverSocket->objectName();
}

void Widget::TCPServerSocketLostClient(QTcpSocket *newSereverSocket)
{

//     QTcpSocket *soc = static_cast<QTcpSocket *>(sender());
//     qInfo()<<"Closeing connection with"<<soc->objectName();
     qInfo()<<"Closeing connection with"<<newSereverSocket->objectName();
     newSereverSocket->close();//
}

void Widget::uiConnectionSuccessful(QString msg)
{
    QPixmap iconConnect {":/16x16/connection.png"};
    QIcon iConnect(iconConnect);
    ui->connectBtn->setText("Disconnect");
    ui->connectBtn->setIcon(iConnect);
    ui->loggerBaseFrame->setEnabled(true);
    ui->homepageframe->setEnabled(true);
    ui->connectionStatus->setStyleSheet("border-radius: 10px;background-color: rgb(0, 255, 5);");
    qDebug()<<"Connected " + lastConnectionState + " port";
    ui->connectionLabel->setText(msg);
    saveSettings();
}

void Widget::uiDisconnectionSuccessful(QString msg)
{
    QPixmap iconDisconnect {":/16x16/disconnect.png"};
    QIcon iDisconnect {iconDisconnect};
    qDebug()<<"Disconnected " +lastConnectionState + " port";
    ui->connectBtn->setText("Connect");
    ui->connectBtn->setIcon(iDisconnect);
    ui->loggerBaseFrame->setEnabled(false);
    ui->homepageframe->setEnabled(false);
    ui->connectionStatus->setStyleSheet("border-radius: 10px;background-color: rgb(255, 0, 5);");
    ui->connectionLabel->setText("Not connected. Check settings or click on the connect button");
}

void Widget::serialReceiveData()
{
    QByteArray rawData = serial.readAll();


    QString stringData{rawData};
    buffer.append(stringData);
    int index = buffer.indexOf("\r\n");
    bool ok;
    if (index !=-1)
    {
        mes = buffer.mid(0,index+2);
        buffer.remove(0,index+2);

        //added to avoid additional signals
        QString currentMessage = "*** % Similarity";
        if(mes.length()>0)
        {
            if(logging && logfile->isOpen()) logfile->write(mes.toUtf8());
            mes.chop(2);
            if (enableOutput) ui->plainTextEdit->appendPlainText(mes);        

            if(mes.at(0)=='S')
            {

                int percent = mes.toInt(&ok);
                currentMessage.replace("***",mes);
                ui->similarityLabel->setText(currentMessage);
                if (ok)
                {
                    if(percent<50) ui->similarityLabel->setStyleSheet("background-color: red");
                    else ui->similarityLabel->setStyleSheet("background-color: transparent");
                }

            }

            // Update the learning progressbar if L is found in the string
            if(mes.at(0)=='l'){

                bool ok;
                int currentLeanringVal=0;

                mes.remove(0,2);
                //qDebug()<<mes;
                //qDebug() << "Message length :"  << mes.length();
                mes.length() <5? currentLeanringVal = mes.first(1).toInt(&ok) : currentLeanringVal = mes.first(2).toInt(&ok);
                int totalLearning = mes.last(2).toInt(&ok);
                //qDebug()<<totalLearning;

                if(ok) {
                    ui->progressBar->setValue((currentLeanringVal));
                    ui->progressBar->setMaximum(totalLearning);
                }
            }

            if (mes.at(0)=='C')
            {
                ui->similarityLabel->setText(mes);
            }
        }

    }
}

void Widget::TCPServerConnection(QString host, quint16 port)
{

    QHostAddress addr;
    addr.setAddress(host);
    if(!server->listen(QHostAddress::LocalHost, tcpPort))
    {
        //If an error occurs, the error message is output
        qWarning()<<"Server error when listening  to "+host+":"<<server->errorString();
        return;
    }
    else
      {
        qInfo()<<"Server Started";
      }
}

Widget::~Widget()
{
    if(openExplorerProcess->isOpen()) openExplorerProcess->close();
    delete ui;
    serial.close();
    TCPSocket.close();
    UDPSocket.close();
    logfile->close();
    delete logfile;
}

