/*******************************************************************************
 * MIT License
 *
 * This file is part of the SimpleChat project:
 * https://github.com/wxmaper/SimpleChat-client
 *
 * Copyright (c) 2019 Aleksandr Kazantsev (https://wxmaper.ru)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "widget.h"
#include "ui_widget.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QSettings>
#include <QUrlQuery>
#include <QJsonArray>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget),
    m_webSocket(new QWebSocket(QString("SimpleChatClient"),
                               QWebSocketProtocol::Version13,
                               this)),
    m_pingTimer(new QTimer(this))
{
    ui->setupUi(this);
    ui->textBrowser->setContextMenuPolicy(Qt::NoContextMenu);

    // По умолчанию мы отправляем сообщения в общий чат
    closePrivateMessage();

    // Обработка клика по кнопке закрытия приватного режима
    connect(ui->toolButton_closePrivateMessage, &QToolButton::clicked,
            this, &Widget::closePrivateMessage);

    // Обработка двойного клика по пользователю из списка.
    // При двойном клике мы активируем отправку приватного сообщения.
    connect(ui->listWidget_users, &QListWidget::itemDoubleClicked,
            this, &Widget::privateWithUserFromItem);

    // Блокируем поле ввода сообщения до тех пор,
    // пока соединение с сервером не будет установлено
    ui->lineEdit_message->setEnabled(false);

    // Соединяем сигнал нажатия кнопки Return со слотом отправки сообщения
    connect(ui->lineEdit_message, &QLineEdit::returnPressed,
            this, &Widget::onReturnPressed);

    // Обработка клика по ссылкам
    connect(ui->textBrowser, &QTextBrowser::anchorClicked,
            this, &Widget::onAnchorClicked);

    // Таймер для пингования сервера, чтобы указать, что соединение все еще живо
    connect(m_pingTimer, SIGNAL(timeout()), m_webSocket, SLOT(ping()));

    // Соединяем сигналы websocket-клиента
    // Подключение к серверу
    connect(m_webSocket, &QWebSocket::connected,
            this, &Widget::onConnected);

    // Отключение от сервера
    connect(m_webSocket, &QWebSocket::disconnected,
            this, &Widget::onDisconnected);

    // Ошибки сокета
    connect(m_webSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(onError(QAbstractSocket::SocketError)));

    // Получение сообщения с сервера
    connect(m_webSocket, &QWebSocket::textMessageReceived,
            this, &Widget::onTextMessageReceived);

    restoreConnectionData();
}

Widget::~Widget()
{
    m_webSocket->close();
    delete ui;
}

void Widget::saveConnectionData()
{
    QSettings settings(qApp->applicationDirPath() + "/settings.ini",
                       QSettings::IniFormat);

    settings.setValue("server", m_connectionData.server);
    settings.setValue("port", m_connectionData.port);
    settings.setValue("userName", m_connectionData.userName);
    settings.setValue("gender", m_connectionData.gender);
    settings.setValue("userColor", m_connectionData.userColor);
}

void Widget::restoreConnectionData()
{
    QSettings settings(qApp->applicationDirPath() + "/settings.ini",
                       QSettings::IniFormat);

    m_connectionData.server = settings.value("server", "127.0.0.1").toString();
    m_connectionData.port = settings.value("port", 27800).toInt();
    m_connectionData.userName = settings.value("userName", "Инкогнито").toString();
    m_connectionData.gender = settings.value("gender", 0).toInt();
    m_connectionData.userColor = settings.value("userColor", "#34495e").toString();
}

void Widget::connectToServer()
{
    AuthDialog authDialog(this);
    authDialog.setConnectionData(m_connectionData);

    int result = authDialog.exec();

    if (result == AuthDialog::Accepted) {
        m_connectionData = authDialog.connectionData();

        QString html = QString("%1 <span style='color:#7f8c8d'>"
                               "<i>Установка соединения с <b>%2:%3</b>...</span>")
                .arg(datetime())
                .arg(m_connectionData.server)
                .arg(m_connectionData.port);
        ui->textBrowser->append(html);

        m_webSocket->open(QUrl(QString("ws://%1:%2?userName=%3&userColor=%4&gender=%5")
                               .arg(m_connectionData.server)
                               .arg(m_connectionData.port)
                               .arg(m_connectionData.userName)
                               .arg(QString(m_connectionData.userColor).replace("#","%23"))
                               .arg(m_connectionData.gender)));
    }
    else {
        close();
        qApp->quit();
    }
}

void Widget::closePrivateMessage()
{
    // "0" указывает на то, что отправляем сообщение в общий чат
    m_toUserId = 0;
    ui->toolButton_closePrivateMessage->hide();

    ui->label_receiver->setText(QString("Отправить в общий чат"));
}

void Widget::privateWithUserFromItem(QListWidgetItem *item)
{
    m_toUserId = item->data(UserIdRole).toInt();
    ui->toolButton_closePrivateMessage->show();

    QString toUserName = item->data(UserNameRole).toInt();
    ui->label_receiver->setText(QString("Отправить пользователю <b>%1</b>")
                                .arg(toUserName));
}

QString Widget::datetime()
{
    QString html = QString("<span style='color:#34495e'><b>[%1]</b></span>")
            .arg(QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm:ss"));
    return html;
}

void Widget::onUserAuthorized(int userId,
                              const QString &userName,
                              Gender gender)
{
    // При авторизации сохраняем данные "о себе"
    m_userId = userId;
    m_userName = userName;
    m_gender = gender;

    QString html = QString("%1 <span style='color:#7f8c8d'>"
                           "<i>Вы авторизованы с именем <b>%2</b></span>")
            .arg(datetime())
            .arg(userName);
    ui->textBrowser->append(html);
}

void Widget::onUserConnected(int userId,
                           const QString &userName,
                           Widget::Gender gender,
                           const QString &userColor)
{
    // При подключении нового пользователя, добавляем его в список
    addUser(userId, userName, gender, userColor);

    // Добавляем сообщение о входе
    QString html = QString("%1 <span style='color:#7f8c8d'>"
                           "<i><b><a style='color:%2' href='action://putUserName?userName=%3&userId=%5'>%3</a></b>"
                           " %4 в чат</i></span>")
            .arg(datetime())
            .arg(userColor)
            .arg(userName)
            .arg(gender == 2 ? "вошла" : "вошёл")
            .arg(userId);

    ui->textBrowser->append(html);
}

void Widget::addUser(int userId,
                     const QString &userName,
                     Widget::Gender gender,
                     const QString &userColor)
{
    QListWidgetItem *item = new QListWidgetItem;
    item->setData(UserIdRole, userId);
    item->setData(UserNameRole, userName);
    item->setData(GenderRole, gender);
    item->setData(UserColorRole, userColor);
    item->setData(Qt::TextColorRole, QColor(userColor));
    item->setText(userName);
    item->setIcon(QIcon(QString(":/icons/gender-%1.png").arg(gender)));

    ui->listWidget_users->addItem(item);
}

void Widget::addUsers(const QJsonArray &users)
{
    foreach (QJsonValue v, users) {
        QJsonObject user = v.toObject();
        int userId = user.value("userId").toInt();
        QString userName = user.value("userName").toString();
        Gender gender = Gender(user.value("gender").toInt());
        QString userColor = user.value("userColor").toString();

        if (userId == m_userId) {
            continue;
        }

        addUser(userId, userName, gender, userColor);
    }
}

void Widget::onUserDisconnected(int userId, const QString &userName, Widget::Gender gender, const QString &userColor)
{
    // Удаляем из списка вышедшего пользователя
    removeUser(userId);

    // Добавляем сообщение о выходе
    QString html = QString("%1 <span style='color:#7f8c8d'>"
                           "<i><b><a style='color:%2' href='action://putUserName?userName=%3&userId=%5'>%3</a></b>"
                           " %4 из чата</i></span>")
            .arg(datetime())
            .arg(userColor)
            .arg(userName)
            .arg(gender == 2 ? "вышла" : "вышел")
            .arg(userId);

    ui->textBrowser->append(html);
}

void Widget::removeUser(int userId)
{
    for (int i = 0; i < ui->listWidget_users->count(); i++) {
        QListWidgetItem *item = ui->listWidget_users->item(i);
        if (item->data(UserIdRole).toInt() == userId) {
            delete item;
            break;
        }
    }
}

void Widget::onConnectionLost(int userId,
                              const QString &userName,
                              Widget::Gender gender,
                              const QString &userColor)
{
    Q_UNUSED(gender);

    // Удаляем из списка отвалившегося пользователя
    removeUser(userId);

    // Добавляем сообщение
    QString html = QString("%1 <span style='color:#7f8c8d'>"
                           "<i>Соединение с <b style='color:%2'>%3</b>"
                           " потеряно</i></span>")
            .arg(datetime())
            .arg(userColor)
            .arg(userName);

    ui->textBrowser->append(html);
}

void Widget::onPublicMessage(int userId,
                             const QString &userName,
                             const QString &userColor,
                             const QString &text)
{
    if (text.contains("<b>" + m_userName + "</b>")) {
        qApp->beep();
        qApp->alert(this);
    }

    QString html = QString("%1 <b><a style='color:%2' href='action://putUserName?userName=%3&userId=%5'>%3:</a></b>"
                           " <span style='color:#34495e'>%4</span>")
            .arg(datetime())
            .arg(userColor)
            .arg(userName)
            .arg(text)
            .arg(userId);

    ui->textBrowser->append(html);
}

void Widget::onPrivateMessage(int userId,
                              const QString &userName,
                              const QString &userColor,
                              const QString &text)
{
    qApp->beep();
    qApp->alert(this);

    QString html = QString("%1 <b>%6</b> <b><a style='color:%2' href='action://putUserName?userName=%3&userId=%5'>%3:</a></b>"
                           " <span style='color:#34495e'>%4</span>")
            .arg(datetime())
            .arg(userColor)
            .arg(userName)
            .arg(text)
            .arg(userId)
            .arg(userId == m_userId ? "&lt;" : "&gt;");

    ui->textBrowser->append(html);
}

void Widget::onConnected()
{
    m_pingTimer->start(15 * 1000); // пингуем сервер каждые 15 сек

    QString html = QString("%1 <span style='color:#16a085'><i>Соединение установлено!</i></span>")
            .arg(datetime());
    ui->textBrowser->append(html);
    ui->lineEdit_message->setEnabled(true);
    saveConnectionData();
}

void Widget::onDisconnected()
{
    m_pingTimer->stop();
    ui->listWidget_users->clear();

    QString html = QString("%1 <span style='color:#c0392b'><i>Соединение разорвано.</i></span>")
            .arg(datetime());
    ui->textBrowser->append(html);
    ui->lineEdit_message->setEnabled(false);

    // Через пять сек мы снова пытаемся соединиться с сервером
    QTimer::singleShot(5000, this, &Widget::connectToServer);
}

void Widget::onError(QAbstractSocket::SocketError error)
{
    m_pingTimer->stop();
    ui->listWidget_users->clear();

    QString html = QString("%1 <span style='color:#c0392b'>Ошибка сокета №%2: %3</span>")
            .arg(datetime())
            .arg(error)
            .arg(m_webSocket->errorString());
    ui->textBrowser->append(html);
    ui->lineEdit_message->setEnabled(false);
}

void Widget::onReturnPressed()
{
    // Достаём сообщение из поля ввода, удалив лишние пробелы
    QString text = ui->lineEdit_message->text().trimmed();

    // Очищаем поле ввода сообщения
    ui->lineEdit_message->clear();

    // Пустые сообщения не отправляем
    if (text.isEmpty()) {
        return;
    }

    // Собираем данные сообщения
    QJsonObject messageData;
    messageData.insert("toUserId", m_toUserId);
    messageData.insert("text", text);

    // Преобразуем JSON-объект в строку
    QByteArray message = QJsonDocument(messageData).toJson(QJsonDocument::Compact);

    // Отправляем данные
    m_webSocket->sendTextMessage(message);
}

void Widget::onAnchorClicked(const QUrl &url)
{
    // При клике на имя пользователя, вставляем его в поле ввода сообщения
    QUrlQuery query(url);

    QString text = ui->lineEdit_message->text();
    if (text.isEmpty()) {
        text = "{" + query.queryItemValue("userName") + "}, ";
    }
    else if (text.endsWith(' ')) {
        text += "{" + query.queryItemValue("userName") + "} ";
    }
    else {
        text += " {" + query.queryItemValue("userName") + "} ";
    }

    ui->lineEdit_message->setText(text);
    ui->lineEdit_message->setFocus();
}

void Widget::sendPong()
{
    QJsonObject messageData;
    messageData.insert("action", "Pong");
    QByteArray message = QJsonDocument(messageData).toJson(QJsonDocument::Compact);
    m_webSocket->sendTextMessage(message);
}

void Widget::onTextMessageReceived(const QString &message)
{
    // Преобразуем полученное сообщение в JSON-объект
    QJsonObject messageData = QJsonDocument::fromJson(message.toUtf8()).object();

    QString action = messageData.value("action").toString();

    if (action == "Ping") {
        // В ответ на "Ping" клиент должен послать действие "Pong",
        // чтобы сервер понял, что клиент в онлайне
        sendPong();
    }
    else {
        int userId = messageData.value("userId").toInt();
        QString userName = messageData.value("userName").toString();
        Gender gender = Gender(messageData.value("gender").toInt());
        QString userColor = messageData.value("userColor").toString();

        if (action == "Authorized") {
            onUserAuthorized(userId, userName, gender);
            QJsonArray users = messageData.value("users").toArray();
            addUsers(users);
        }

        else if (action == "Connected") {
            onUserConnected(userId, userName, gender, userColor);
        }

        else if (action == "Disconnected") {
            onUserDisconnected(userId, userName, gender, userColor);
        }

        else if (action == "ConnectionLost") {
            onConnectionLost(userId, userName, gender, userColor);
        }

        else if (action == "PublicMessage") {
            QString text = messageData.value("text").toString();
            onPublicMessage(userId, userName, userColor, text);
        }

        else if (action == "PrivateMessage") {
            QString text = messageData.value("text").toString();
            onPrivateMessage(userId, userName, userColor, text);
        }

        else {
            // неизвестное действие
            qWarning() << "unknown action: " << action;
        }
    }
}
