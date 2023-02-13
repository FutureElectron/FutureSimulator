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

}

void Widget::signalSlotsConnections()
{
    connect(ui->horizontalSlider, &QSlider::sliderMoved, this, &Widget::sendSliderPWM);
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
    connect(ui->adjustTableBtn,&QPushButton::clicked, this, &Widget::adjustTable);
    connect(ui->resetLogginBtn,&QPushButton::clicked, this, &Widget::restartLogging);
    connect(ui->settingsBtn,&QToolButton::clicked,this,&Widget::showConfigDialog);
    connect(ui->connectBtn,&QToolButton::clicked,this, &Widget::connectSerial);
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
    connect(proc,&QProcess::readyRead, this, &Widget::processRead);
    connect(proc,&QProcess::readyReadStandardOutput, this, &Widget::processRead);
    connect(proc,&QProcess::readyReadStandardError, this, &Widget::processRead);

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


}

void Widget::connectSerial()
{
    static bool btnToggle = false;
    btnToggle = !btnToggle;

    QPixmap iconDisconnect {":/16x16/disconnect.png"};
    QIcon iDisconnect {iconDisconnect};
    serial.setPortName(port);
    portConfiguration();
    serial.open(QIODevice::ReadWrite);


    if(!serial.isOpen())
    {
        ui->connectionStatus->setStyleSheet("border-radius: 10px;background-color: rgb(255, 6, 31);");
        QMessageBox::information(this,"Connection Error","Connect to one of the "
                                                         "available ports by clicking on the settings button");
        connected = false;
        //return;
    }
    else{
        QPixmap iconConnect {":/16x16/connection.png"};
        QIcon iConnect(iconConnect);
        ui->connectBtn->setText("Disconnect");
        ui->connectBtn->setIcon(iConnect);
        ui->loggerBaseFrame->setEnabled(true);
        ui->homepageframe->setEnabled(true);
        ui->connectionStatus->setStyleSheet("border-radius: 10px;background-color: rgb(0, 255, 5);");
        QString val = "Connected to %1   Baud:%2 bps   Parity: %3   Stop Bits: %4   Mode: %5";
        val = val.arg(port,baudRate,parity,stopBits,opMode);
        ui->connectionLabel->setText(val);
        saveSettings();
        connected = true;
        qDebug()<<"connected to serial port";
    }


    if(connected && !btnToggle){
       qDebug()<<"Disconnected serial port";
      serial.close();
      ui->connectBtn->setText("Connect");
      ui->connectBtn->setIcon(iDisconnect);
      ui->loggerBaseFrame->setEnabled(false);
      ui->homepageframe->setEnabled(false);
      ui->connectionStatus->setStyleSheet("border-radius: 10px;background-color: rgb(255, 0, 5);");
      ui->connectionLabel->setText("Not connected. Check settings or click on the connect button");
    }
}

void Widget::restartLogging()
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

void Widget::adjustTable()
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

void Widget::portConfiguration()
{
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
    serial.write(cmd,qstrlen(cmd));
    ui->lineEdit->clear();
}

void Widget::saveSettings()
{
    QString filename = QCoreApplication::applicationDirPath() + "/settings.ini";
    QSettings settings(filename, QSettings::Format::IniFormat,this);

    settings.setValue("port",QVariant(port));
    settings.setValue("baudrate",QVariant(baudRate));
    settings.setValue("stopbits",QVariant(stopBits));
    settings.setValue("parity",QVariant(parity));
    settings.setValue("flowcontrol",QVariant(flowControl));
    settings.setValue("opmode", QVariant(opMode));
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

void Widget::sendSliderPWM()
{
    // send the horizontal slider value as a duty cycle to the controller
    QString val = "PWM **%";
    int sliderval = ui->horizontalSlider->value();
    val.replace("**", QString::number(sliderval*10));
    QByteArray pwm = ("4"+QString::number(sliderval)).toUtf8();
    //qDebug()<<pwm;
    if(serial.isOpen()) serial.write(pwm,qstrlen(pwm));
    ui->pwmLabel->setText(val);
}

void Widget::showConfigDialog()
{
    // Get the configuration parameters from the user
    Config *c = new Config(this);
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
}


void Widget::openFileInExplorer()
{
   QStringList args;
   args << "/open," << QDir::toNativeSeparators(logfolderPath+"/");
   QProcess *process = new QProcess(this);
   process->start("explorer.exe", args);
}

void Widget::processRead()
{
    qint64 value = 0;
    do
    {
        QByteArray line = proc->readAll();
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
        future = QtConcurrent::run(&Widget::receiveData, this);
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

void Widget::receiveData()
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

Widget::~Widget()
{
    if(proc->isOpen()) proc->close();
    delete ui;
    serial.close();
    logfile->close();
    delete logfile;
}

