#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "modbus-tcp.h"
#include "modbus.h"


#include <QIntValidator>
#include <QSettings>
#include <QScrollBar>
#include <QDebug>
#include <errno.h>
#include <QTimer>

const int DataTypeColumn = 0;
const int AddrColumn = 1;
const int DataColumn = 2;

extern MainWindow * globalMainWin;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
    isPlotting(false),
    m_tcpModbus(NULL),
    m_tcpActive(false),
    m_poll(false)
{
    ui->setupUi(this);
    refreshRate = new QTimer(this);
    connect(refreshRate, SIGNAL(timeout()), this, SLOT(sendTcpRequest()));
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout,this, &MainWindow::updatePlot);


    ui->customPlot->addGraph();
    ui->customPlot->graph(0)->setScatterStyle(QCPScatterStyle::ssCircle);
    ui->customPlot->graph(0)->setLineStyle(QCPGraph::lsLine);
    ui->customPlot->xAxis->setLabel("Data");
    ui->customPlot->yAxis->setLabel("Register");
    ui->customPlot->xAxis->setRange(0,100);
    ui->customPlot->yAxis->setRange(-500,500);

    ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

//    QVector<double> x = {AddrColumn},y = {DataColumn};

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_tcpEnable_clicked(bool checked)
{
    enableTCPEdit(true);
    if(checked)
    {
        refreshRate->setInterval(ui->refreshRateEdit->text().toInt());
        refreshRate->start();
        m_tcpActive = true;
        enableTCPEdit(false);
    }
    else
    {
        refreshRate->stop();
        enableTCPEdit(false);
    }
}


void MainWindow::enableTCPEdit(bool checked)
{
    ui->edNetworkAddress->setEnabled(checked);
    ui->edPort->setEnabled(checked);
    ui->refreshRateEdit->setEnabled(checked);
}

void MainWindow::tcpConnect()
{
    int portNbr = ui->edPort->text().toInt();
    changeModbusInterface(ui->edNetworkAddress->text(), portNbr);
}

int MainWindow::setupModbusPort()
{
    return 0;
}

static QString descriptiveDataTypeName( int funcCode )
{
    switch( funcCode )
    {
    case MODBUS_FC_READ_COILS:
    case MODBUS_FC_WRITE_SINGLE_COIL:
    case MODBUS_FC_WRITE_MULTIPLE_COILS:
        return "Coil (binary)";
    case MODBUS_FC_READ_DISCRETE_INPUTS:
        return "Discrete Input (binary)";
    case MODBUS_FC_READ_HOLDING_REGISTERS:
    case MODBUS_FC_WRITE_SINGLE_REGISTER:
    case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
        return "Holding Register (16 bit)";
    case MODBUS_FC_READ_INPUT_REGISTERS:
        return "Input Register (16 bit)";
    default:
        break;
    }
    return "Unknown";
}


void MainWindow::changeModbusInterface(const QString& address, int portNbr )
{
    if (m_tcpModbus)
    {
        modbus_close(m_tcpModbus);
        modbus_free(m_tcpModbus);
        m_tcpModbus = NULL;
    }

    m_tcpModbus = modbus_new_tcp( address.toLatin1().constData(), portNbr);
    if(modbus_connect(m_tcpModbus) == -1)
    {
        if (m_tcpModbus)
        {
            emit connectionError(tr ("Could not connect to TCP/IP Port!"));

            modbus_close(m_tcpModbus);
            modbus_free(m_tcpModbus);
            m_tcpModbus = NULL;
        }
    }

}

void MainWindow::sendTcpRequest(void)
{

    if(m_tcpActive)
    {
        tcpConnect();
    }
    if(m_tcpModbus == NULL)
    {
        ui->connectionStatus->setText("Not Connected");
        return;
    }

    ui->connectionStatus->setText("Connection Established");
    const int slave = ui->slaveId->value();
    const int func = 04;
    const int addr = ui->addressReg->value();
    int num = ui->coilNum->value();
    uint8_t dest[1024];
    uint16_t * dest16 = (uint16_t *) dest;

    memset(dest, 0, 1024);

    int ret = -1;
    bool is16Bit = false;
    const QString dataType = descriptiveDataTypeName(func);

    modbus_set_slave(m_tcpModbus, slave);

    ret = modbus_read_registers( m_tcpModbus, addr, num, dest16);
    is16Bit = true;
    //ui->debugLabel->setText();
    if(ret == num)
    {
        bool b_hex = is16Bit;
        QString qs_num;
        ui->regTable->setRowCount( num );

        for(int i = 0; i < num; ++i)
        {
            int data = is16Bit ? dest16[i] : dest[i];

            if (is16Bit && (data & 0x8000))
            {
                data = data - 0x10000;
            }

            QTableWidgetItem * dtItem =
                new QTableWidgetItem( dataType );
            QTableWidgetItem * addrItem =
                new QTableWidgetItem ( QString::number( addr+i ) );
            qs_num = QString::number(data);
//            qs_num = QString::asprintf( b_hex ? "%d" : "%d", data);
            QTableWidgetItem * dataItem =
                new QTableWidgetItem( qs_num );
            dtItem->setFlags(dtItem->flags() &
                             ~Qt::ItemIsEditable);
            addrItem->setFlags(addrItem->flags() &
                             ~Qt::ItemIsEditable);
            dataItem->setFlags( dataItem->flags() &
                              ~Qt::ItemIsEditable);

            ui->regTable->setItem(i, DataTypeColumn, dtItem);
            ui->regTable->setItem(i, AddrColumn, addrItem);
            ui->regTable->setItem(i, DataColumn, dataItem);
            samples.append(dataItem->text().toInt());
            samples_x.append(x++);
        }

        ui->regTable->resizeColumnToContents(0);
        ui->debugLabel->setText("Recieve Success");
    }
    else
    {
    ui->debugLabel->setText("Recieve Error");
    }


}

void MainWindow::setStatusError(const QString &msg)
{
    m_statusText->setText( msg );
    m_statusInd->setStyleSheet("background: red");
    m_statusTimer->start( 2000 );
}



void MainWindow::updatePlot()
{
    ui->customPlot->graph(0)->setData(samples_x,samples);
    ui->customPlot->rescaleAxes();
    ui->customPlot->replot();
    ui->customPlot->update();
}

void MainWindow::on_pushButton_clicked()
{
    if (isPlotting)
    {
        timer->stop();
        isPlotting = false;
    }
    else
    {
        timer->start(500);
        isPlotting = true;
    }

   // ui->customPlot->graph(0)->setData(samples_x,samples);
    //ui->customPlot->rescaleAxes();
    //ui->customPlot->replot();
   // ui->customPlot->update();
}


void MainWindow::on_pushButton_2_clicked()
{
    tcpConnect();
    modbus_set_slave(m_tcpModbus,slave);
    addr= 0;

    ret=modbus_write_register(m_tcpModbus,addr,0);     //0,1,2
}


void MainWindow::on_pushButton_3_clicked()
{
    tcpConnect();
    modbus_set_slave(m_tcpModbus,slave);
    addr= 0;

    ret=modbus_write_register(m_tcpModbus, addr,1);     //0,1,2
}


void MainWindow::on_pushButton_4_clicked()
{
    tcpConnect();
    modbus_set_slave(m_tcpModbus,slave);
    addr= 0;

    ret=modbus_write_register(m_tcpModbus, addr,2);     //0,1,2
}


void MainWindow::on_coilNum_textChanged(const QString &arg1)
{

}


void MainWindow::on_coilNum_valueChanged(int arg1)
{

}

