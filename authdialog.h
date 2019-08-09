#ifndef AUTHDIALOG_H
#define AUTHDIALOG_H

#include <QDialog>

namespace Ui {
class AuthDialog;
}

class AuthDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AuthDialog(QWidget *parent = nullptr);
    ~AuthDialog();

    struct ConnectionData
    {
        QString server;
        int port;
        QString userName;
        int gender;
        QString userColor;
    };

    void setConnectionData(const ConnectionData &connectionData);
    ConnectionData connectionData() const;

private:
    Ui::AuthDialog *ui;
};

#endif // AUTHDIALOG_H
