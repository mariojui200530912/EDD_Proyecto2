#include "mainwindow.h"
#include "ui_mainwindow.h"

// Librerías visuales de Qt
#include <chrono>
#include <QThread>
#include <QCoreApplication>
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
    if (redNacional.estaVacio()) {
        QMessageBox::critical(this, "Error de Integridad Referencial",
                              "No existen sucursales en el sistema.\n\n"
                              "No puedes agregar inventario a la nada. Por favor carga primero el CSV de Sucursales.");
        return;
    }

    bool ok;
    int idSucursalDestino = QInputDialog::getInt(this, "Destino de Carga",
                                          "¿A qué ID de sucursal deseas cargar este archivo CSV?",
                                          1, 1, 9999, 1, &ok);
    if (!ok) return;

    // Verificamos si existe
    if (redNacional.buscarVertice(idSucursalDestino) == nullptr) {
        QMessageBox::warning(this, "Error", "La sucursal destino no existe en la red.");
        return;
    }

    QString ruta = QFileDialog::getOpenFileName(this, "Seleccionar CSV de Productos", "", "Archivos CSV (*.csv)");
    if (!ruta.isEmpty()) {
        // Pasamos el idSucursalDestino
        CSVReader::cargarProductos(ruta.toStdString(), redNacional, idSucursalDestino);
        QMessageBox::information(this, "Éxito", "El inventario ha sido cargado en la Sucursal " + QString::number(idSucursalDestino) + ".");
    }
}

// --- CREAR SUCURSAL ---
void MainWindow::on_btnCrearSucursal_clicked() {
    QString idStr = ui->txtSucId->text();
    QString nombre = ui->txtSucNombre->text();

    if (idStr.isEmpty() || nombre.isEmpty()) {
        QMessageBox::warning(this, "Error", "El ID y Nombre son obligatorios.");
        return;
    }

    if (redNacional.buscarVertice(idStr.toInt()) != nullptr) {
        QMessageBox::warning(this, "Error", "Ya existe una sucursal con ese ID.");
        return;
    }

    Sucursal nueva;
    nueva.id = idStr.toInt();
    nueva.nombre = nombre.toStdString();
    nueva.ubicacion = ui->txtSucUbi->text().toStdString();
    nueva.t_ingreso = ui->txtSucTI->text().isEmpty() ? 0 : ui->txtSucTI->text().toInt();
    nueva.t_traspaso = ui->txtSucTT->text().isEmpty() ? 0 : ui->txtSucTT->text().toInt();
    nueva.t_despacho = ui->txtSucTD->text().isEmpty() ? 0 : ui->txtSucTD->text().toInt();

    redNacional.insertarSucursal(nueva);
    QMessageBox::information(this, "Éxito", "Sucursal creada e indexada a la red.");

    // Limpiar campos
    ui->txtSucId->clear(); ui->txtSucNombre->clear(); ui->txtSucUbi->clear();
    ui->txtSucTI->clear(); ui->txtSucTT->clear(); ui->txtSucTD->clear();
}

// --- MODIFICAR SUCURSAL ---
void MainWindow::on_btnModificarSucursal_clicked() {
    int id = ui->txtSucId->text().toInt();

    if (redNacional.buscarVertice(id) == nullptr) {
        QMessageBox::warning(this, "Error", "La sucursal indicada no existe.");
        return;
    }

    bool exito = redNacional.modificarSucursal(
        id,
        ui->txtSucNombre->text().toStdString(),
        ui->txtSucUbi->text().toStdString(),
        ui->txtSucTI->text().toInt(),
        ui->txtSucTT->text().toInt(),
        ui->txtSucTD->text().toInt()
    );

    if (exito) QMessageBox::information(this, "Éxito", "Datos de la sucursal actualizados.");
}

