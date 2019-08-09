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
