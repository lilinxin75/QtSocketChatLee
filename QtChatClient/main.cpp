#include "chatclientwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ChatClientWindow w;
    w.show();

    return a.exec();
}
