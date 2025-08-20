// Ваш файл Dialog.h

#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QStringList>
#include <QTcpSocket>  // Добавлен заголовок для использования QTcpSocket
#include<QMultiMap>
QT_BEGIN_NAMESPACE
namespace Ui {
class Dialog;
}
QT_END_NAMESPACE

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(const QStringList& logins, QWidget *parent = nullptr);
    ~Dialog();

    // Новый метод для установки сокета
    void setSocket(QTcpSocket* socket);

public slots:
    void handleNewUser(const QString& newLogin);
    void receiveMessageFromMainWindow(const QString& message);
    void updateComboBox(const QStringList& newLogins);

signals:
    void sendMessageToDialog(const QString& message);
    void sendPrivateMessageToServer(const QString& targetLogin, const QString& message);
    void dialogClosed();

private slots:
    void on_sendButton_clicked();
     void onComboBoxIndexChanged(int index);

private:
    Ui::Dialog *ui;
    QTcpSocket* socket;  // Добавлен указатель на объект QTcpSocket
    QMultiMap<QString,QString> dialogBuff;

protected:
    void closeEvent(QCloseEvent *event) override;

};

#endif // DIALOG_H
