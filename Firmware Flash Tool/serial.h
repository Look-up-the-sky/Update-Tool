#ifndef SERIAL_H
#define SERIAL_H
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QDebug>
#include <QMutex>


class cSerial :public QObject
{
    Q_OBJECT
public:
     explicit cSerial(QObject *parent = nullptr);
     unsigned long int CRC32( unsigned long int uCurCRC,  unsigned long int *pcData,  unsigned long int dwSize);
     unsigned int CRC16(unsigned char *ptr, unsigned int len);

signals:
     void send_messagebox(QString);
     void send_progressbar(unsigned int);

public slots:
     void doworks();
     void receive_com(QString str);
     void receive_baud(int index);
     void open_serial_port();
     void close_serial_port();
     void set_File_Update_array();


protected:
    QByteArray readbuf;
    QByteArray sendbuf;
    QByteArray Enable_IAP;
    QByteArray Enable_IAP_ACK;
    QByteArray Start_IAP;
    QByteArray Start_IAP_ACK;
    QByteArray File_Update;
    QByteArray File_Update_ACK;
    QByteArray Code_Update;
    QByteArray Code_Update_ACK;
    QByteArray SunnyBee_Version;
    QByteArray Version_ACK;
    QByteArray Hornet_Version;


private:
    void cSerial_init();
    void open_com();
    void close_com();
    QByteArray read_all();
    QString com;
    int baud;
public:
     bool stopped;
     QMutex m_mutex;
     QSerialPort *serial;
     QByteArray code_data;
public:
    QByteArray Soft_Version ;
    unsigned long int File_CRC = 0;
    unsigned long int Total_Len;
    unsigned int Last_package_len;
    unsigned int Total_Package = 0;
    unsigned int Package_Num = 0;
    int mcu = 0;
    bool Enable_IAP_flag;
    bool Start_IAP_flag;
    bool File_Update_flag;
    bool Code_Update_flag;
    bool Version_Ask_flag;




};



#endif // SERIAL_H
