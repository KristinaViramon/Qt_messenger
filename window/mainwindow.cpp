#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QTcpSocket>
#include "dialog.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
     dialog(nullptr),
 socket(new QTcpSocket(this)),
  userLogin()
{
    ui->setupUi(this);

    // Создаем сокет для клиента
    socket = new QTcpSocket(this);
    // Подключаем сигналы и слоты для обработки событий сокета
    connect(socket, &QTcpSocket::connected, this, &MainWindow::onSocketConnected);
    connect(socket, &QTcpSocket::disconnected, this, &MainWindow::onSocketDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::onSocketReadyRead);

    // Подключаем к серверу
    socket->connectToHost("192.168.206.1", 12345); // Укажите IP-адрес и порт сервера

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_door_clicked()
{
     userLogin = ui->Login->text();
    QString nickName = ui->pass->text();
    if (socket->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "Socket is not connected!";
        return;
    }
    // Отправляем логин и никнейм на сервер
    QString message = "Логин:" + userLogin + " Никнейм:" + nickName;
    socket->write(message.toUtf8());
    socket->flush();
}

void MainWindow::onSocketConnected()
{
    qDebug() << "Connected to server";
}

void MainWindow::onSocketDisconnected()
{
    qDebug() << "Disconnected from server";
}


QStringList MainWindow::getLoginsFromServer()
{
    // Отправляем запрос на сервер
    socket->write("GetLogins");
    socket->waitForBytesWritten();  // Ждем, пока данные будут отправлены

    // Ждем ответ от сервера
    if (socket->waitForReadyRead()) {
        QByteArray data = socket->readAll();
        return QString::fromUtf8(data).split(',');  // Предполагаем, что логины разделены запятыми
    }

    return QStringList();  // Возвращаем пустой список в случае ошибки или отсутствия данных
}

void MainWindow::onSocketReadyRead()
{
    QByteArray data = socket->readAll();
    QString response = QString::fromUtf8(data);


    // Обработка ответа от сервера
    if (response.startsWith("true")) {
        QStringList updatedLogins = response.mid(18).split("\n").first().split(',');
        dialog =new Dialog(updatedLogins, this);

        if (dialog) {
            qDebug() << "Sending sendMessageToDialog signal...";
        } else {
            qDebug() << "Dialog is not initialized!";
        }
        // Подключаем сигнал newUserAdded к слоту handleNewUser
        connect(this, &MainWindow::newUserAdded, dialog, &Dialog::handleNewUser);
        connect(dialog, &Dialog::sendPrivateMessageToServer, this, &MainWindow::sendPrivateMessage);
        connect(this, &MainWindow::sendMessageToDialog, dialog, &Dialog::receiveMessageFromMainWindow);
        connect(dialog, &Dialog::dialogClosed, this, &MainWindow::handleDialogClosed);
        connect(this, &MainWindow::updateComboBoxSignal, dialog, &Dialog::updateComboBox);
        dialog -> show();

    } else if (response == "false") {
        ui->statusbar->showMessage("Неверные данные =(");
    }
    else if (response.startsWith("NewUser:")) {
        // Обновление списка логинов в comboBox при появлении нового пользователя
        QString newLogin = response.mid(8);
        newLogin.remove("NewUser");
        newLogin = newLogin.split(":").first();
        emit newUserAdded(newLogin);
    }else if (response.startsWith("PrivateMessage")) {
        QStringList parts = response.split(" ");
        if (parts.size() >= 3) {
            QString senderLogin = parts[2];
            QString message = parts.mid(3).join(" ");
            emit sendMessageToDialog("PrivateMessage: " + senderLogin + " " + message);
        }
        // Далее можно что-то делать с сообщением, например, отобразить в окне

    }else if(response.startsWith("CommonMessage")){
        QStringList parts = response.split(" ");
        if (parts.size() >= 3) {
            QString senderLogin = parts[2];
            QString message = parts.mid(3).join(" ");
            emit sendMessageToDialog("CommonMessage: " + senderLogin + " " + message);
        }
    } else if (response.startsWith("UpdatedLogins:")) {
        QStringList updatedLogins = response.mid(14).split("\n").first().split(",");
        emit updateComboBoxSignal(updatedLogins);
    }
}

void MainWindow::sendPrivateMessage(const QString& targetLogin, const QString& message)
{
   // Формируем сообщение в формате "PrivateMessage: [targetLogin] [message]"
    QString privateMessage = "PrivateMessage: " + userLogin + " " + targetLogin + " "  + message + " ";

    // Отправляем сообщение на сервер
    socket->write(privateMessage.toUtf8());
    socket->flush();
}

void MainWindow::handleDialogClosed()
{
    // Отправите сообщение на сервер о закрытии окна dialog и передайте логин пользователя
    QString closeMessage = "ClosedDialog:" + userLogin;
    socket->write(closeMessage.toUtf8());
    socket->flush();
}
