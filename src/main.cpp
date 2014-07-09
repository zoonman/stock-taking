#include <QtWidgets/QApplication>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QSqlError>
#include <QtWidgets/QTableView>

#include "mainwindow.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
		a.setApplicationVersion(("1.0.4 (2011-05-26)"));

		QCoreApplication::setOrganizationName("ZM Software");
		QCoreApplication::setOrganizationDomain("zoonman.ru");
		QCoreApplication::setApplicationName("Stock-taking");

    MainWindow w;

    w.show();

    return a.exec();
}
