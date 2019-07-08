#include "serial.h"
#include <windows.h>

cSerial::cSerial(QObject *parent) : QObject(parent)
{
    cSerial_init();

    stopped = false;

}

 unsigned long int cSerial::CRC32( unsigned long int uCurCRC,  unsigned long int *pcData,  unsigned long int dwSize)
{
     unsigned long int bits;
     unsigned long int xbits;
     unsigned long int data;
     unsigned long int CRC = uCurCRC;
    while (dwSize--)
    {
        xbits = ((unsigned long int)1<<31);
        data = *pcData++;
        for(bits = 0; bits < 32;bits++)
        {
            if(CRC & 0x80000000)
            {
                CRC <<= 1;
                CRC^= 0x04c11db7;
            }
            else
                CRC <<= 1;
            if(data & xbits)
                CRC^= 0x04c11db7;
            xbits >>= 1;

        }
    }
    return CRC;
}

unsigned int cSerial::CRC16(unsigned char *ptr, unsigned int len)
{
    unsigned char j, CRClen;
    unsigned int CRCdata;
    CRCdata = 0xffff;
    for(CRClen = 0; CRClen < len; CRClen++)
    {
        CRCdata^= ptr[CRClen];
        for(j = 0; j < 8; ++j)
        {
            if(CRCdata & 0x01)
            {
                CRCdata = (CRCdata >> 1)^0xa001;
            }
            else
            {
                CRCdata = (CRCdata >> 1);
            }
        }
    }
    CRCdata = ((CRCdata & 0xff) << 8)|(CRCdata >> 8);
    return CRCdata;
}

