#ifndef KLIENT_H
#define KLIENT_H
#include <QDialog>
#include <QWidget>
#endif // KLIENT_H

class Dialog : public QDialog
{
private slots:
    //определим слоты для обработки сигналов сокета
    void onSokConnected();
    void onSokDisconnected();
    //сигнал readyRead вызывается, когда сокет получает пакет (который может быть лишь частью отправленых данных) байтов
    void onSokReadyRead();
    void onSokDisplayError(QAbstractSocket::SocketError socketError);
private:
    QTcpSocket *_sok; //сокет
    quint16 _blockSize;//текущий размер блока данных
    QString _name;//имя клиента
};
