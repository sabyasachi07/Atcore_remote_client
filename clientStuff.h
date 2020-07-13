

#include <QString>
#include <QTcpSocket>
#include <QDataStream>
#include <QTimer>
#include <QMessageBox>
#include <QSslSocket>

class ClientStuff : public QObject
{
    Q_OBJECT

public:
    ClientStuff(const QString hostAddress, int portVal,QObject *parent = 0);

    QSslSocket *server;
    bool getStatus();

public slots:
    void closeConnection();
    void connectTohost();

signals:
    void statusChanged(bool);
    void hasReadSome(QString msg);

private slots:
    void readyRead();
    void connected();
    void connectionTimeout();
    void sslErrors(const QList<QSslError> &errors);
    void encrypted();
    void encryptedBytesWritten(qint64 written);


private:
    QString host;
    int port;
    bool status;
    quint16 m_nNextBlockSize;
    QTimer *timeoutTimer;
};