void cSerial::doworks()
{

    unsigned int count= 0,step = 0,Package_count = 0;
    unsigned char temp_buf[280];
    unsigned long int bin_address = 0;
    if(serial->isOpen())
    {
        while(1)
        {

            {
                QMutexLocker locker(&m_mutex);
                        if(stopped){
                            return;
                        }
            }

            if(Enable_IAP_flag)
            {
                if(serial->waitForBytesWritten(2000))
                {
                    Sleep(100);
                    if(serial->waitForReadyRead(2000))
                    {
                        readbuf = serial->readAll();
                        if(readbuf == Enable_IAP_ACK)
                        {
                            count = 0;
                            Enable_IAP_flag = false;
                            step = 1;
                            emit send_messagebox(tr("使能IAP成功"));
                        }
                        else
                        {
                            count++;
                            if(count > 10)
                            {
                                emit send_messagebox(tr("升级失败"));
                                return;
                            }
                            Enable_IAP_flag = false;
                            emit send_messagebox(tr("使能IAP失败"));
                        }
                    }
                    else
                    {
                        count++;
                        if(count > 10)
                        {
                            emit send_messagebox(tr("升级失败"));
                            return;
                        }
                        Enable_IAP_flag = false;
                        emit send_messagebox(tr("无应答"));
                    }
                }
                else
                {
                    return;
                }
            }
            else if(step == 0)
            {
                serial->write(Enable_IAP);
                QMutexLocker locker(&m_mutex);
                Enable_IAP_flag = true;

            }

            if(Start_IAP_flag)
            {
                if(serial->waitForBytesWritten(2000))
                {
                    Sleep(100);
                    if(serial->waitForReadyRead(2000))
                    {
                        readbuf = serial->readAll();
                        if(readbuf == Start_IAP_ACK)
                        {
                            count = 0;
                            Start_IAP_flag = false;
                            step = 2;
                            emit send_messagebox(tr("启动IAP成功"));
                        }
                        else
                        {
                            count++;
                            if(count > 10)
                            {
                                emit send_messagebox(tr("升级失败"));
                                return;
                            }
                            Start_IAP_flag = false;
                            emit send_messagebox(tr("启动IAP失败"));
                        }
                    }
                    else
                    {
                        count++;
                        if(count > 10)
                        {
                            emit send_messagebox(tr("升级失败"));
                            return;
                        }
                        Start_IAP_flag = false;
                        emit send_messagebox(tr("无应答"));
                    }
                }
                else
                {
                    return;
                }
            }
            else if(step == 1)
            {
                Start_IAP[18] = mcu;
                memcpy(temp_buf,Start_IAP,19);
                Start_IAP[19] = (CRC16(temp_buf,19)>>8);
                Start_IAP[20] = (CRC16(temp_buf,19)&0xff);
                serial->write(Start_IAP);
                QMutexLocker locker(&m_mutex);
                Start_IAP_flag = true;
            }

            if(File_Update_flag)
            {
                if(serial->waitForBytesWritten(2000))
                {
                    Sleep(200);
                    if(serial->waitForReadyRead(2000))
                    {
                        readbuf = serial->readAll();
                        if(readbuf == File_Update_ACK)
                        {
                            count = 0;
                            File_Update_flag = false;
                            step = 3;
                            emit send_messagebox(tr("文件信息传送成功"));
                        }
                        else
                        {
                            count++;
                            if(count > 10)
                            {
                                emit send_messagebox(tr("升级失败"));
                                return;
                            }
                            File_Update_flag = false;
                            emit send_messagebox(tr("文件信息传送失败"));
                        }
                    }
                    else
                    {
                        count++;
                        if(count > 10)
                        {
                            emit send_messagebox(tr("升级失败"));
                            return;
                        }
                        File_Update_flag = false;
                        emit send_messagebox(tr("无应答"));
                    }
                }
                else
                {
                    return;
                }
            }
            else if(step == 2)
            {
                serial->write(File_Update);
                QMutexLocker locker(&m_mutex);
                File_Update_flag = true;
            }

            if(Code_Update_flag)
            {
                if(serial->waitForBytesWritten(2000))
                {
                    Sleep(300);
                    if(serial->waitForReadyRead(2000))
                    {
                        readbuf = serial->readAll();
                        if(readbuf == Code_Update_ACK)
                        {
                            count = 0;
                            Code_Update_flag = false;
                            bin_address=bin_address+236;
                            emit send_messagebox(tr("Bag%1 传送成功").arg(Package_count));
                            emit send_progressbar(Package_count);
                            if(Package_count == Total_Package)
                            {

                                step = 4;
                            }
                        }
                        else
                        {
                            count++;
                            if(count > 10)
                            {
                                emit send_messagebox(tr("升级失败"));
                                return;
                            }
                            Code_Update_flag = false;
                            emit send_messagebox(tr("Bag%1 传送失败").arg(Package_count));
                            Package_count--;
                        }
                    }
                    else
                    {
                        count++;
                        Package_count--;
                        if(count > 10)
                        {
                            emit send_messagebox(tr("升级失败"));
                            return;
                        }
                        Code_Update_flag = false;
                        emit send_messagebox(tr("无应答"));
                    }
                }
                else
                {
                    return;
                }
            }
            else if(step == 3)
            {
                if(Package_count < (Total_Package-1))
                {
                    for(int x = 0; x < 236; x++)
                    {
                        Code_Update[15+x] = code_data[(Package_count*236)+x];

                    }
                    Package_count++;
                    Code_Update[4] = 0x00;
                    Code_Update[5] = 0x7A;
                    Code_Update[6] = 0xF4;
                    Code_Update[7] = (Package_count>>8);
                    Code_Update[8] = (Package_count&0xff);
                    Code_Update[9] = (bin_address >> 24);
                    Code_Update[10] = (bin_address >> 16);
                    Code_Update[11] = (bin_address >> 8);
                    Code_Update[12] = (bin_address&0xff);
                    Code_Update[13] = 0x00;
                    Code_Update[14] = 0xEC;
                    memcpy(temp_buf,Code_Update,251);
                    Code_Update[251] = (CRC16(temp_buf,251)>>8);
                    Code_Update[252] = (CRC16(temp_buf,251)&0xff);
                    serial->write(Code_Update.constData(),253);

                    Code_Update_ACK[5] = 0x7A;
                    Code_Update_ACK[6] = 0xDB;
                    Code_Update_ACK[7] = 0x22;
                }
                else
                {
                    for(int x = 0; x < Last_package_len; x++)
                    {
                        Code_Update[15+x] = code_data[(Package_count*236)+x];

                    }
                    Package_count++;
                    Code_Update[4] = 0x00;
                    Code_Update[5] = (4+(Last_package_len/2));
                    Code_Update[6] = (4+(Last_package_len/2))*2;
                    Code_Update[7] = (Package_count>>8);
                    Code_Update[8] = (Package_count&0xff);
                    Code_Update[9] = (bin_address >> 24);
                    Code_Update[10] = (bin_address >> 16);
                    Code_Update[11] = (bin_address >> 8);
                    Code_Update[12] = (bin_address&0xff);
                    Code_Update[13] = 0x00;
                    Code_Update[14] = Last_package_len;
                    unsigned int temp_len;
                    temp_len = 15+Last_package_len;
                    memcpy(temp_buf,Code_Update,temp_len);
                    Code_Update[temp_len] = (CRC16(temp_buf,temp_len)>>8);
                    Code_Update[temp_len+1] = (CRC16(temp_buf,temp_len)&0xff);
                    serial->write(Code_Update.constData(),temp_len+2);

                    Code_Update_ACK[5] = (4+(Last_package_len/2));
                    memcpy(temp_buf,Code_Update_ACK,6);
                    Code_Update_ACK[6] = (CRC16(temp_buf,6)>>8);
                    Code_Update_ACK[7] = (CRC16(temp_buf,6)&0xff);

                }
                QMutexLocker locker(&m_mutex);
                Code_Update_flag = true;
            }

            if(Version_Ask_flag)
            {
                if(serial->waitForBytesWritten(2000))
                {
                    Sleep(100);
                    if(serial->waitForReadyRead(2000))
                    {
                        readbuf = serial->readAll();
                        if(readbuf.left(2) == Version_ACK)
                        {
                            QByteArray Version_Temp;
                            Version_Temp = readbuf.mid(3,16);
                            count = 0;
                            Version_Ask_flag = false;
                            emit send_messagebox(tr("版本号读取成功"));
                            emit send_messagebox(tr("版本号: ")+Version_Temp);
                            emit send_messagebox(tr("升级成功"));
                            return;

                        }
                        else
                        {
                            count++;
                            if(count > 10)
                            {
                                emit send_messagebox(tr("无法读取版本号，请验证！"));
                                return;
                            }
                            Version_Ask_flag = false;
                            emit send_messagebox(tr("读取版本号失败"));
                        }
                    }
                    else
                    {
                        count++;
                        if(count > 10)
                        {
                            emit send_messagebox(tr("无法读取版本号，请验证！"));
                            return;
                        }
                        Version_Ask_flag = false;
                        emit send_messagebox(tr("无应答"));
                    }
                }
                else
                {
                    return;
                }
            }
            else if(step == 4)
            {
                emit send_messagebox(tr("重启中，等待读取版本号！"));
                Sleep(Total_Package*30);
                if(Soft_Version.left(3) == "Sun")
                {
                    serial->write(SunnyBee_Version);
                }
                else if(Soft_Version.left(3) == "Hor")
                {
                    serial->write(Hornet_Version);
                }
                else
                {
                    emit send_messagebox(tr("升级成功"));
                    return;
                }

                QMutexLocker locker(&m_mutex);
                Version_Ask_flag = true;
            }
        }
    }
}

