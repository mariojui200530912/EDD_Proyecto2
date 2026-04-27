#include "mainwindow.h"
#include "ui_mainwindow.h"

// Librerías visuales de Qt
#include <chrono>
#include <QInputDialog>
#include <QListWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <cstdlib>
#include <QPixmap>


// Backend
#include "../utils/CSVReader.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow() {
    delete ui;
}

// Evento: Al hacer clic en "Cargar Archivos"
// --- CARGA DE SUCURSALES ---
void MainWindow::on_btnCargarSucursales_clicked() {
    QString ruta = QFileDialog::getOpenFileName(this, "Seleccionar CSV de Sucursales", "", "Archivos CSV (*.csv)");
    if (!ruta.isEmpty()) {
        CSVReader::cargarSucursales(ruta.toStdString(), redNacional);
        QMessageBox::information(this, "Éxito", "Sucursales cargadas correctamente en el mapa.");
    }
}

// --- CARGA DE CONEXIONES ---
void MainWindow::on_btnCargarConexiones_clicked() {
    // Validación de Integridad
    if (redNacional.estaVacio()) {
        QMessageBox::warning(this, "Acción Denegada", "Debes cargar primero las sucursales antes de crear conexiones.");
        return;
    }

    QString ruta = QFileDialog::getOpenFileName(this, "Seleccionar CSV de Conexiones", "", "Archivos CSV (*.csv)");
    if (!ruta.isEmpty()) {
        CSVReader::cargarConexiones(ruta.toStdString(), redNacional);
        QMessageBox::information(this, "Éxito", "Conexiones de red establecidas.");
    }
}

// --- CARGA DE PRODUCTOS ---
void MainWindow::on_btnCargarProductos_clicked() {
    // Validación de Integridad Estricta
    if (redNacional.estaVacio()) {
        QMessageBox::critical(this, "Error de Integridad Referencial",
                              "No existen sucursales en el sistema.\n\n"
                              "No puedes agregar inventario a la nada. Por favor carga primero el CSV de Sucursales.");
        return;
    }

    QString ruta = QFileDialog::getOpenFileName(this, "Seleccionar CSV de Productos", "", "Archivos CSV (*.csv)");
    if (!ruta.isEmpty()) {
        CSVReader::cargarProductos(ruta.toStdString(), redNacional);
        QMessageBox::information(this, "Éxito", "El inventario ha sido distribuido a las sucursales correspondientes.");
    }
}

void MainWindow::on_btnAgregarManual_clicked() {
    if (redNacional.estaVacio()) {
        QMessageBox::critical(this, "Error", "Debes crear al menos una sucursal primero.");
        return;
    }

    int idSucursalDestino = ui->txtManualID->text().toInt();
    VertexNode* nodo = redNacional.buscarVertice(idSucursalDestino);

    if (nodo == nullptr) {
        QMessageBox::warning(this, "Error", "La Sucursal con ID " + QString::number(idSucursalDestino) + " no existe.");
        return;
    }

    // Armamos el producto con los datos de la interfaz
    Product p;
    p.sucursal_id = idSucursalDestino;
    p.name = ui->txtManualNombre->text().toStdString();
    p.barcode = ui->txtManualCodigo->text().toStdString();
    p.category = ui->txtManualCategoria->text().toStdString();
    p.expiry_date = ui->txtManualFecha->text().toStdString();
    p.brand = ui->txtManualMarca->text().toStdString();
    p.price = ui->txtManualPrecio->text().toDouble();
    p.stock = ui->txtManualStock->text().toInt();

    // Iniciamos el cronómetro
    auto inicioReloj = std::chrono::high_resolution_clock::now();

    // Insertamos atomizadamente en la SUCURSAL ESPECÍFICA
    if (nodo->sucursal.inventarioHash.insertar(p)) {
        nodo->sucursal.inventarioAVL.insertar(p);
        nodo->sucursal.inventarioB.insertar(p);
        nodo->sucursal.inventarioBPlus.insertar(p);

        // Lo guardamos en la pila local por si el usuario quiere hacer Ctrl+Z
        nodo->sucursal.pilaRollback.apilar(p);

        // Detenemos el reloj
        auto finReloj = std::chrono::high_resolution_clock::now();
        auto duracion = std::chrono::duration_cast<std::chrono::microseconds>(finReloj - inicioReloj).count();

        QMessageBox::information(this, "Éxito", "Producto agregado correctamente en " + QString::number(duracion) + " µs");

        // Limpiamos los campos
        ui->txtManualCodigo->clear();
        ui->txtManualNombre->clear();
    } else {
        QMessageBox::warning(this, "Duplicado", "Este código de barras ya existe en el inventario de esta sucursal.");
    }
}

