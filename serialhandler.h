#ifndef SERIALHANDLER_H
#define SERIALHANDLER_H

#include <QRunnable>
#include <QSerialPort>
#include <QMutex>
#include <QEventLoop>
#include <QPlainTextEdit>


class serialHandler : public QObject
{
    Q_OBJECT
public:
    explicit serialHandler(QSerialPort *port, QPlainTextEdit *edit, QObject *parent = nullptr);

    ~serialHandler();

public slots:
    void run();
    void receiveData();
    void sendCommand(QString cmd);
    void sendPWM(QString pwm);

signals:
   void transmit(QString);

private:
    QSerialPort *port;
    QString buffer;
    QMutex m_mutex;
    QEventLoop loop;
    QPlainTextEdit edit;

};

#endif // SERIALHANDLER_H
