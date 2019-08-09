#include "authdialog.h"
#include "ui_authdialog.h"

AuthDialog::AuthDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AuthDialog)
{
    ui->setupUi(this);

    ui->comboBox_userColor->addItem("#34495e");
    ui->comboBox_userColor->addItem("#16a085");
    ui->comboBox_userColor->addItem("#27ae60");
    ui->comboBox_userColor->addItem("#2980b9");
    ui->comboBox_userColor->addItem("#8e44ad");
    ui->comboBox_userColor->addItem("#f39c12");
    ui->comboBox_userColor->addItem("#d35400");
    ui->comboBox_userColor->addItem("#c0392b");
    ui->comboBox_userColor->addItem("#7f8c8d");
    ui->comboBox_userColor->addItem("#2c3e50");

    for (int i = 0; i < ui->comboBox_userColor->count(); i++) {
        QColor color(ui->comboBox_userColor->itemText(i));
        ui->comboBox_userColor->setItemData(i, color, Qt::TextColorRole);
    }

    connect(ui->pushButton_reject, &QPushButton::clicked,
            this, &AuthDialog::reject);
    connect(ui->pushButton_connect, &QPushButton::clicked,
            this, &AuthDialog::accept);
}

AuthDialog::~AuthDialog()
{
    delete ui;
}

void AuthDialog::setConnectionData(const AuthDialog::ConnectionData &connectionData)
{
    ui->lineEdit_server->setText(connectionData.server);
    ui->lineEdit_port->setText(QString::number(connectionData.port));
    ui->lineEdit_userName->setText(connectionData.userName);
    ui->comboBox_gender->setCurrentIndex(connectionData.gender);
    ui->comboBox_userColor->setCurrentText(connectionData.userColor);
}

AuthDialog::ConnectionData AuthDialog::connectionData() const
{
    ConnectionData connectionData;
    connectionData.server = ui->lineEdit_server->text();
    connectionData.port = ui->lineEdit_port->text().toInt();
    connectionData.userName = ui->lineEdit_userName->text();
    connectionData.gender = ui->comboBox_gender->currentIndex();
    connectionData.userColor = ui->comboBox_userColor->currentText();

    return connectionData;
}
