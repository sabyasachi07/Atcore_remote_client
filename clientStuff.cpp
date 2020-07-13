#include "clientStuff.h"
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(ATCORE_CLIENT, "org.kde.atelier.core.client")

ClientStuff::ClientStuff(
        const QString hostAddress,
        int portNumber,
        QObject *parent
        ) : QObject(parent), m_nNextBlockSize(0)


{
    status = false;

               host  =hostAddress;
               port= portNumber;

     server = new QSslSocket;
     connect(server, &QSslSocket::disconnected, this, &ClientStuff::closeConnection);
     connect(server, &QSslSocket::encrypted, this, &ClientStuff::encrypted);
     connect(server, &QSslSocket::connected, this, &ClientStuff::connected);
     connect(server, &QSslSocket::readyRead, this, &ClientStuff::readyRead);

     connect(server, &QSslSocket::encryptedBytesWritten,this, &ClientStuff::encryptedBytesWritten);
     connect(server,QOverload<const QList<QSslError> &>::of(&QSslSocket::sslErrors),this,&ClientStuff::sslErrors);
     server->addCaCertificates(QStringLiteral("/home/trex108/code/server/server.crt"));
     server->setPrivateKey(QStringLiteral("/home/trex108/code/client/client.key"));
     server->setLocalCertificate(QStringLiteral("/home/trex108/code/client/client.crt"));
     server->setPeerVerifyMode(QSslSocket::VerifyPeer);
     server->setProtocol(QSsl::TlsV1SslV3);

     timeoutTimer = new QTimer();
     timeoutTimer->setSingleShot(true);
     connect(timeoutTimer, &QTimer::timeout, this, &ClientStuff::connectionTimeout);
}

void ClientStuff::connectTohost()
{
    timeoutTimer->start(3000);

      server->connectToHostEncrypted(host,port);

      if (server->waitForEncrypted(5000)) {

         qCDebug(ATCORE_CLIENT) << (tr(" connected to server"));

      } else {
          qCDebug(ATCORE_CLIENT) << (tr("Unable to connect to server"));

          exit(0);
      }

}



void ClientStuff::connectionTimeout()
{
    qCDebug(ATCORE_CLIENT) << server->state();
    if(server->state() == QAbstractSocket::ConnectingState)
    {
        server->abort();
        emit server->error(QAbstractSocket::SocketTimeoutError);
    }
}

void ClientStuff::sslErrors(const QList<QSslError> &errors)
{
    foreach (const QSslError &error, errors)
        qCDebug(ATCORE_CLIENT) << error.errorString();
}



void ClientStuff::encrypted()
{
    qCDebug(ATCORE_CLIENT) << tr("Encrypted") <<server;
    if (!server)
           return;
}

void ClientStuff::encryptedBytesWritten(qint64 written)
{
    qCDebug(ATCORE_CLIENT) << tr("encryptedBytesWritten") << server << written;

}

void ClientStuff::connected()
{
    status = true;
    emit statusChanged(status);
}

bool ClientStuff::getStatus() {return status;}

void ClientStuff::readyRead()
{
    QDataStream in(server);
    //in.setVersion(QDataStream::Qt_5_10);
  for(;;)
    {
        if (!m_nNextBlockSize)
        {
            if (server->bytesAvailable() < static_cast<qint64>(sizeof(quint16))) { break; }
            in >> m_nNextBlockSize;
        }

        if (server->bytesAvailable() < m_nNextBlockSize) { break; }

        QString str; in >> str;

        if (str == "NULL")
        {
            str = "Connection closed";
            closeConnection();
        }

        emit hasReadSome(str);
        m_nNextBlockSize = 0;
    }
}



void ClientStuff::closeConnection()
{
    timeoutTimer->stop();

    //qDebug() << server->state();
    disconnect(server, &QSslSocket::connected,0,0);
    disconnect(server, &QSslSocket::readyRead,0,0);

    bool shouldEmit;
    switch (server->state())
    {
        case 0:
            server->disconnectFromHost();
            shouldEmit = false;
            break;
        case 2:
            server->abort();
            shouldEmit = false;
            break;
        default:
            server->abort();
    }

    if(shouldEmit)
     {
         status=false;
         emit statusChanged(status);
      }
}


