#include "updatewindow.h"
#include "ui_updatewindow.h"
#include <QTextStream>
#include <windows.h>
#include <QString>
#include <QThread>
#include <QTimer>
#include <QtEndian>

UpdateWindow::UpdateWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::UpdateWindow)
{
     ui->setupUi(this);
     translator.load(":/I18N/I18N_en_UK.qm");
     QApplication::instance()->installTranslator(&translator);
     ui->retranslateUi(this);
     cserial = new cSerial;
     thread = new QThread;
     cserial->moveToThread(thread);
     scan_serial_port();
     QTimer *M_time = new QTimer(this);
     ui->McuBox->setCurrentIndex(0);
     ui->BaudBox->setCurrentIndex(0);
     ui->action_English->setChecked(true);

     for(int i = 0;i < 5; i++)
     {
       ui->PortBox->addItem("");
     }


     QObject::connect(this,&UpdateWindow::send_update,cserial,&cSerial::doworks);
     QObject::connect(this,&UpdateWindow::send_serial_com,cserial,&cSerial::receive_com);
     QObject::connect(this,&UpdateWindow::send_serial_baud,cserial,&cSerial::receive_baud);
     QObject::connect(this,&UpdateWindow::open_serial_port,cserial,&cSerial::open_serial_port);
     QObject::connect(this,&UpdateWindow::close_serial_port,cserial,&cSerial::close_serial_port);
     QObject::connect(this,&UpdateWindow::send_file_info,cserial,&cSerial::set_File_Update_array);
     QObject::connect(cserial,&cSerial::send_messagebox,this,&UpdateWindow::set_textoutput);
     QObject::connect(cserial,&cSerial::send_progressbar,this,&UpdateWindow::receive_progressbar);
     QObject::connect(M_time,&QTimer::timeout,this,&UpdateWindow::time_handle);
     QObject::connect(thread,&QThread::finished,cserial,&cSerial::deleteLater);
     QObject::connect(thread,&QThread::finished,thread,&QThread::deleteLater);
     QObject::connect(ui->action_Chinese,&QAction::triggered,this,&UpdateWindow::Translation_to_zh);
     QObject::connect(ui->action_English,&QAction::triggered,this,&UpdateWindow::Translation_to_en);

     M_time->start(1000);
}

UpdateWindow::~UpdateWindow()
{
    thread->quit();
    thread->wait();
    delete ui;

}

void UpdateWindow::scan_serial_port()
{
    int Port_count = 0;
    for(int i = 0; i < 5; i++)
     ui->PortBox->setItemText(i,"");
    foreach (const QSerialPortInfo &info,QSerialPortInfo::availablePorts())
        {
            nameSerialport[Port_count] = info.portName();
            ui->PortBox->setItemText(Port_count,nameSerialport[Port_count]);
            Port_count++;
        }
}

void UpdateWindow::time_handle()
{
    if(ui->Open_com->text() == tr("打开串口"))
    {
        scan_serial_port();
    }
    else
    {

    }
}

void UpdateWindow::on_Open_com_clicked()
{

    if(ui->Open_com->text() == tr("打开串口"))
    {
        if(ui->PortBox->currentText() == "")
        {
            QMessageBox::information(this,tr("警告"),tr("无可用串口"),QMessageBox::Yes);
            return;
        }
        thread->start();
        if(cBaud == 0)
        {
            emit send_serial_baud(QSerialPort::Baud9600);
        }
        else
        {
            emit send_serial_baud(QSerialPort::Baud115200);
        }

         emit send_serial_com(ui->PortBox->currentText());
        //关闭设置菜单使能
        ui->McuBox->setEnabled(false);
        ui->PortBox->setEnabled(false);
        ui->BaudBox->setEnabled(false);
        ui->Open_com->setText(tr("关闭串口"));
        emit open_serial_port();

    }
    else
    {     
        //恢复设置使能
        ui->McuBox->setEnabled(true);
        ui->PortBox->setEnabled(true);
        ui->BaudBox->setEnabled(true);
        ui->Open_com->setText(tr("打开串口"));
        cserial->stopped = true;
        emit close_serial_port();


    }
}

void UpdateWindow::on_BaudBox_activated(int index)
{
    switch (index)
    {
        case 0:
            cBaud = QSerialPort::Baud9600;
            emit send_serial_baud(cBaud);
        break;
        case 1:
            cBaud = QSerialPort::Baud115200;
            emit send_serial_baud(cBaud);
        break;
    }
}

