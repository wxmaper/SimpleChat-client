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

#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QWebSocket>
#include <QListWidget>
#include "authdialog.h"

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

    void restoreConnectionData();
    void saveConnectionData();

    enum ItemRole {
        UserIdRole,
        UserNameRole,
        GenderRole,
        UserColorRole
    };

    enum Gender {
        Unknown,
        Male,
        Female
    };

    void connectToServer();
    void closePrivateMessage();
    void privateWithUserFromItem(QListWidgetItem *item);

    QString datetime();

    void onUserAuthorized(int userId,
                          const QString &userName,
                          Gender gender);

    void onUserConnected(int userId,
                         const QString &userName,
                         Gender gender,
                         const QString &userColor);
    void addUser(int userId,
                 const QString &userName,
                 Gender gender,
                 const QString &userColor);
    void addUsers(const QJsonArray &users);

    void onUserDisconnected(int userId,
                            const QString &userName,
                            Gender gender,
                            const QString &userColor);
    void removeUser(int userId);

    void onConnectionLost(int userId,
                          const QString &userName,
                          Gender gender,
                          const QString &userColor);

    void onPublicMessage(int userId,
                         const QString &userName,
                         const QString &userColor,
                         const QString &text);

    void onPrivateMessage(int userId,
                          const QString &userName,
                          const QString &userColor,
                          const QString &text);

public slots:
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError error);
    void onReturnPressed();
    void onAnchorClicked(const QUrl &url);
    void sendPong();
    void onTextMessageReceived(const QString &message);

private:
    Ui::Widget *ui;
    QWebSocket *m_webSocket;
    QTimer *m_pingTimer;

    AuthDialog::ConnectionData m_connectionData;

    int m_toUserId; // кому отправляем сообщение

    int m_userId; // id нашего соединения
    Gender m_gender; // половая принадлежность
    QString m_userName; // имя пользователя
    QString m_usercolor; // цвет имени пользователя
};

#endif // WIDGET_H
