#ifndef UPDATEWINDOW_H
#define UPDATEWINDOW_H

#include <QApplication>
#include <QMainWindow>
#include <QDebug>
#include <QThread>
#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QTranslator>
#include "serial.h"


namespace Ui {
class UpdateWindow;
}

class UpdateWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit UpdateWindow(QWidget *parent = nullptr);
    ~UpdateWindow();
    QByteArray bin_date;
    QTranslator translator;
    void scan_serial_port();
    QString nameSerialport[5];
    unsigned long int swap_32(unsigned long int val);
    void Translation_to_zh();
    void Translation_to_en();



signals:
    void send_serial_com(QString str);
    void send_serial_baud(int index);
    void open_serial_port();
    void close_serial_port();
    void send_update();
    void send_file_info();

public slots:

    void on_Open_com_clicked();

    void on_BaudBox_activated(int index);

    void on_Open_file_clicked();

    void on_Update_clicked();

    void set_textoutput(const QString str);

    void receive_messagebox(int index);

    void receive_progressbar(unsigned int index);

    void time_handle();


private slots:
    void on_McuBox_activated(int index);

private:
    qint32 cBaud = 0;
    QString Com;
    Ui::UpdateWindow *ui;
    QThread *thread ;
    cSerial *cserial;

};

#endif // UPDATEWINDOW_H
