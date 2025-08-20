// server.cpp

#include "server.h"
#include <QThread>

Server::Server(QObject *parent) : QObject(parent)
{
    chatServer = new QTcpServer(this);

    connect(chatServer, &QTcpServer::newConnection, this, &Server::newClientConnection);
}

void Server::startServer()
{
    if (chatServer->listen(QHostAddress("192.168.206.1"), 12345)) {
        qDebug() << "Server listening on port 12345...";
    } else {
        qDebug() << "Error: Unable to start the server.";
    }
}

void Server::sendMessageToClients(QString message)
{
    for (const UserClient &client : allClients) {
        client.socket->write(message.toUtf8());
        client.socket->flush();
    }
}

void Server::sendMessageToUser(const QString& targetLogin, const QString& senderLogin, const QString& message)
{
    if(targetLogin == "Общий"){
        for (UserClient& client : allClients) {
            if (client.Login != senderLogin) {
                client.socket->write(message.toUtf8());
                client.socket->flush();
            }
        }
    }
    for (UserClient& client : allClients) {
        if (client.Login == targetLogin) {
            client.socket->write(message.toUtf8());
            client.socket->flush();
            break;  // Мы нашли нужного пользователя, поэтому прерываем цикл
        }
    }
}



void Server::newClientConnection()
{
    while (chatServer->hasPendingConnections()) {
        QTcpSocket *clientSocket = chatServer->nextPendingConnection();

        connect(clientSocket, &QTcpSocket::disconnected, this, &Server::socketDisconnected);
        connect(clientSocket, &QTcpSocket::readyRead, this, &Server::socketReadyRead);
        connect(clientSocket, &QTcpSocket::stateChanged, this, &Server::socketStateChanged);
        UserClient user;
        user.socket = clientSocket;
        if(user.Login == ""){
            qDebug()<<"error connection";
        }else {
            allClients.append(user);
        }

        qDebug() << "New connection from: " << clientSocket->peerAddress().toString();
    }
}

void Server::socketDisconnected()
{
    QTcpSocket *disconnectedSocket = qobject_cast<QTcpSocket*>(sender());

    for (int i = 0; i < allClients.size(); ++i) {
        if (allClients.at(i).socket == disconnectedSocket) {
            qDebug() << "Client disconnected: " << allClients.at(i).Login;
            allClients.remove(i);
            break;
        }
    }

    disconnectedSocket->deleteLater();
}

void Server::socketReadyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket)
        return;

    QByteArray data = socket->readAll();
    QString receivedMessage = QString::fromUtf8(data);

    // Пример: Сообщение от клиента "Логин:username Никнейм:nickname"
    if (receivedMessage.startsWith("Логин:") && receivedMessage.contains(" Никнейм:")) {
        QStringList parts = receivedMessage.split(" ");
        if (parts.size() >= 2) {  // Проверяем, что список частей достаточно длинный
            QString loginToCheck = parts.at(0).mid(6); // Извлекаем логин после "Логин:"
            QString nickname = parts.at(1).mid(8);  // Извлекаем никнейм после "Никнейм:"

            bool loginExists = false;
            for (const UserClient &client : allClients) {
                if (client.Login == loginToCheck) {
                    loginExists = true;
                    break;
                }
            }

            // Отправляем результат клиенту
            if (loginExists) {
                socket->write("false");
            } else {
                UserClient newUser;
                newUser.socket = socket;
                newUser.Login = loginToCheck;
                newUser.NickName = nickname;
                allClients.append(newUser);
                socket->write("true");
                QString updatedLogins = getUserLoginsString();
                socket->write(("UpdatedLogins:" + updatedLogins + "\n").toUtf8());
                sendMessageToClients("NewUser:" + newUser.Login);
            }


            socket->flush();
        } else {
            // Некорректный формат сообщения, обработайте ошибку по вашему усмотрению
            qDebug() << "Invalid message format";
        }

    }
    if (receivedMessage.startsWith("PrivateMessage:")) {
        QStringList parts = receivedMessage.split(" ");
        if (parts.size() >= 3) {
            QString senderLogin = parts.at(1);
            QString targetLogin = parts.at(2);
            QString message = parts.mid(3).join(" ");
            if(targetLogin == "Общий"){
                sendMessageToUser(targetLogin, senderLogin, "CommonMessage from " + senderLogin + ": " + message);
            }else{ sendMessageToUser(targetLogin, senderLogin, "PrivateMessage from " + senderLogin + ": " + message);}
            // Отправляем сообщение конкретному пользователю

        }
        socket->flush();
    }
    if (receivedMessage.startsWith("ClosedDialog:")) {
        QString closedLogin = receivedMessage.mid(13);
        handleDialogClosed(closedLogin);
    }
    if (receivedMessage == "GetLogins") {
        QString loginsString = getUserLoginsString();
        socket->write(loginsString.toUtf8());
        socket->flush();
    }
    // Другие типы сообщений обрабатываются здесь...
}

void Server::socketStateChanged(QAbstractSocket::SocketState state)
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket)
        return;

    qDebug() << "Socket state changed to: " << state;
}

QString Server::getUserLoginsString() const
{
    QStringList logins;
    for (const UserClient &client : allClients) {
        logins.append(client.Login);
    }
    return logins.join(",");
}

void Server::updateComboBox(const QString& newLogin)
{
    emit loginUpdated(newLogin);

}

void Server::handleDialogClosed(const QString& closedLogin)
{
    // Удалите пользователя из вектора на сервере
    auto it = std::remove_if(allClients.begin(), allClients.end(),
                             [closedLogin](const UserClient& client) {
                                 return client.Login == closedLogin;
                             });
    allClients.erase(it, allClients.end());

    // Оповестите остальных клиентов о закрытии окна dialog

    // Отправите сообщение клиенту, чтобы тот удалил пользователя из comboBox
    sendUpdateComboBoxMessage();
}

void Server::sendUpdateComboBoxMessage()
{
    QString updatedLogins = getUserLoginsString();
    for (const UserClient& client : allClients) {
        client.socket->write(("UpdatedLogins:" + updatedLogins + "\n").toUtf8());
        client.socket->flush();
    }
}