// --- BÚSQUEDA (Con Ventana Emergente y Cronómetro) ---
void MainWindow::on_btnBuscar_clicked() {
    if (redNacional.estaVacio()) return;

    std::string codigo = ui->txtCodigoBarras->text().toStdString();
    if (codigo.empty()) return;

    // Pedimos el ID de la sucursal con un popup elegante de Qt
    bool ok;
    int idSucursal = QInputDialog::getInt(this, "Seleccionar Sucursal",
                                          "¿En qué ID de sucursal deseas buscar?",
                                          1, 1, 9999, 1, &ok);
    if (!ok) return; // Si el usuario cancela, no hacemos nada

    VertexNode* nodo = redNacional.buscarVertice(idSucursal);
    if (nodo == nullptr) {
        QMessageBox::warning(this, "Error", "Sucursal no encontrada.");
        return;
    }

    // Cronometramos la búsqueda O(1) de la tabla Hash local
    auto inicioReloj = std::chrono::high_resolution_clock::now();
    Product* p = nodo->sucursal.inventarioHash.buscar(codigo);
    auto finReloj = std::chrono::high_resolution_clock::now();
    auto duracion = std::chrono::duration_cast<std::chrono::microseconds>(finReloj - inicioReloj).count();

    if (p != nullptr) {
        QString resultado = QString::fromStdString("Encontrado: " + p->name +
                            "\nPrecio: Q" + std::to_string(p->price) +
                            "\nTiempo de Búsqueda: " + std::to_string(duracion) + " µs");
        ui->lblResultado->setText(resultado);
    } else {
        QMessageBox::warning(this, "No Encontrado", "El producto no existe en esta sucursal.");
        ui->lblResultado->setText("Resultado: ");
    }
}

// --- ELIMINAR (Borrado en cascada cronometrado) ---
void MainWindow::on_btnEliminar_clicked() {
    if (redNacional.estaVacio()) return;

    std::string codigo = ui->txtEliminarCodigo->text().toStdString();
    if (codigo.empty()) return;

    bool ok;
    int idSucursal = QInputDialog::getInt(this, "Seleccionar Sucursal",
                                          "¿De qué sucursal deseas eliminar el producto?",
                                          1, 1, 9999, 1, &ok);
    if (!ok) return;

    VertexNode* nodo = redNacional.buscarVertice(idSucursal);
    if (nodo == nullptr) {
        QMessageBox::warning(this, "Error", "Sucursal no encontrada.");
        return;
    }

    // 1. Buscamos primero para hacer backup profundo de los datos
    Product* p = nodo->sucursal.inventarioHash.buscar(codigo);
    if (p == nullptr) {
        QMessageBox::critical(this, "Error", "El producto no existe en esta sucursal.");
        return;
    }
    Product productoBackup = *p;

    // 2. Cronometramos la eliminación en los 4 árboles
    auto inicioReloj = std::chrono::high_resolution_clock::now();

    bool h = nodo->sucursal.inventarioHash.eliminar(productoBackup.barcode);
    bool a = nodo->sucursal.inventarioAVL.eliminar(productoBackup.name);
    bool b = nodo->sucursal.inventarioB.eliminarProducto(productoBackup);
    bool bp = nodo->sucursal.inventarioBPlus.eliminarProducto(productoBackup);

    auto finReloj = std::chrono::high_resolution_clock::now();
    auto duracion = std::chrono::duration_cast<std::chrono::microseconds>(finReloj - inicioReloj).count();

    if (h) {
        QString mensaje = QString::fromStdString(productoBackup.name) + " eliminado.\n" +
                          "- Hash: OK\n" +
                          "- AVL: " + (a ? "OK" : "Falló") + "\n" +
                          "- B-Tree: " + (b ? "OK" : "Falló") + "\n" +
                          "- B+ Tree: " + (bp ? "OK" : "Falló");

        ui->lblResultadoEliminar->setText(QString("Tiempo de borrado: %1 µs").arg(duracion));
        QMessageBox::information(this, "Operación Exitosa", mensaje);
    }
}

