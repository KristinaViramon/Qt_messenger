#include "dialog.h"
#include "ui_dialog.h"
#include<QTextEdit>
#include "mainwindow.h"
#include<QCloseEvent>
#include<QMessageBox>
Dialog::Dialog(const QStringList& logins, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
    ui->comboBox->addItem("Общий");
    // Добавляем логины в comboBox
    ui->comboBox->addItems(logins);
    ui->textEdit->setReadOnly(true);
    connect(ui->comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Dialog::onComboBoxIndexChanged);
    // Подключаем слот для приема сообщений от MainWindow
    connect(this, &Dialog::sendMessageToDialog, this, &Dialog::receiveMessageFromMainWindow);

}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::onComboBoxIndexChanged(int index)
{
    if (index >= 0 && index < ui->comboBox->count()) {
        QString selectedLogin = ui->comboBox->itemText(index);
        // Проверяем, что textEdit не пустой
        if (!ui->textEdit->toPlainText().isEmpty()) {
            ui->textEdit->clear();
        }

        QStringList valuesForKey = dialogBuff.values(selectedLogin+":");
        foreach (const QString &value, valuesForKey) {
            ui->textEdit->append(value);
        }

    }

}


void Dialog::handleNewUser(const QString& newLogin)
{
    ui->comboBox->addItem(newLogin);
}

void Dialog::on_sendButton_clicked()
{
    QString userText = ui->input->text();

    // Формируем строку в формате "Я: текст"
    QString formattedText = "Я: " + userText;
    ui->input->clear();
    // Добавляем сформированный текст в TextEdit
    ui->textEdit->append(formattedText);
    QString selectedLogin = ui->comboBox->currentText();
    dialogBuff.insert(selectedLogin+":", formattedText);
    // Формируем сообщение в формате "Текст сообщения: [userText]"
    emit sendPrivateMessageToServer(selectedLogin, userText);
}

void Dialog::receiveMessageFromMainWindow(const QString& message)
{
    QMessageBox::StandardButton reply;
    // Обработка сообщения от MainWindow
    // Предполагается, что сообщение имеет формат "PrivateMessage: [отправитель] [сообщение]"
    QStringList parts = message.split(" ");
    QString type = parts[0];
    QString senderLogin = parts[1];
    QString textMessage = parts.mid(2).join(" ");
    // Выводим сообщение в формате "Отправитель: текст сообщения"
    QString formattedMessage = senderLogin + textMessage;
    QString selectedLogin = ui->comboBox->currentText();
    if(type == "PrivateMessage:"){
        reply = QMessageBox::information(this,
                                 "Сообщение в чате от",
                                 senderLogin + textMessage);
        if(senderLogin == selectedLogin + ":"){
            ui->textEdit->append(formattedMessage);
            dialogBuff.insert(senderLogin, formattedMessage);
        }else{dialogBuff.insert(senderLogin, formattedMessage);}
    }

    if(type == "CommonMessage:"){
       reply = QMessageBox::information(this,
                                 "Сообщение в общем чате от",
                                 senderLogin + textMessage);
        if(selectedLogin=="Общий"){
            dialogBuff.insert("Общий:", formattedMessage);
            ui->textEdit->append(formattedMessage);
        }else{dialogBuff.insert("Общий:", formattedMessage);}
    }
}

void Dialog::closeEvent(QCloseEvent *event)
{
    // Оповестите MainWindow о закрытии окна dialog и передайте логин пользователя
    emit dialogClosed();
}

void Dialog::updateComboBox(const QStringList& newLogins)
{
    // Обновите содержимое comboBox новыми логинами
    ui->comboBox->clear();
    ui->comboBox->addItem("Общий");
    ui->comboBox->addItems(newLogins);
}
