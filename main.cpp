#include <iostream>
#include "src/ui/mainwindow.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Crea la ventana principal
    MainWindow w;
    w.setWindowTitle("Sistema de Gestión Logística - Estructuras de Datos");
    w.resize(800, 600);

    // La muestra en pantalla
    w.show();

    // Mantiene la aplicación corriendo hasta que presiones la 'X'
    return a.exec();
}