// --- VISOR DE INVENTARIO (Pestaña 3) ---
void MainWindow::on_btnVerPorSucursal_clicked() {
    int idBusqueda = ui->txtIdSucursalVer->text().toInt();
    VertexNode* nodo = redNacional.buscarVertice(idBusqueda);

    if (nodo == nullptr) {
        QMessageBox::warning(this, "Error", "La Sucursal indicada no existe.");
        return;
    }

    ui->listaProductos->clear(); // Limpiamos la pantalla

    // Recorremos la tabla Hash de la sucursal para extraer los productos
    int capacidad = nodo->sucursal.inventarioHash.getCapacidad();
    int contador = 0;

    for (int i = 0; i < capacidad; i++) {
        LinkedList* lista = nodo->sucursal.inventarioHash.getListaEnPosicion(i);
        ListNode* actual = lista->getInicio();

        while (actual != nullptr) {
            QString itemTexto = QString::fromStdString(
                "[" + actual->data.barcode + "] " +
                actual->data.name + " - Stock: " +
                std::to_string(actual->data.stock)
            );
            ui->listaProductos->addItem(itemTexto);
            actual = actual->next;
            contador++;
        }
    }

    QMessageBox::information(this, "Carga Completa", "Se cargaron " + QString::number(contador) + " productos.");
}

void MainWindow::on_btnGenerarReporte_clicked() {
    // Obtener los datos del usuario
    std::string estructuraSeleccionada = ui->cbEstructura->currentText().toStdString();
    QString idText = ui->txtReporteSucursalId->text();

    if (idText.isEmpty()) {
        QMessageBox::warning(this, "Aviso", "Por favor ingresa un ID de Sucursal.");
        return;
    }

    int sucursalId = idText.toInt();

    // Buscar la sucursal en el Grafo
    VertexNode* nodoSucursal = redNacional.buscarVertice(sucursalId);
    if (nodoSucursal == nullptr) {
        QMessageBox::critical(this, "Error", "La sucursal indicada no existe.");
        return;
    }

    // Nombres de archivos temporales
    std::string nombreDot = "reporte_temporal.dot";
    std::string nombrePng = "reporte_temporal.png";

    // Generar el archivo .dot según la selección
    if (estructuraSeleccionada == "Tabla Hash") {
        nodoSucursal->sucursal.inventarioHash.generarReporte(nombreDot);
    } else if (estructuraSeleccionada == "Árbol AVL") {
        nodoSucursal->sucursal.inventarioAVL.generarReporte(nombreDot);
    } else if (estructuraSeleccionada == "Árbol B") {
        nodoSucursal->sucursal.inventarioB.generarReporte(nombreDot);
    } else if (estructuraSeleccionada == "Árbol B+") {
        nodoSucursal->sucursal.inventarioBPlus.generarReporte(nombreDot);
    }

    // Ejecutar Graphviz desde la consola
    // El comando es: dot -Tpng reporte_temporal.dot -o reporte_temporal.png
    std::string comando = "dot -Tpng " + nombreDot + " -o " + nombrePng;

    int resultado = system(comando.c_str());

    // Cargar la imagen generada en la interfaz
    if (resultado == 0) {
        QPixmap pixmap(QString::fromStdString(nombrePng));
        if (!pixmap.isNull()) {
            ui->lblImagenReporte->setPixmap(pixmap.scaledToWidth(ui->scrollArea->width() - 20, Qt::SmoothTransformation));
        } else {
            QMessageBox::warning(this, "Error de Renderizado", "Se generó el reporte pero Qt no pudo cargar el archivo PNG.");
        }
    } else {
        QMessageBox::critical(this, "Error de Graphviz",
                              "No se pudo ejecutar Graphviz. \n\n"
                              "Asegúrate de que 'dot' esté en tus variables de entorno PATH.");
    }
}