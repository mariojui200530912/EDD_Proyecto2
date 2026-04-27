#ifndef PROYECTO2_MAINWINDOW_H
#define PROYECTO2_MAINWINDOW_H

#include <QMainWindow>
#include "../structures/HashTable.h"
#include "../structures/AVLTree.h"
#include "../structures/BTree.h"
#include "../structures/BTreePlus.h"
#include "../structures/Graph.h"
#include "../structures/Stack.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnCargarSucursales_clicked();
    void on_btnCargarConexiones_clicked();
    void on_btnCargarProductos_clicked();
    void on_btnBuscar_clicked();
    void on_btnAgregarManual_clicked();
    void on_btnVerPorSucursal_clicked();
    void on_btnEliminar_clicked();
    void on_btnGenerarReporte_clicked();

private:
    Ui::MainWindow *ui;
    Graph redNacional;
};

#endif //PROYECTO2_MAINWINDOW_H