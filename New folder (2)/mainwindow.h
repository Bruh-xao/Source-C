#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "imodbus.h"
#include <QLabel>
#include <QVector>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow, public IModbus
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // IModbus Interface
    virtual modbus_t *modbus() { return m_tcpModbus; }
    virtual int setupModbusPort();
    void tcpConnect();


protected:
    void changeModbusInterface(const QString& address, int portNbr);
    void releaseTcpModbus();
    void enableTCPEdit(bool checked);

private slots:
    void on_tcpEnable_clicked(bool checked);
    void sendTcpRequest( void );
    void setStatusError(const QString &msg);

    void on_pushButton_clicked();

    void updatePlot();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

signals:
    void tcpPortActive(bool val);
    void connectionError(const QString &msg);

private:
    Ui::MainWindow *ui;
    modbus_t*   m_tcpModbus;
    modbus_t* m_modbus;
    QTimer *refreshRate;
    QTimer *timer;
    bool isPlotting;

    QWidget * m_statusInd;
    QLabel * m_statusText;
    QTimer * m_pollTimer;
    QTimer * m_statusTimer;
    bool m_tcpActive;
    bool m_poll;
    QVector <double> samples;
    QVector <double> samples_x;
    int x = 0;
    int temp;
    int addr;
    int ret;
    int slave;
    int func;




};
#endif // MAINWINDOW_H