void cSerial::cSerial_init()
{
    Enable_IAP_flag = false;
    Start_IAP_flag = false;
    File_Update_flag = false;
    Code_Update_flag = false;
    Version_Ask_flag = false;

    Enable_IAP[0] = 0x01;
    Enable_IAP[1] = 0x10;
    Enable_IAP[2] = 0x70;
    Enable_IAP[3] = 0x00;
    Enable_IAP[4] = 0x00;
    Enable_IAP[5] = 0x02;
    Enable_IAP[6] = 0x04;
    Enable_IAP[7] = 0x49;
    Enable_IAP[8] = 0x41;
    Enable_IAP[9] = 0x50;
    Enable_IAP[10] = 0x45;
    Enable_IAP[11] = 0x2D;
    Enable_IAP[12] = 0xD6;

    Enable_IAP_ACK[0] = 0x01;
    Enable_IAP_ACK[1] = 0x10;
    Enable_IAP_ACK[2] = 0x70;
    Enable_IAP_ACK[3] = 0x00;
    Enable_IAP_ACK[4] = 0x00;
    Enable_IAP_ACK[5] = 0x02;
    Enable_IAP_ACK[6] = 0x5B;
    Enable_IAP_ACK[7] = 0x08;

    Start_IAP[0] = 0x01;
    Start_IAP[1] = 0x10;
    Start_IAP[2] = 0x70;
    Start_IAP[3] = 0x09;
    Start_IAP[4] = 0x00;
    Start_IAP[5] = 0x06;
    Start_IAP[6] = 0x0c;
    Start_IAP[7] = 0xAA;
    Start_IAP[8] = 0xAA;
    Start_IAP[9] = 0x45;
    Start_IAP[10] = 0x41;
    Start_IAP[11] = 0x53;
    Start_IAP[12] = 0x54;
    Start_IAP[13] = 0x46;
    Start_IAP[14] = 0x49;
    Start_IAP[15] = 0x52;
    Start_IAP[16] = 0x4D;
    Start_IAP[17] = 0x00;
    Start_IAP[18] = mcu;
    Start_IAP[19] = 0x05;
    Start_IAP[20] = 0xDE;

    Start_IAP_ACK[0] = 0x01;
    Start_IAP_ACK[1] = 0x10;
    Start_IAP_ACK[2] = 0x70;
    Start_IAP_ACK[3] = 0x09;
    Start_IAP_ACK[4] = 0x00;
    Start_IAP_ACK[5] = 0x06;
    Start_IAP_ACK[6] = 0x8A;
    Start_IAP_ACK[7] = 0xC9;

    File_Update[0] = 0x01;
    File_Update[1] = 0x10;
    File_Update[2] = 0x70;
    File_Update[3] = 0x12;
    File_Update[4] = 0x00;
    File_Update[5] = 0x16;
    File_Update[6] = 0x2C;
    for(int i = 7; i < 47; i++)
    {
        File_Update[i] = 0x30;
    }
    File_Update[47] = 0x00;
    File_Update[48] = 0xEC;
    File_Update[49] = 0x00;
    File_Update[50] = 0x00;
    File_Update[51] = 0x00;
    File_Update[52] = 0x00;

    File_Update_ACK[0] = 0x01;
    File_Update_ACK[1] = 0x10;
    File_Update_ACK[2] = 0x70;
    File_Update_ACK[3] = 0x12;
    File_Update_ACK[4] = 0x00;
    File_Update_ACK[5] = 0x16;
    File_Update_ACK[6] = 0xfb;
    File_Update_ACK[7] = 0x02;

    Code_Update[0] = 0x01;
    Code_Update[1] = 0x10;
    Code_Update[2] = 0x70;
    Code_Update[3] = 0x28;
    Code_Update[4] = 0x00;
    Code_Update[5] = 0x00;
    Code_Update[6] = 0x00;
    for(int i = 7; i <253; i++ )
    {
        Code_Update[i] = 0x00;
    }

    Code_Update_ACK[0] = 0x01;
    Code_Update_ACK[1] = 0x10;
    Code_Update_ACK[2] = 0x70;
    Code_Update_ACK[3] = 0x28;
    Code_Update_ACK[4] = 0x00;
    Code_Update_ACK[5] = 0x7A;
    Code_Update_ACK[6] = 0xDB;
    Code_Update_ACK[7] = 0x22;

    SunnyBee_Version[0] = 0x01;
    SunnyBee_Version[1] = 0x04;
    SunnyBee_Version[2] = 0x00;
    SunnyBee_Version[3] = 0x2c;
    SunnyBee_Version[4] = 0x00;
    SunnyBee_Version[5] = 0x10;
    SunnyBee_Version[6] = 0x30;
    SunnyBee_Version[7] = 0x0f;

    Hornet_Version[0] = 0x01;
    Hornet_Version[1] = 0x04;
    Hornet_Version[2] = 0x00;
    Hornet_Version[3] = 0x22;
    Hornet_Version[4] = 0x00;
    Hornet_Version[5] = 0x10;
    Hornet_Version[6] = 0x51;
    Hornet_Version[7] = 0xcc;

    Version_ACK[0] = 0x01;
    Version_ACK[1] = 0x04;



}

