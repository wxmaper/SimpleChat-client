#include "widget.h"
#include <QApplication>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("SimpleChat");
    a.setApplicationDisplayName("SimpleChat");

    Widget w;
    w.setWindowTitle("Простой чат");
    w.show();

    QTimer::singleShot(1000, &w, &Widget::connectToServer);

    return a.exec();
}
