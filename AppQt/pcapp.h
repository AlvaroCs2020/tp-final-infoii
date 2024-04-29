#ifndef PCAPP_H
#define PCAPP_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>

QT_BEGIN_NAMESPACE
namespace Ui { class PcApp; }
QT_END_NAMESPACE

class PcApp : public QMainWindow
{
    Q_OBJECT

public:
    PcApp(QWidget *parent = nullptr);
    ~PcApp();

private slots:
    void on_bSearchFile_clicked();

    void on_bLoadData_clicked();

    void on_bSetMaxV_clicked();

    void show_plot();


    void on_bConnectAr_clicked();

    void on_bSetMin_clicked();

private:
    Ui::PcApp *ui;
    //Data for the graphics
    QVector<QString> dataY;
    QVector<QString> dataX;
    QSerialPort *arduino;
    static const quint16 arduino_uno_vendor_id = 9025;
    static const quint16 arduino_UNO = 6790;
    static const quint16 arduino_MEGA = 29987;
    QString arduino_puerto;
    bool arduino_esta_disponible;
    void buscaYconectaArduino();
};
#endif // PCAPP_H
