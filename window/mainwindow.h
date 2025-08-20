// Ваш файл MainWindow.h

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>  // Добавлен заголовок для использования QTcpSocket
#include "dialog.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
signals:
    void newUserAdded(const QString& newLogin);
    void sendMessageToDialog(const QString& message);
     void updateComboBoxSignal(const QStringList& newLogins);
private:
    Ui::MainWindow *ui;
    Dialog *dialog;
    QTcpSocket *socket;  // Добавлен указатель на объект QTcpSocket
    QStringList getLoginsFromServer();
    QString userLogin;
public slots:
  void sendPrivateMessage(const QString& targetLogin, const QString& message);
  void handleDialogClosed();
private slots:
    void on_door_clicked();
    void onSocketConnected();  // Добавлен слот для обработки события подключения к серверу
    void onSocketDisconnected();  // Добавлен слот для обработки события отключения от сервера
    void onSocketReadyRead();  // Добавлен слот для обработки готовности чтения данных от серверая
};

#endif // MAINWINDOW_H