QByteArray cSerial::read_all()
{
    static QByteArray read_temp_buf;
    unsigned char read_buf[20];
    int i = 0;



        read_temp_buf = serial->readAll();
        i = read_temp_buf.size();
        memcpy(read_buf,read_temp_buf,i);
        auto crc = CRC16(read_buf,i-2);
        if(read_buf[i-1] == (crc&0xff))
        {
            qDebug()<< read_temp_buf<<endl;
             return read_temp_buf;
        }
        else
            return 0;



}

void cSerial::open_com()
{
    serial = new QSerialPort;
    serial->setPortName(com);
    qDebug()<<com<<endl;
    serial->open(QIODevice::ReadWrite);
    serial->setBaudRate(baud);//设置波特率为115200
    serial->setDataBits(QSerialPort::Data8);//设置数据位8
    serial->setParity(QSerialPort::NoParity); //校验位设置为0
    serial->setStopBits(QSerialPort::OneStop);//停止位设置为1
    serial->setFlowControl(QSerialPort::NoFlowControl);//设置为无流控制

}

void cSerial::close_com()
{
    //关闭串口
    serial->clear();
    serial->close();
    serial->deleteLater();
    qDebug()<<"串口关闭"<<endl;

}

void cSerial::receive_com(QString str)
{
    com = str;

}