// --- ELIMINAR SUCURSAL ---
void MainWindow::on_btnEliminarSucursal_clicked() {
    int id = ui->txtSucId->text().toInt();

    VertexNode* nodo = redNacional.buscarVertice(id);
    if (nodo == nullptr) {
        QMessageBox::warning(this, "Error", "La sucursal indicada no existe.");
        return;
    }

    QMessageBox::StandardButton resBtn = QMessageBox::question( this, "Confirmar Clausura",
        QString::fromStdString("¿Estás absolutamente seguro de eliminar la sucursal " + nodo->sucursal.nombre + "?\n\nSe destruirán todas las conexiones logísticas hacia ella."),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (resBtn == QMessageBox::Yes) {
        redNacional.eliminarSucursal(id);
        QMessageBox::information(this, "Eliminada", "Sucursal clausurada y conexiones podadas.");
        ui->txtSucId->clear();
    }
}

// --- CREAR CONEXION (ARISTA) MANUAL ---
void MainWindow::on_btnCrearConexion_clicked() {
    QString origenStr = ui->txtConnOrigen->text();
    QString destinoStr = ui->txtConnDestino->text();
    QString tiempoStr = ui->txtConnTiempo->text();
    QString costoStr = ui->txtConnCosto->text();

    // Validar que no haya campos vacios
    if (origenStr.isEmpty() || destinoStr.isEmpty() || tiempoStr.isEmpty() || costoStr.isEmpty()) {
        QMessageBox::warning(this, "Campos Incompletos", "Por favor llena todos los campos de la conexión.");
        return;
    }

    int idOrigen = origenStr.toInt();
    int idDestino = destinoStr.toInt();

    // Verificar que el Origen exista
    if (redNacional.buscarVertice(idOrigen) == nullptr) {
        QMessageBox::critical(this, "Error de Origen",
                              "La sucursal de ORIGEN (ID " + QString::number(idOrigen) + ") no existe en el sistema.");
        return;
    }

    // Verificar que el Destino exista
    if (redNacional.buscarVertice(idDestino) == nullptr) {
        QMessageBox::critical(this, "Error de Destino",
                              "La sucursal de DESTINO (ID " + QString::number(idDestino) + ") no existe en el sistema.");
        return;
    }

    // Armar el objeto Connection e insertarlo
    Connection nuevaConexion;
    nuevaConexion.origen_id = idOrigen;
    nuevaConexion.destino_id = idDestino;
    nuevaConexion.tiempo = tiempoStr.toInt();
    nuevaConexion.costo = costoStr.toDouble();

    redNacional.insertarConexion(nuevaConexion, false);

    QMessageBox::information(this, "Ruta Creada",
                             "Conexión establecida exitosamente de la Sucursal " +
                             QString::number(idOrigen) + " a la " + QString::number(idDestino) + ".");

    // Limpiar campos
    ui->txtConnOrigen->clear();
    ui->txtConnDestino->clear();
    ui->txtConnTiempo->clear();
    ui->txtConnCosto->clear();
}

// --- AGREGAR MANUAL ---
void MainWindow::on_btnAgregarManual_clicked() {
    VertexNode* nodo = obtenerSucursalContexto();
    if (!nodo) return;

    Product p;
    p.name = ui->txtManualNombre->text().toStdString();
    p.barcode = ui->txtManualCodigo->text().toStdString();
    p.category = ui->txtManualCategoria->text().toStdString();
    p.expiry_date = ui->txtManualFecha->text().toStdString();
    p.brand = ui->txtManualMarca->text().toStdString();
    p.price = ui->txtManualPrecio->text().toDouble();
    p.stock = ui->txtManualStock->text().toInt();

    auto inicioReloj = std::chrono::high_resolution_clock::now();

    if (nodo->sucursal.inventarioHash.insertar(p)) {
        nodo->sucursal.inventarioAVL.insertar(p);
        nodo->sucursal.inventarioB.insertar(p);
        nodo->sucursal.inventarioBPlus.insertar(p);
        nodo->sucursal.inventarioLista.insertarFinal(p);

        nodo->sucursal.pilaRollback.apilar(p);

        auto finReloj = std::chrono::high_resolution_clock::now();
        auto duracion = std::chrono::duration_cast<std::chrono::microseconds>(finReloj - inicioReloj).count();

        QMessageBox::information(this, "Éxito", "Producto agregado correctamente en " + QString::number(duracion) + " µs");

        ui->txtManualCodigo->clear();
        ui->txtManualNombre->clear();
    } else {
        QMessageBox::warning(this, "Duplicado", "Este código de barras ya existe en el inventario de esta sucursal.");
    }
}

// --- ELIMINAR  ---
void MainWindow::on_btnEliminar_clicked() {
    // Obtenemos la sucursal del Contexto
    VertexNode* nodo = obtenerSucursalContexto();
    if (!nodo) return;

    std::string codigo = ui->txtEliminarCodigo->text().toStdString();
    if (codigo.empty()) return;

    // Buscamos primero para hacer backup profundo de los datos
    Product* p = nodo->sucursal.inventarioHash.buscar(codigo);
    if (p == nullptr) {
        QMessageBox::critical(this, "Error", "El producto no existe en esta sucursal.");
        return;
    }
    Product productoBackup = *p;

    // Cronometramos la eliminación en los 4 árboles
    auto inicioReloj = std::chrono::high_resolution_clock::now();

    bool h = nodo->sucursal.inventarioHash.eliminar(productoBackup.barcode);
    bool a = nodo->sucursal.inventarioAVL.eliminar(productoBackup.name);
    bool b = nodo->sucursal.inventarioB.eliminarProducto(productoBackup);
    bool bp = nodo->sucursal.inventarioBPlus.eliminarProducto(productoBackup);
    bool l = nodo->sucursal.inventarioLista.eliminarPorCodigo(productoBackup.barcode);

    auto finReloj = std::chrono::high_resolution_clock::now();
    auto duracion = std::chrono::duration_cast<std::chrono::microseconds>(finReloj - inicioReloj).count();

    if (h) {
        QString mensaje = QString::fromStdString(productoBackup.name) +
                          " eliminado exitosamente en " + QString::number(duracion) + " µs.\n\n" +
                          "Desglose de la cascada:\n" +
                          "- Hash: OK\n" +
                          "- AVL: " + (a ? "OK" : "Falló") + "\n" +
                          "- B-Tree: " + (b ? "OK" : "Falló") + "\n" +
                          "- B+ Tree: " + (bp ? "OK" : "Falló") + "\n" +
                          "- Lista General: " + (l ? "OK" : "Falló");

        QMessageBox::information(this, "Operación Exitosa", mensaje);

        // Limpiamos el campo de texto
        ui->txtEliminarCodigo->clear();
    }
}

// --- BUSCAR POR NOMBRE (AVL) ---
void MainWindow::on_btnBuscarAVL_clicked() {
    VertexNode* nodo = obtenerSucursalContexto();
    if (!nodo) return;

    std::string nombre = ui->txtBuscarAVL->text().toStdString();
    if (nombre.empty()) return;

    ui->listaResultadosBusqueda->clear();

    auto inicio = std::chrono::high_resolution_clock::now();
    Product* p = nodo->sucursal.inventarioAVL.buscar(nombre);
    auto fin = std::chrono::high_resolution_clock::now();
    auto duracion = std::chrono::duration_cast<std::chrono::microseconds>(fin - inicio).count();

    if (p != nullptr) {
        ui->listaResultadosBusqueda->addItem(QString::fromStdString("✅ Encontrado (" + std::to_string(duracion) + " µs)"));
        ui->listaResultadosBusqueda->addItem(QString::fromStdString("Nombre: " + p->name));
        ui->listaResultadosBusqueda->addItem(QString::fromStdString("Código: " + p->barcode));
        ui->listaResultadosBusqueda->addItem(QString::fromStdString("Precio: Q" + std::to_string(p->price)));
        ui->listaResultadosBusqueda->addItem(QString::fromStdString("Stock: " + std::to_string(p->stock)));
        ui->listaResultadosBusqueda->addItem(QString::fromStdString("Estado: " + p->estado));
    } else {
        ui->listaResultadosBusqueda->addItem("❌ Producto no encontrado.");
    }
}

// --- BUSCAR POR CÓDIGO (HASH) ---
void MainWindow::on_btnBuscarHash_clicked() {
    VertexNode* nodo = obtenerSucursalContexto();
    if (!nodo) return;

    std::string codigo = ui->txtBuscarHash->text().toStdString();
    if (codigo.empty()) return;

    ui->listaResultadosBusqueda->clear();

    auto inicio = std::chrono::high_resolution_clock::now();
    Product* p = nodo->sucursal.inventarioHash.buscar(codigo);
    auto fin = std::chrono::high_resolution_clock::now();
    auto duracion = std::chrono::duration_cast<std::chrono::microseconds>(fin - inicio).count();

    if (p != nullptr) {
        ui->listaResultadosBusqueda->addItem(QString::fromStdString("✅ Encontrado (" + std::to_string(duracion) + " µs)"));
        ui->listaResultadosBusqueda->addItem(QString::fromStdString("Nombre: " + p->name));
        ui->listaResultadosBusqueda->addItem(QString::fromStdString("Código: " + p->barcode));
        ui->listaResultadosBusqueda->addItem(QString::fromStdString("Precio: Q" + std::to_string(p->price)));
        ui->listaResultadosBusqueda->addItem(QString::fromStdString("Stock: " + std::to_string(p->stock)));
        ui->listaResultadosBusqueda->addItem(QString::fromStdString("Estado: " + p->estado));
    } else {
        ui->listaResultadosBusqueda->addItem("❌ Producto no encontrado.");
    }
}

// --- BUSCAR POR CATEGORÍA (B+) ---
void MainWindow::on_btnBuscarBPlus_clicked() {
    VertexNode* nodo = obtenerSucursalContexto();
    if (!nodo) return;

    std::string categoria = ui->txtBuscarBPlus->text().toStdString();
    if (categoria.empty()) return;

    ui->listaResultadosBusqueda->clear();

    auto inicio = std::chrono::high_resolution_clock::now();
    LinkedList* listaCat = nodo->sucursal.inventarioBPlus.buscarCategoria(categoria);
    auto fin = std::chrono::high_resolution_clock::now();
    auto duracion = std::chrono::duration_cast<std::chrono::microseconds>(fin - inicio).count();

    if (listaCat != nullptr && !listaCat->estaVacia()) {
        ui->listaResultadosBusqueda->addItem(QString::fromStdString("✅ Categoría: " + categoria + " (" + std::to_string(duracion) + " µs)"));

        ListNode* actual = listaCat->getInicio();
        while (actual != nullptr) {
            ui->listaResultadosBusqueda->addItem(QString::fromStdString("- " + actual->data.name + " (" + actual->data.barcode + ")"));
            actual = actual->next;
        }
    } else {
        ui->listaResultadosBusqueda->addItem("❌ No hay productos en esta categoría.");
    }
}

// --- BUSCAR POR RANGO DE FECHAS (B-TREE) ---
void MainWindow::on_btnBuscarB_clicked() {
    VertexNode* nodo = obtenerSucursalContexto();
    if (!nodo) return;

    std::string fechaIn = ui->txtBuscarBInicio->text().toStdString();
    std::string fechaFin = ui->txtBuscarBFin->text().toStdString();

    if (fechaIn.empty() || fechaFin.empty()) return;

    ui->listaResultadosBusqueda->clear();

    // Usamos tu LinkedList personalizada
    LinkedList resultados;

    auto inicio = std::chrono::high_resolution_clock::now();
    nodo->sucursal.inventarioB.buscarPorRangoFechas(fechaIn, fechaFin, resultados);
    auto fin = std::chrono::high_resolution_clock::now();
    auto duracion = std::chrono::duration_cast<std::chrono::microseconds>(fin - inicio).count();

    if (!resultados.estaVacia()) {
        ui->listaResultadosBusqueda->addItem(QString::fromStdString("✅ Rango encontrado (" + std::to_string(duracion) + " µs)"));

        ListNode* actual = resultados.getInicio();
        while (actual != nullptr) {
            ui->listaResultadosBusqueda->addItem(QString::fromStdString("- [" + actual->data.expiry_date + "] " + actual->data.name));
            actual = actual->next;
        }
    } else {
        ui->listaResultadosBusqueda->addItem("❌ Sin productos por vencer en ese rango.");
    }
}

void MainWindow::on_btnCompararTiempos_clicked() {
    VertexNode* nodo = obtenerSucursalContexto();
    if (!nodo) return;

    std::string codigoTest = ui->txtBuscarHash->text().toStdString();
    if (codigoTest.empty()) {
        QMessageBox::information(this, "Prueba de Rendimiento", "Ingresa un Código de Barras válido en la casilla de 'Buscar en Hash' para iniciar la prueba.");
        return;
    }

    // Tiempo Hash O(1)
    auto t1 = std::chrono::high_resolution_clock::now();
    nodo->sucursal.inventarioHash.buscar(codigoTest);
    auto t2 = std::chrono::high_resolution_clock::now();
    auto tiempoHash = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

    // Tiempo AVL O(log n) - Forzamos el peor caso buscando un nombre que no existe
    auto t3 = std::chrono::high_resolution_clock::now();
    nodo->sucursal.inventarioAVL.buscar("ZZZ_WORST_CASE");
    auto t4 = std::chrono::high_resolution_clock::now();
    auto tiempoAVL = std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count();

    // Tiempo Lista O(n) - Búsqueda real en la LinkedList principal
    auto t5 = std::chrono::high_resolution_clock::now();
    nodo->sucursal.inventarioLista.buscarPorCodigo(codigoTest);
    auto t6 = std::chrono::high_resolution_clock::now();
    auto tiempoLista = std::chrono::duration_cast<std::chrono::microseconds>(t6 - t5).count();

    // Mostrar Resultados
    QString resultado = QString("Hash O(1): %1 µs  |  AVL O(log n): %2 µs  |  Lista O(n): %3 µs")
                            .arg(tiempoHash).arg(tiempoAVL).arg(tiempoLista);
    ui->lblTiemposComparativa->setText(resultado);
}

void MainWindow::on_btnCompararOrdenamiento_clicked() {
    VertexNode* nodo = obtenerSucursalContexto();
    if (!nodo) return;

    // AVL O(n) para listar
    LinkedList resAVL;
    auto t1 = std::chrono::high_resolution_clock::now();
    nodo->sucursal.inventarioAVL.listarAlfabeticamente(resAVL);
    auto t2 = std::chrono::high_resolution_clock::now();
    auto tiempoAVL = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

    // Lista O(n^2) -> Clono la lista y aplico burbuja
    LinkedList listaClon;
    ListNode* it = nodo->sucursal.inventarioLista.getInicio();
    while(it != nullptr) { listaClon.insertarFinal(it->data); it = it->next; }

    auto t3 = std::chrono::high_resolution_clock::now();
    listaClon.ordenarPorNombreBurbuja();
    auto t4 = std::chrono::high_resolution_clock::now();
    auto tiempoBurbuja = std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count();

    QString resultado = QString("📊 ORDENAMIENTO (Alfabetico):\nAVL (In-Order) O(n): %1 µs\nLista (Burbuja) O(n²): %2 µs")
                            .arg(tiempoAVL).arg(tiempoBurbuja);
    ui->lblTiemposComparativa->setText(resultado);

    ui->listaResultadosBusqueda->clear();
    ui->listaResultadosBusqueda->addItem(QString("✅ Ordenamiento Burbuja (%1 µs):").arg(tiempoBurbuja));
    ListNode* actual = listaClon.getInicio();
    while (actual != nullptr) {
        ui->listaResultadosBusqueda->addItem(QString::fromStdString("- " + actual->data.name));
        actual = actual->next;
    }
}

void MainWindow::on_btnVerHistorial_clicked() {
    VertexNode* nodo = obtenerSucursalContexto();
    if (!nodo) return;

    ui->listaResultadosBusqueda->clear();
    ui->listaResultadosBusqueda->addItem("🕒 HISTORIAL DE CAMBIOS (Pila):");

    LinkedList temporal;
    nodo->sucursal.pilaRollback.obtenerContenido(temporal);

    if (temporal.estaVacia()) {
        ui->listaResultadosBusqueda->addItem("No hay movimientos recientes.");
        return;
    }

    ListNode* actual = temporal.getInicio();
    while (actual != nullptr) {
        ui->listaResultadosBusqueda->addItem(QString::fromStdString("-> Ingreso: " + actual->data.name + " (" + actual->data.barcode + ")"));
        actual = actual->next;
    }
}

// --- LISTAR IN-ORDER (AVL) ---
void MainWindow::on_btnListarAVL_clicked() {
    VertexNode* nodo = obtenerSucursalContexto();
    if (!nodo) return;

    ui->listaResultadosBusqueda->clear();

    LinkedList resultados;

    auto inicio = std::chrono::high_resolution_clock::now();
    nodo->sucursal.inventarioAVL.listarAlfabeticamente(resultados);
    auto fin = std::chrono::high_resolution_clock::now();
    auto duracion = std::chrono::duration_cast<std::chrono::microseconds>(fin - inicio).count();

    if (!resultados.estaVacia()) {
        ui->listaResultadosBusqueda->addItem(QString::fromStdString("✅ Catálogo Alfabético (" + std::to_string(duracion) + " µs)"));

        ListNode* actual = resultados.getInicio();
        while (actual != nullptr) {
            ui->listaResultadosBusqueda->addItem(QString::fromStdString("- " + actual->data.name + " (Q" + std::to_string(actual->data.price) + ")"));
            actual = actual->next;
        }
    } else {
        ui->listaResultadosBusqueda->addItem("El catálogo está vacío.");
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
                actual->data.name + " - Stock: " + std::to_string(actual->data.stock) +
                " | Estado: " + actual->data.estado // <--- AÑADE ESTO
            );
            ui->listaProductos->addItem(itemTexto);
            actual = actual->next;
            contador++;
        }
    }

    QMessageBox::information(this, "Carga Completa", "Se cargaron " + QString::number(contador) + " productos.");
}

