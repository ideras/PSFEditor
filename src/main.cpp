#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString filePath;
    QStringList arguments = QCoreApplication::arguments();
    if (arguments.size() > 1) {
        filePath = arguments.at(1);
    }
    MainWindow w(nullptr, filePath);

    a.setStyle("fusion");
    w.show();

    return a.exec();
}