void cSerial::receive_baud(int index)
{
    baud = index;

}

void cSerial::open_serial_port()
{
    open_com();

}

void cSerial::close_serial_port()
{
    close_com();

}

void cSerial::set_File_Update_array()
{
    unsigned char buf[60];
    File_Update[0] = 0x01;
    File_Update[1] = 0x10;
    File_Update[2] = 0x70;
    File_Update[3] = 0x12;
    File_Update[4] = 0x00;
    File_Update[5] = 0x16;
    File_Update[6] = 0x2C;
   // QMutexLocker locker(&m_mutex);
    for(int i = 0; i < Soft_Version.size(); i++)
    {
       File_Update[7+i] = Soft_Version[i];
    }
    File_Update[39] = (File_CRC >> 24);
    File_Update[40] = (File_CRC >> 16);
    File_Update[41] = (File_CRC >> 8);
    File_Update[42] = (File_CRC);
    File_Update[43] = (Total_Len >> 24);
    File_Update[44] = (Total_Len >> 16);
    File_Update[45] = (Total_Len >> 8);
    File_Update[46] = (Total_Len );
    File_Update[47] = 0x00;
    File_Update[48] = 0xEC;
    File_Update[49] = (char)(Total_Package >> 8);
    File_Update[50] = (char)(Total_Package);
    memcpy(buf,File_Update,51);
    File_Update[51] = (CRC16(buf,51) >> 8);
    File_Update[52] = (CRC16(buf,51)&0xff);

}


