#include <QCoreApplication>
#include <iostream>
#include "client.h"

using namespace std;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Client client;
    return a.exec();
}