void UpdateWindow::on_Open_file_clicked()
{
    unsigned long int temp_bin_data[200000];
    if(ui->Open_com->text() == tr("打开串口"))
    {
        QMessageBox::information(this,tr("警告"),tr("请先打开串口"),QMessageBox::Yes);
    }
    else
    {
        QString Soft_Version;
        QByteArray Total_Len;
        QString filename = QFileDialog::getOpenFileName(this,"Open file",nullptr,"bin File(*.bin)");
        ui->lineEdit->setText(filename);
        QFile file(filename);
        QFileInfo fileinfo;
        fileinfo = QFileInfo(filename);
        Soft_Version = fileinfo.fileName();
        cserial->Soft_Version = Soft_Version.toLatin1();

        cserial->Total_Len = (unsigned long)file.size();
        if(cserial->Total_Len%236)
        {
            cserial->Total_Package = (cserial->Total_Len/236)+1;
            cserial->Last_package_len = cserial->Total_Len%236;
        }
        else
        {
            cserial->Total_Package = cserial->Total_Len/236;
        }

        ui->progressBar->setRange(0,cserial->Total_Package);


        bool ok = file.open(QIODevice::ReadOnly);
        if(ok)
        {
            bin_date = file.readAll();
            memcpy(temp_bin_data,bin_date,bin_date.size());
            for(int k = 0; k < (bin_date.size()/4); k++)
            temp_bin_data[k] = swap_32(temp_bin_data[k]);
            cserial->File_CRC = cserial->CRC32(0xffffffff,temp_bin_data,(bin_date.size()/4));
            cserial->code_data = bin_date;
        }
        emit send_file_info();
        file.close();
    }


}

unsigned long int UpdateWindow::swap_32(unsigned long int val)
{
    val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0x00FF00FF);
    return (val << 16) | (val >> 16);

}

void UpdateWindow::on_Update_clicked()
{
    if(ui->Update->text() == tr("更新"))
    {
        if(ui->Open_com->text() == tr("打开串口"))
        {
            QMessageBox::information(this,tr("警告"),tr("未打开串口"),QMessageBox::Yes);
        }
        else
        {
            if(ui->lineEdit->text() == tr(""))
            {
                 QMessageBox::information(this,tr("警告"),tr("未选择升级文件"),QMessageBox::Yes);
            }
            else
            {
                cserial->stopped = false;
                emit send_update();
                ui->Update->setText(tr("停止"));
            }

        }

    }
    else if(ui->Update->text() == tr("停止"))
    {
       ui->Update->setText(tr("更新"));
       cserial->stopped = true;
       cserial->Enable_IAP_flag = false;
       cserial->Start_IAP_flag = false;
       cserial->File_Update_flag = false;
       cserial->Code_Update_flag = false;
       cserial->Version_Ask_flag = false;
    }

}

void UpdateWindow::set_textoutput(const QString str)
{

    ui->textoutput->append(tr("%1").arg(str));
    if((str == tr("升级失败"))||(str == tr("升级成功")))
    {
        ui->Update->setText(tr("更新"));
        cserial->stopped = true;
        cserial->Enable_IAP_flag = false;
        cserial->Start_IAP_flag = false;
        cserial->File_Update_flag = false;
        cserial->Code_Update_flag = false;
        cserial->Version_Ask_flag = false;

        ui->McuBox->setEnabled(true);
        ui->PortBox->setEnabled(true);
        ui->BaudBox->setEnabled(true);
        ui->Open_com->setText(tr("打开串口"));
        ui->progressBar->setValue(0);
        emit close_serial_port();

    }
}

void UpdateWindow::receive_progressbar(unsigned int index)
{
    ui->progressBar->setValue(index);
}

void UpdateWindow::receive_messagebox(int index)
{
    switch(index)
    {
        case 0:
        QMessageBox::information(this,tr("警告"),tr("未打开串口"),QMessageBox::Yes);
        break;

    }

}



void UpdateWindow::on_McuBox_activated(int index)
{
    cserial->mcu = index;
}

void UpdateWindow::Translation_to_en()
{

    translator.load(":/I18N/I18N_en_UK.qm");
    QApplication::instance()->installTranslator(&translator);
    ui->retranslateUi(this);
    ui->action_Chinese->setChecked(false);
    ui->action_English->setChecked(true);
}

void UpdateWindow::Translation_to_zh()
{
    QApplication::instance()->removeTranslator(&translator);
    QApplication::instance()->installTranslator(nullptr);
    ui->retranslateUi(this);
    ui->action_English->setChecked(false);
    ui->action_Chinese->setChecked(true);
}

