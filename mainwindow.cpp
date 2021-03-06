#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->pushButton_disconnect->setVisible(false);
    setWindowTitle(tr("AtCore - Remote Client"));


    client = new ClientStuff("127.0.0.1",38917);

    setStatus(client->getStatus());

    connect(client, &ClientStuff::hasReadSome, this, &MainWindow::receivedSomething);
    connect(client, &ClientStuff::statusChanged, this, &MainWindow::setStatus);
    connect(client->server, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &MainWindow::gotError);
}

MainWindow::~MainWindow()
{
    delete client;
    delete ui;
}


void MainWindow::setStatus(bool newStatus)
{
    if(newStatus)
    {
        ui->label_status->setText(tr("<font color=\"green\">CONNECTED</font>"));
        ui->textEdit_log->append(tr("<font color=\"green\"><b>client is connected.</b></font>"));
        ui->pushButton_connect->setVisible(false);
        ui->pushButton_disconnect->setVisible(true);
    }
    else
    {
        ui->label_status->setText(
                tr("<font color=\"red\">DISCONNECTED</font>"));

        ui->pushButton_connect->setVisible(true);
        ui->pushButton_disconnect->setVisible(false);
    }
}

void MainWindow::receivedSomething(QString msg)
{
    ui->textEdit_log->append(msg);
}

void MainWindow::gotError(QAbstractSocket::SocketError err)
{

    QString strError = "unknown";
    switch (err)
    {
        case 0:
            strError = "Connection was refused";
            break;
        case 1:
            strError = "Remote host closed the connection";
            break;
        case 2:
            strError = "Host address was not found";
            break;
        case 5:
            strError = "Connection timed out";
            break;
        default:
            strError = "Unknown error";
    }

    ui->textEdit_log->append(strError);
}

void MainWindow::on_pushButton_connect_clicked()
{
    client->connectTohost();
}

void MainWindow::on_pushButton_send_clicked()
{
    QByteArray arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);

    out << quint16(0) << ui->lineEdit_message->text();

    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

    client->server->write(arrBlock);
}

void MainWindow::on_pushButton_disconnect_clicked()
{
    client->closeConnection();
    ui->textEdit_log->append(tr("<font color=\"red\"><b>client is disconnected</b>.</font>"));

}
