#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDebug>
#include <QVector>

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = nullptr);
    QString getUserLoginsString() const;
    void startServer();
    void sendMessageToClients(QString message);
    void sendMessageToUser(const QString& targetLogin,const QString& senderLogin, const QString& message);  // Новый метод для отправки сообщения конкретному пользователю
    void handleDialogClosed(const QString& closedLogin);
    void sendUpdateComboBoxMessage();

signals:
    void loginUpdated(const QString& newLogin);

public slots:
    void newClientConnection();
    void socketDisconnected();
    void updateComboBox(const QString& newLogin);
    void socketReadyRead();
    void socketStateChanged(QAbstractSocket::SocketState state);

private:
    QTcpServer*             chatServer;
    struct UserClient{
        QTcpSocket* socket;
        QString Login;
        QString NickName;
    };
    QVector<UserClient>   allClients;

};

#endif // SERVER_H