// --- Simulacion (Pestaña 4) ---
void MainWindow::on_btnIniciarDespacho_clicked() {
    ui->listaLogDespacho->clear();

    if (redNacional.estaVacio()) {
        QMessageBox::critical(this, "Error", "La red nacional está vacía.");
        return;
    }

    int idOrigen = ui->txtDespachoOrigen->text().toInt();
    int idDestino = ui->txtDespachoDestino->text().toInt();
    std::string codigoStr = ui->txtDespachoCodigo->text().toStdString();
    bool optTiempo = ui->rbOptTiempo->isChecked();

    if (codigoStr.empty() || ui->txtDespachoOrigen->text().isEmpty() || ui->txtDespachoDestino->text().isEmpty()) {
        QMessageBox::warning(this, "Aviso", "Todos los campos son obligatorios.");
        return;
    }

    // Calcular Ruta (Dijkstra)
    int ruta[100];
    int tamanoRuta = 0;

    if (!redNacional.obtenerRutaDijkstra(idOrigen, idDestino, optTiempo, ruta, tamanoRuta)) {
        ui->listaLogDespacho->addItem("❌ ERROR: No existe un camino viable entre estas sucursales.");
        return;
    }

    // Buscar el producto en la sucursal origen
    VertexNode* nodoOrigen = redNacional.buscarVertice(idOrigen);
    Product* p = nodoOrigen->sucursal.inventarioHash.buscar(codigoStr);

    if (p == nullptr) {
        ui->listaLogDespacho->addItem("❌ ERROR: El producto no existe en el origen.");
        return;
    }

    Product productoTransferencia = *p;
    productoTransferencia.estado = "En tránsito";

    // Extraccion de Origen
    nodoOrigen->sucursal.inventarioHash.eliminar(productoTransferencia.barcode);
    nodoOrigen->sucursal.inventarioAVL.eliminar(productoTransferencia.name);
    nodoOrigen->sucursal.inventarioB.eliminarProducto(productoTransferencia);
    nodoOrigen->sucursal.inventarioBPlus.eliminarProducto(productoTransferencia);
    nodoOrigen->sucursal.inventarioLista.eliminarPorCodigo(productoTransferencia.barcode);

    // --- INICIO DE LA ANIMACION VISUAL DE COLAS  ---

    auto log = [&](const QString& mensaje, int delayMs = 600) {
        ui->listaLogDespacho->addItem(mensaje);
        ui->listaLogDespacho->scrollToBottom();
        QCoreApplication::processEvents();
        QThread::msleep(delayMs);
    };

    // --- NUEVA FUNCIÓN PARA RENDERIZAR EL MAPA ---
    auto actualizarMapa = [&](int idNodoActivo) {
        redNacional.generarReporteSimulacion("animacion_temp.dot", idNodoActivo, ruta, tamanoRuta);
        system("dot -Tpng animacion_temp.dot -o animacion_temp.png");
        QPixmap pixmap("animacion_temp.png");
        ui->lblMapaAnimado->setPixmap(pixmap.scaled(ui->lblMapaAnimado->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        QCoreApplication::processEvents(); // Refresca la UI
    };

    log(QString("▶ INGRESO A RED LOGÍSTICA: [ %1 ]").arg(QString::fromStdString(productoTransferencia.name)));
    log(QString("📍 Estado actual: %1").arg(QString::fromStdString(productoTransferencia.estado)));

    // ... (Tu código para mostrar la ruta en texto se mantiene igual)

    // 4. Recorrido por el Grafo usando las Colas y Animación Gráfica
    for (int i = 0; i < tamanoRuta; i++) {
        VertexNode* nodoActual = redNacional.buscarVertice(ruta[i]);
        Product tmp;

        // ¡Magia! Renderizamos el mapa con este nodo en Amarillo Brillante
        actualizarMapa(nodoActual->sucursal.id);

        if (i == 0) {
            // --- CASO A: SUCURSAL ORIGEN ---
            log(QString("🏢 [ORIGEN] Sucursal %1:").arg(nodoActual->sucursal.id), 200);

            log("   ↳ Entrando a Cola de Ingreso...");
            nodoActual->colaIngreso.encolar(productoTransferencia);
            QThread::msleep(nodoActual->sucursal.t_ingreso * 50);
            nodoActual->colaIngreso.desencolar(tmp);

            log("   ↳ Pasando a Cola de Salida (Despacho)...");
            nodoActual->colaSalida.encolar(productoTransferencia);
            QThread::msleep(nodoActual->sucursal.t_despacho * 50);
            nodoActual->colaSalida.desencolar(tmp);

        } else if (i == tamanoRuta - 1) {
            // --- CASO B: SUCURSAL DESTINO ---
            log(QString("🏢 [DESTINO] Sucursal %1:").arg(nodoActual->sucursal.id), 200);

            log("   ↳ Entrando a Cola de Ingreso (Recepción)...");
            nodoActual->colaIngreso.encolar(productoTransferencia);
            QThread::msleep(nodoActual->sucursal.t_ingreso * 50);
            nodoActual->colaIngreso.desencolar(tmp);

        } else {
            // --- CASO C: SUCURSALES INTERMEDIAS ---
            log(QString("🔄 [INTERMEDIO] Sucursal %1 (En tránsito):").arg(nodoActual->sucursal.id), 200);

            log("   ↳ Descargando en Cola de Ingreso...");
            nodoActual->colaIngreso.encolar(productoTransferencia);
            QThread::msleep(nodoActual->sucursal.t_ingreso * 50);
            nodoActual->colaIngreso.desencolar(tmp);

            // GESTION DE COLA DE PREPARACION
            log("   ↳ Procesando en Cola de Preparación (Traspaso)...");
            nodoActual->colaPreparacion.encolar(productoTransferencia);
            QThread::msleep(nodoActual->sucursal.t_traspaso * 50);
            nodoActual->colaPreparacion.desencolar(tmp);

            log("   ↳ Cargando en Cola de Salida...");
            nodoActual->colaSalida.encolar(productoTransferencia);
            QThread::msleep(nodoActual->sucursal.t_despacho * 50);
            nodoActual->colaSalida.desencolar(tmp);
        }

        if (i < tamanoRuta - 1) {
            log(QString("🚚 Viajando hacia la Sucursal %1...").arg(ruta[i+1]), 800);
            log("--------------------------------------------------", 200);
        }
    }

    // Insercion en Destino (Atomizada)
    log("\n[FASE FINAL: ALMACENAJE]");
    VertexNode* nodoDestino = redNacional.buscarVertice(idDestino);
    Product* productoExistente = nodoDestino->sucursal.inventarioHash.buscar(productoTransferencia.barcode);
    if (productoExistente != nullptr) {
        log("📍 El producto ya existe en destino. Actualizando inventario local...");

        // Sumamos el stock que acaba de llegar al que ya tenia
        int nuevoStock = productoExistente->stock + productoTransferencia.stock;

        productoExistente->stock = nuevoStock;
        productoExistente->estado = "Disponible";

        nodoDestino->sucursal.inventarioB.eliminarProducto(*productoExistente);
        nodoDestino->sucursal.inventarioBPlus.eliminarProducto(*productoExistente);

        nodoDestino->sucursal.inventarioB.insertar(*productoExistente);
        nodoDestino->sucursal.inventarioBPlus.insertar(*productoExistente);

        log(QString("✔ Stock consolidado. Nuevo stock: %1 unidades.").arg(nuevoStock));

    } else {
        log("📍 Producto nuevo en destino. Indexando en Árboles...");

        productoTransferencia.estado = (productoTransferencia.stock > 0) ? "Disponible" : "Agotado";

        nodoDestino->sucursal.inventarioHash.insertar(productoTransferencia);
        nodoDestino->sucursal.inventarioAVL.insertar(productoTransferencia);
        nodoDestino->sucursal.inventarioB.insertar(productoTransferencia);
        nodoDestino->sucursal.inventarioBPlus.insertar(productoTransferencia);
        nodoDestino->sucursal.inventarioLista.insertarFinal(productoTransferencia);

        log("✔ Indexación completada exitosamente.");
    }

    log("\n🎉 PRODUCTO INGRESADO Y ENRUTADO CON ÉXITO 🎉", 0);
    QMessageBox::information(this, "Operación Exitosa", "El producto completó su ruta y ya está disponible en su destino.");

    ui->txtDespachoCodigo->clear();
}

void MainWindow::on_btnGenerarReporte_clicked()
{
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

    // Generar el archivo .dot según la seleccion
    if (estructuraSeleccionada == "Mapa de Red"){
        redNacional.generarReporteGrafo(nombreDot);
    }else if (estructuraSeleccionada == "Tabla Hash") {
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

VertexNode* MainWindow::obtenerSucursalContexto() {
    QString idText = ui->txtContextoSucursal->text();
    if (idText.isEmpty()) {
        QMessageBox::warning(this, "Aviso", "Primero ingresa el ID de la Sucursal en el panel de Contexto.");
        return nullptr;
    }

    VertexNode* nodo = redNacional.buscarVertice(idText.toInt());
    if (nodo == nullptr) {
        QMessageBox::warning(this, "Error", "La sucursal indicada no existe.");
    }
    return nodo;
}

void MainWindow::on_btnDeshacer_clicked() {
    VertexNode* nodo = obtenerSucursalContexto();
    if (!nodo) return;

    Product productoDeshacer;
    if (nodo->sucursal.pilaRollback.desapilar(productoDeshacer)) {

        nodo->sucursal.inventarioHash.eliminar(productoDeshacer.barcode);
        nodo->sucursal.inventarioAVL.eliminar(productoDeshacer.name);
        nodo->sucursal.inventarioB.eliminarProducto(productoDeshacer);
        nodo->sucursal.inventarioBPlus.eliminarProducto(productoDeshacer);
        nodo->sucursal.inventarioLista.eliminarPorCodigo(productoDeshacer.barcode);

        QMessageBox::information(this, "Rollback Exitoso",
            QString::fromStdString("Se deshizo el ingreso del producto: " + productoDeshacer.name));
    } else {
        QMessageBox::warning(this, "Pila Vacía", "No hay operaciones recientes para deshacer en esta sucursal.");
    }
}