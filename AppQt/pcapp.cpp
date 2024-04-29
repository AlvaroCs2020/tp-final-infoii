#include "pcapp.h"
#include "ui_pcapp.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <QRandomGenerator>
#include <QtCharts>

///using namespace QtCharts;
using namespace std;

PcApp::PcApp(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PcApp)
{
    ui->setupUi(this);
}

PcApp::~PcApp()
{
    delete ui;
}


void PcApp::on_bSearchFile_clicked()
{
    QString filename = QFileDialog::getOpenFileName(
        this,
        tr("Open File"),
        "~/home",
        "All files (*.*)"
        );
    ui->lineEdit->setText(filename);
}


void PcApp::on_bLoadData_clicked()
{
    this->dataY.clear();
    this->dataX.clear();
    string filename = ui->lineEdit->text().toStdString();//file path entered by the user
    //todo esto podria ir en una clase aparte
    ifstream inputFile(filename);
    vector<vector<string>> data; // 3xN array
    if (inputFile.is_open()) {
        string line;

        while (getline(inputFile, line)) {
            vector<string> tokens;
            stringstream ss(line);
            string token;

            while (getline(ss, token, ';')) {
                tokens.push_back(token);
            }
            data.push_back(tokens);
        }

        inputFile.close();
    } else {
        QMessageBox::information(
            this,
            tr("Error opening file!"),
            "Error: Unable to open the file."
            );
    }

    //Data stored into the
    for (const vector<string>& row : data) {

        this->dataY.append(QString::fromStdString(row[0]));
        this->dataX.append(QString::fromStdString(row[1]));

    }

    this->show_plot();

}
void PcApp::show_plot()
{
    QLineSeries *series = new QLineSeries();
    for (int var = 0; var < this->dataX.length(); ++var)
    {

        QStringList values = this->dataX[var].split(QLatin1Char('-'), Qt::SkipEmptyParts);
        QDateTime momentInTime;
        momentInTime.setDate(QDate(values[2].toInt(), values[1].toInt() , values[0].toInt()));
        qDebug() << values[0] << values[1] << values[2];
        series->append(momentInTime.toMSecsSinceEpoch(), this->dataY[var].toDouble());
    }
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->legend()->hide();

    QDateTimeAxis *axisX = new QDateTimeAxis;
    axisX->setTickCount(this->dataX.length());
    axisX->setFormat("mmm yy");
    axisX->setLabelsAngle(30);
    axisX->setTitleText("Date");
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis;
    axisY->setLabelFormat("%f");
    axisY->setTitleText("Voltage");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chart->setTitle("");
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    QMainWindow window;

    window.setCentralWidget(chartView);
    window.resize(1200, 1200);
    window.show();
    this->ui->graphicsView->setViewport(window.centralWidget());

}

void PcApp::on_bSetMaxV_clicked()
{
    // Getting voltage value and formating it
    QString voltage = this->ui->voltageInp->text();
    bool isDouble;
    double doubleValueVoltage = voltage.toDouble(&isDouble);
    voltage = QString::number(doubleValueVoltage, 'f', 2);
    voltage = 10 > doubleValueVoltage ? "0" + voltage : voltage;
    voltage = "VH" + voltage + "\n";

    // Convert QString to char*
    QByteArray byteArray = voltage.toUtf8();
    char *arduinoCommand = byteArray.data();

    if (!isDouble || doubleValueVoltage < 0)
    {
        QMessageBox::information(
            this,
            tr("Error with voltage format"),
            "Please enter a positive number"
            );
        return;
    }
    if(!arduino_esta_disponible)
    {
        return;
    }

    if(arduino_esta_disponible && arduino->isWritable())
    {
        qDebug() << "se mando el mensaje " << voltage;
        arduino->write(arduinoCommand);
    }

}
void PcApp::on_bSetMin_clicked()
{
    // Getting voltage value and formating it
    QString voltage = this->ui->percentageInp->text();
    bool isDouble;
    double doubleValueVoltage = voltage.toDouble(&isDouble);
    voltage = QString::number(doubleValueVoltage, 'f', 2);
    voltage = 10 > doubleValueVoltage ? "0" + voltage : voltage;
    voltage = "VL" + voltage + "\n";

    // Convert QString to char*
    QByteArray byteArray = voltage.toUtf8();
    char *arduinoCommand = byteArray.data();

    if (!isDouble || doubleValueVoltage < 0)
    {
        QMessageBox::information(
            this,
            tr("Error with voltage format"),
            "Please enter a positive number"
            );
        return;
    }

    if(!arduino_esta_disponible)
    {
        return;
    }


    if(arduino_esta_disponible && arduino->isWritable())
    {
        qDebug() << "se mando el mensaje " << voltage;
        arduino->write(arduinoCommand);
    }

}

void PcApp::on_bConnectAr_clicked()
{
    //Parte # 1, declaración inicial de las variables
    arduino_esta_disponible = false;
    arduino_puerto = "";
    arduino = new QSerialPort;
    QString nombreDispositivoSerial = "";
    int nombreProductID = 0;

    //Parte # 2,buscar puertos con los identificadores de Arduino
    qDebug() << "Puertos disponibles: " << QSerialPortInfo::availablePorts().length();
    foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()) {
        if(serialPortInfo.hasVendorIdentifier()){
            qDebug() << "ID Vendedor " << serialPortInfo.vendorIdentifier();
            qDebug() << "ID Producto: " << serialPortInfo.productIdentifier();
            qDebug() << "Nombre del puerto: " << serialPortInfo.portName();

            if(serialPortInfo.productIdentifier() == arduino_UNO || serialPortInfo.productIdentifier() == arduino_MEGA){
                arduino_esta_disponible = true;
                nombreDispositivoSerial = serialPortInfo.portName();
                nombreProductID = serialPortInfo.productIdentifier();
            }

        }

    }
    //Parte # 3, conexión del puerto encontrado con Arduino

    if(arduino_esta_disponible){
        arduino_puerto = nombreDispositivoSerial;
        arduino ->setPortName(arduino_puerto);
        arduino->open(QIODevice::ReadWrite);
        arduino->setDataBits(QSerialPort::Data8);
        arduino ->setBaudRate(QSerialPort::Baud115200);
        arduino->setParity(QSerialPort::NoParity);
        arduino->setStopBits(QSerialPort::OneStop);
        arduino->setFlowControl(QSerialPort::NoFlowControl);
        ui->label->clear();
        qDebug() << "Producto: " << nombreProductID;
        if(nombreProductID == arduino_UNO || nombreProductID == arduino_MEGA) ui->label->setText("Arduino NANO conectado");
        else ui->label->setText("Producto desconocido");
    }
    else{
        ui->label->clear();
        ui->label->setText("No hay arduino");
    }
    qDebug() << arduino->isWritable();
}




