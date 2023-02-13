#include "serialhandler.h"
#include "qdebug.h"

serialHandler::serialHandler(QSerialPort *port, QPlainTextEdit *edit,  QObject *parent)
        : QObject(parent), edit(edit)
{
    this->port = port;

    //connect receive slots
    connect(port, &QSerialPort::readyRead, this,&serialHandler::receiveData);
}

void serialHandler::run()
{
    qDebug()<<"Currently running thread. and waiting for data..";
//    port->waitForReadyRead();
    loop.exec();

    deleteLater();
}

serialHandler::~serialHandler()
{
    loop.quit();
    qDebug()<<"Leaving thread";
}


void serialHandler::receiveData()
{
    m_mutex.lock();
    QByteArray rawData = port->readAll();
    m_mutex.unlock();

    QString stringData{rawData};
    buffer.append(stringData);
    int index = buffer.indexOf("\n");

    if (index !=-1)
    {
        QString message = buffer.mid(0,index);
        qInfo()<<message;
        emit transmit(message);
        buffer.remove(0,index+1);
        edit.appendPlainText(message);
    }
//    rawData.clear();
}

void serialHandler::sendCommand(QString cmd)
{
    m_mutex.lock();
    port->write(cmd.toUtf8());
    qDebug()<<"Sending: "<<cmd;
    m_mutex.unlock();
}

void serialHandler::sendPWM(QString pwm)
{
    m_mutex.lock();
    QString valToEmit  = "4"+pwm;
    m_mutex.unlock();
    port->write(valToEmit.toUtf8(),qstrlen(valToEmit.toUtf8()));
}
