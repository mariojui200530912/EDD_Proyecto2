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
    #include <QWheelEvent>
    #include <QTableWidget>
    #include <QHeaderView>
    #include <QPixmap>
    #include <QTime>


    // Backend
    #include "../utils/CSVReader.h"

    MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent)
        , ui(new Ui::MainWindow)
    {
        ui->setupUi(this);
        ui->scrollArea->installEventFilter(this);
        ui->scrollArea->setWidgetResizable(false);
        ui->lblImagenReporte->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        ui->lblImagenReporte->setScaledContents(false);
    }

    MainWindow::~MainWindow() {
        delete ui;
    }

    // Evento: Al hacer clic en "Cargar Archivos"
    // --- CARGA DE SUCURSALES MASIVA ---
    void MainWindow::on_btnCargarSucursales_clicked() {
        QString ruta = QFileDialog::getOpenFileName(this, "Seleccionar CSV de Sucursales", "", "Archivos CSV (*.csv)");
        if (!ruta.isEmpty()) {

            ReporteCarga reporte = CSVReader::cargarSucursales(ruta.toStdString(), redNacional);

            if (reporte.errores > 0) {
                QString mensaje = QString(
                    "El proceso finalizó con advertencias.\n\n"
                    "📊 Resumen de Operación:\n"
                    "- Líneas procesadas: %1\n"
                    "- Sucursales creadas: %2\n"
                    "- Rechazadas (Formato/Duplicados): %3\n\n"
                    "Haz clic en 'Show Details...' para ver qué filas fallaron."
                ).arg(reporte.totalLeidos).arg(reporte.ingresados).arg(reporte.errores);

                QMessageBox msgBox(this);
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setWindowTitle("Reporte de Sucursales");
                msgBox.setText(mensaje);
                msgBox.setDetailedText(QString::fromStdString(reporte.detalleErrores));
                msgBox.setStyleSheet("QMessageBox { width: 500px; }");
                msgBox.exec();
            } else {
                QMessageBox::information(this, "Carga Perfecta",
                    QString("Se indexaron %1 sucursales correctamente al mapa.").arg(reporte.ingresados));
            }
        }
    }

    // --- CARGA DE CONEXIONES MASIVA ---
    void MainWindow::on_btnCargarConexiones_clicked() {
        // Validación de Integridad
        if (redNacional.estaVacio()) {
            QMessageBox::warning(this, "Acción Denegada", "Debes cargar primero las sucursales antes de crear conexiones.");
            return;
        }

        QString ruta = QFileDialog::getOpenFileName(this, "Seleccionar CSV de Conexiones", "", "Archivos CSV (*.csv)");
        if (!ruta.isEmpty()) {

            ReporteCarga reporte = CSVReader::cargarConexiones(ruta.toStdString(), redNacional);

            if (reporte.errores > 0) {
                QString mensaje = QString(
                    "El trazado de rutas finalizó con advertencias.\n\n"
                    "📊 Resumen de Operación:\n"
                    "- Líneas procesadas: %1\n"
                    "- Aristas conectadas: %2\n"
                    "- Fallidas (Nodos fantasma/Formato): %3\n\n"
                    "Haz clic en 'Show Details...' para revisar el log."
                ).arg(reporte.totalLeidos).arg(reporte.ingresados).arg(reporte.errores);

                QMessageBox msgBox(this);
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setWindowTitle("Reporte de Red Logística");
                msgBox.setText(mensaje);
                msgBox.setDetailedText(QString::fromStdString(reporte.detalleErrores));
                msgBox.setStyleSheet("QMessageBox { width: 500px; }");
                msgBox.exec();
            } else {
                QMessageBox::information(this, "Carga Perfecta",
                    QString("Se tendieron %1 conexiones logísticas exitosamente.").arg(reporte.ingresados));
            }
        }
    }

    // --- CARGA DE PRODUCTOS MASIVA ---
    void MainWindow::on_btnCargarProductos_clicked() {
        if (redNacional.estaVacio()) {
            QMessageBox::critical(this, "Error de Integridad",
                                  "No existen sucursales en el sistema.\n\n"
                                  "No puedes agregar inventario a la nada. Por favor carga primero el CSV de Sucursales.");
            return;
        }

        bool ok;
        int idSucursalDestino = QInputDialog::getInt(this, "Destino de Carga",
                                              "¿A qué ID de sucursal deseas cargar este archivo CSV?",
                                              1, 1, 9999, 1, &ok);
        if (!ok) return;

        if (redNacional.buscarVertice(idSucursalDestino) == nullptr) {
            QMessageBox::warning(this, "Error", "La sucursal destino no existe en la red.");
            return;
        }

        QString ruta = QFileDialog::getOpenFileName(this, "Seleccionar CSV de Productos", "", "Archivos CSV (*.csv)");
        if (!ruta.isEmpty()) {

            ReporteCarga reporte = CSVReader::cargarProductos(ruta.toStdString(), redNacional, idSucursalDestino);

            if (reporte.errores > 0) {
                QString mensaje = QString(
                    "La sincronización de inventario finalizó con advertencias.\n\n"
                    "📊 Reporte del Backend:\n"
                    "- Filas procesadas: %1\n"
                    "- Productos indexados: %2\n"
                    "- Rechazados (Formato/Duplicados): %3\n\n"
                    "Haz clic en 'Show Details...' para ver qué códigos fallaron."
                ).arg(reporte.totalLeidos).arg(reporte.ingresados).arg(reporte.errores);

                QMessageBox msgBox(this);
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setWindowTitle("Alerta de Inventario");
                msgBox.setText(mensaje);
                msgBox.setDetailedText(QString::fromStdString(reporte.detalleErrores));
                msgBox.setStyleSheet("QMessageBox { width: 500px; }");
                msgBox.exec();
            } else {
                QString mensajeExito = QString(
                    "Integración exitosa.\n\n"
                    "📊 Reporte del Backend:\n"
                    "- Productos indexados: %1\n"
                    "- Errores: 0\n\n"
                    "El inventario está listo en la Sucursal %2."
                ).arg(reporte.ingresados).arg(idSucursalDestino);

                QMessageBox::information(this, "Carga Perfecta", mensajeExito);
            }
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
        bool esBidi =  ui->chkBidireccional->isChecked();

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

        redNacional.insertarConexion(nuevaConexion, esBidi);

        QMessageBox::information(this, "Ruta Creada",
                                 "Conexión establecida exitosamente de la Sucursal " +
                                 QString::number(idOrigen) + " a la " + QString::number(idDestino) + ".");

        // Limpiar campos
        ui->txtConnOrigen->clear();
        ui->txtConnDestino->clear();
        ui->txtConnTiempo->clear();
        ui->txtConnCosto->clear();
    }

    void MainWindow::on_btnModificarConexion_clicked() {
        if (ui->txtConnOrigen->text().isEmpty() || ui->txtConnDestino->text().isEmpty() ||
            ui->txtConnTiempo->text().isEmpty() || ui->txtConnCosto->text().isEmpty()) {
            QMessageBox::warning(this, "Aviso", "Llena todos los campos para modificar una ruta.");
            return;
            }

        int origen = ui->txtConnOrigen->text().toInt();
        int destino = ui->txtConnDestino->text().toInt();
        int nuevoTiempo = ui->txtConnTiempo->text().toInt();
        double nuevoCosto = ui->txtConnCosto->text().toDouble();
        bool esBidi = ui->chkBidireccional->isChecked();

        if (redNacional.modificarConexion(origen, destino, nuevoTiempo, nuevoCosto, esBidi)) {
            QMessageBox::information(this, "Éxito", "Tiempos y costos de la ruta actualizados.");
            ui->txtConnOrigen->clear(); ui->txtConnDestino->clear();
            ui->txtConnTiempo->clear(); ui->txtConnCosto->clear();
        } else {
            QMessageBox::critical(this, "Error", "No se encontró una conexión directa entre estos nodos.");
        }
    }

    // --- ELIMINAR CONEXION ---
    void MainWindow::on_btnEliminarConexion_clicked() {
        if (ui->txtConnOrigen->text().isEmpty() || ui->txtConnDestino->text().isEmpty()) {
            QMessageBox::warning(this, "Aviso", "Solo necesitas el Origen y Destino para eliminar.");
            return;
        }

        int origen = ui->txtConnOrigen->text().toInt();
        int destino = ui->txtConnDestino->text().toInt();
        bool esBidi = ui->chkBidireccional->isChecked();

        if (redNacional.eliminarConexion(origen, destino, esBidi)) {
            QMessageBox::information(this, "Éxito", "La carretera fue clausurada y eliminada del mapa.");
            ui->txtConnOrigen->clear(); ui->txtConnDestino->clear();
        } else {
            QMessageBox::critical(this, "Error", "No existe conexión entre estas sucursales.");
        }
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

        // --- CONFIGURACIÓN DE LA TABLA ---
        ui->tablaProductos->clear();
        ui->tablaProductos->setColumnCount(8); // 8 atributos
        ui->tablaProductos->setHorizontalHeaderLabels({
            "Código", "Nombre", "Marca", "Categoría", "Precio (Q)", "Stock", "Vencimiento", "Estado"
        });

        // Ajustar el ancho de las columnas automáticamente
        ui->tablaProductos->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        ui->tablaProductos->horizontalHeader()->setStretchLastSection(true); // Que el estado llene el resto del espacio
        ui->tablaProductos->setRowCount(0); // Reiniciamos el contador de filas

        // --- RECOLECCIÓN DE DATOS SEGÚN EL ORDEN ---
        QString criterio = ui->cbOrdenVisor->currentText();
        LinkedList resultados;

        if (criterio == "Orden Alfabético (AVL)") {
            nodo->sucursal.inventarioAVL.listarAlfabeticamente(resultados);
        }
        else if (criterio == "Por Categoría (B+)") {
            nodo->sucursal.inventarioBPlus.obtenerTodaLaLista(resultados);
        }
        else if (criterio == "Por Fecha de Expiración (B)") {
            nodo->sucursal.inventarioB.listarPorFecha(resultados);
        }

        // --- LLENADO VISUAL DE LA TABLA ---
        ListNode* actual = resultados.getInicio();
        int fila = 0;

        while (actual != nullptr) {
            ui->tablaProductos->insertRow(fila);

            // Insertamos los datos básicos
            ui->tablaProductos->setItem(fila, 0, new QTableWidgetItem(QString::fromStdString(actual->data.barcode)));
            ui->tablaProductos->setItem(fila, 1, new QTableWidgetItem(QString::fromStdString(actual->data.name)));
            ui->tablaProductos->setItem(fila, 2, new QTableWidgetItem(QString::fromStdString(actual->data.brand)));
            ui->tablaProductos->setItem(fila, 3, new QTableWidgetItem(QString::fromStdString(actual->data.category)));
            ui->tablaProductos->setItem(fila, 4, new QTableWidgetItem(QString::number(actual->data.price, 'f', 2)));
            ui->tablaProductos->setItem(fila, 5, new QTableWidgetItem(QString::number(actual->data.stock)));
            ui->tablaProductos->setItem(fila, 6, new QTableWidgetItem(QString::fromStdString(actual->data.expiry_date)));

            // Tratamiento especial para la columna "Estado" (Colores)
            QTableWidgetItem* itemEstado = new QTableWidgetItem(QString::fromStdString(actual->data.estado));
            itemEstado->setFont(QFont("Arial", 9, QFont::Bold)); // Negrita para el estado

            if (actual->data.estado == "Agotado") {
                itemEstado->setForeground(QBrush(Qt::red));
            } else if (actual->data.estado == "En tránsito") {
                itemEstado->setForeground(QBrush(QColor(255, 140, 0))); // Naranja
            } else {
                itemEstado->setForeground(QBrush(Qt::darkGreen)); // Disponible
            }

            ui->tablaProductos->setItem(fila, 7, itemEstado);

            actual = actual->next;
            fila++;
        }

        QMessageBox::information(this, "Carga Completa", "Se mostraron " + QString::number(fila) + " productos.");
    }

    // --- Simulacion (Pestaña 4) ---
    void MainWindow::on_btnIniciarDespacho_clicked() {
        // BLOQUEO DE DOBLE CLIC
        ui->btnIniciarDespacho->setEnabled(false);
        ui->listaLogDespacho->clear();

        if (redNacional.estaVacio()) {
            QMessageBox::critical(this, "Error", "La red nacional está vacía.");
            ui->btnIniciarDespacho->setEnabled(true);
            return;
        }

        int idOrigen = ui->txtDespachoOrigen->text().toInt();
        int idDestino = ui->txtDespachoDestino->text().toInt();
        std::string codigoStr = ui->txtDespachoCodigo->text().toStdString();
        bool optTiempo = ui->rbOptTiempo->isChecked();

        if (codigoStr.empty() || ui->txtDespachoOrigen->text().isEmpty() || ui->txtDespachoDestino->text().isEmpty()) {
            QMessageBox::warning(this, "Aviso", "Todos los campos son obligatorios.");
            ui->btnIniciarDespacho->setEnabled(true);
            return;
        }

        // Calcular Ruta (Dijkstra)
        int ruta[100];
        int tamanoRuta = 0;

        if (!redNacional.obtenerRutaDijkstra(idOrigen, idDestino, optTiempo, ruta, tamanoRuta)) {
            ui->listaLogDespacho->addItem("❌ ERROR: No existe un camino viable entre estas sucursales.");
            ui->btnIniciarDespacho->setEnabled(true);
            return;
        }

        // Buscar el producto
        VertexNode* nodoOrigen = redNacional.buscarVertice(idOrigen);
        Product* p = nodoOrigen->sucursal.inventarioHash.buscar(codigoStr);

        if (p == nullptr) {
            ui->listaLogDespacho->addItem("❌ ERROR: El producto no existe en el origen.");
            ui->btnIniciarDespacho->setEnabled(true);
            return;
        }

        Product productoTransferencia = *p;
        productoTransferencia.estado = "En tránsito";

        // borramos el origen
        nodoOrigen->sucursal.inventarioHash.eliminar(productoTransferencia.barcode);
        nodoOrigen->sucursal.inventarioAVL.eliminar(productoTransferencia.name);
        nodoOrigen->sucursal.inventarioLista.eliminarPorCodigo(productoTransferencia.barcode);
        nodoOrigen->sucursal.inventarioB.eliminarProducto(productoTransferencia);
        nodoOrigen->sucursal.inventarioBPlus.eliminarProducto(productoTransferencia);

        // -- INICIO DE LA ANIMACION VISUAL DE COLAS
        int tiempoTotalSimulado = 0;
        double costoTotalSimulado = 0.0;
        auto log = [&](const QString& mensaje, int delayMs = 600) {
            // Obtenemos la hora exacta
            QString timestamp = QTime::currentTime().toString("HH:mm:ss");

            // Formateamos el texto [14:30:05] Mensaje...
            QString logText = QString("[%1] %2").arg(timestamp, mensaje);

            ui->listaLogDespacho->addItem(logText);
            ui->listaLogDespacho->scrollToBottom();
            QCoreApplication::processEvents();
            QThread::msleep(delayMs);
        };

        auto actualizarMapa = [&](int idNodoActivo) {
            redNacional.generarReporteSimulacion("animacion_temp.dot", idNodoActivo, ruta, tamanoRuta);
            system("dot -Tpng animacion_temp.dot -o animacion_temp.png");

            QThread::msleep(50); // Darle tiempo a Windows para que suelte el archivo

            QPixmap pixmap("animacion_temp.png");
            // Solo la mostramos si el archivo no está corrupto
            if (!pixmap.isNull()) {
                ui->lblMapaAnimado->setPixmap(pixmap.scaled(ui->lblMapaAnimado->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            }
            QCoreApplication::processEvents();
        };

        log(QString("▶ INGRESO A RED LOGÍSTICA: [ %1 ]").arg(QString::fromStdString(productoTransferencia.name)));
        log(QString("📍 Estado actual del producto: %1").arg(QString::fromStdString(productoTransferencia.estado)));
        log(QString("📍 Calculando ruta óptima por %1...").arg(optTiempo ? "TIEMPO" : "COSTO"));

        QString rutaStr = "Ruta: ";
        for (int i = 0; i < tamanoRuta; i++) rutaStr += QString::number(ruta[i]) + (i < tamanoRuta - 1 ? " -> " : "");
        log(rutaStr, 1000);
        int etaSegundos = 0;
        for (int i = 0; i < tamanoRuta; i++) {
            VertexNode* nodoETA = redNacional.buscarVertice(ruta[i]);

            if (i == 0) {
                etaSegundos += nodoETA->sucursal.t_ingreso + nodoETA->sucursal.t_despacho;
            } else if (i == tamanoRuta - 1) {
                etaSegundos += nodoETA->sucursal.t_ingreso;
            } else {
                etaSegundos += nodoETA->sucursal.t_ingreso + nodoETA->sucursal.t_traspaso + nodoETA->sucursal.t_despacho;
            }

            if (i < tamanoRuta - 1) {
                EdgeNode* arista = nodoETA->edges;
                while(arista != nullptr) {
                    if(arista->destino_id == ruta[i+1]) {
                        etaSegundos += arista->tiempo;
                        break;
                    }
                    arista = arista->next;
                }
            }
        }
        log(QString("⏳ ETA (Estimación de Llegada): %1 segundos de operación.").arg(etaSegundos), 1500);
        log("--------------------------------------------------", 200);

        // Recorrido por el Grafo usando las Colas
        for (int i = 0; i < tamanoRuta; i++) {
            VertexNode* nodoActual = redNacional.buscarVertice(ruta[i]);
            Product tmp;

            actualizarMapa(nodoActual->sucursal.id);

            if (i == 0) {
                log(QString("🏢 [ORIGEN] Sucursal %1:").arg(nodoActual->sucursal.id), 200);

                log(QString("   ↳ Ingreso a Recepción (+%1s)").arg(nodoActual->sucursal.t_ingreso));
                nodoActual->colaIngreso.encolar(productoTransferencia);
                tiempoTotalSimulado += nodoActual->sucursal.t_ingreso;
                QThread::msleep(nodoActual->sucursal.t_ingreso * 50);
                nodoActual->colaIngreso.desencolar(tmp);

                log(QString("   ↳ Despachando a ruta (+%1s)").arg(nodoActual->sucursal.t_despacho));
                nodoActual->colaSalida.encolar(productoTransferencia);
                tiempoTotalSimulado += nodoActual->sucursal.t_despacho;
                QThread::msleep(nodoActual->sucursal.t_despacho * 50);
                nodoActual->colaSalida.desencolar(tmp);

            } else if (i == tamanoRuta - 1) {
                log(QString("🏢 [DESTINO] Sucursal %1:").arg(nodoActual->sucursal.id), 200);

                log(QString("   ↳ Recepción final (+%1s)").arg(nodoActual->sucursal.t_ingreso));
                nodoActual->colaIngreso.encolar(productoTransferencia);
                tiempoTotalSimulado += nodoActual->sucursal.t_ingreso;
                QThread::msleep(nodoActual->sucursal.t_ingreso * 50);
                nodoActual->colaIngreso.desencolar(tmp);

            } else {
                log(QString("🔄 [INTERMEDIO] Sucursal %1:").arg(nodoActual->sucursal.id), 200);

                log(QString("   ↳ Descargando (+%1s)").arg(nodoActual->sucursal.t_ingreso));
                nodoActual->colaIngreso.encolar(productoTransferencia);
                tiempoTotalSimulado += nodoActual->sucursal.t_ingreso;
                QThread::msleep(nodoActual->sucursal.t_ingreso * 50);
                nodoActual->colaIngreso.desencolar(tmp);

                log(QString("   ↳ Traspaso interno (+%1s)").arg(nodoActual->sucursal.t_traspaso));
                nodoActual->colaPreparacion.encolar(productoTransferencia);
                tiempoTotalSimulado += nodoActual->sucursal.t_traspaso;
                QThread::msleep(nodoActual->sucursal.t_traspaso * 50);
                nodoActual->colaPreparacion.desencolar(tmp);

                log(QString("   ↳ Cargando a ruta (+%1s)").arg(nodoActual->sucursal.t_despacho));
                nodoActual->colaSalida.encolar(productoTransferencia);
                tiempoTotalSimulado += nodoActual->sucursal.t_despacho;
                QThread::msleep(nodoActual->sucursal.t_despacho * 50);
                nodoActual->colaSalida.desencolar(tmp);
            }

            if (i < tamanoRuta - 1) {
                // Buscamos el tiempo de viaje real en la arista de Dijkstra
                int tiempoViaje = 0;
                double costoViaje = 0.0;
                EdgeNode* arista = nodoActual->edges;
                while(arista != nullptr) {
                    if(arista->destino_id == ruta[i+1]) {
                        tiempoViaje = arista->tiempo;
                        costoViaje = arista->costo;
                        break;
                    }
                    arista = arista->next;
                }

                log(QString("🚚 Tránsito hacia S%1 (Viaje: %2s | Peaje/Flete: Q%3)...").arg(ruta[i+1]).arg(tiempoViaje).arg(costoViaje), 800);

                tiempoTotalSimulado += tiempoViaje;
                costoTotalSimulado += costoViaje; // <--- SUMAMOS EL DINERO
                log("--------------------------------------------------", 200);
            }
        }

        // Insercion en Destino
        log("\n[FASE FINAL: ALMACENAJE]");
        VertexNode* nodoDestino = redNacional.buscarVertice(idDestino);

        productoTransferencia.estado = (productoTransferencia.stock > 0) ? "Disponible" : "Agotado";
        log(QString("📍 Actualizando estado a: %1").arg(QString::fromStdString(productoTransferencia.estado)));

        Product* productoExistente = nodoDestino->sucursal.inventarioHash.buscar(productoTransferencia.barcode);

        if (productoExistente != nullptr) {
            log("📍 El producto ya existe en destino. Actualizando inventario local...");
            int nuevoStock = productoExistente->stock + productoTransferencia.stock;
            productoExistente->stock = nuevoStock;
            productoExistente->estado = "Disponible";
            log(QString("✔ Stock consolidado. Nuevo stock: %1 unidades.").arg(nuevoStock));
        } else {
            log("📍 Producto nuevo en destino. Indexando...");
            nodoDestino->sucursal.inventarioHash.insertar(productoTransferencia);
            nodoDestino->sucursal.inventarioAVL.insertar(productoTransferencia);
            nodoDestino->sucursal.inventarioB.insertar(productoTransferencia);
            nodoDestino->sucursal.inventarioBPlus.insertar(productoTransferencia);
            nodoDestino->sucursal.inventarioLista.insertarFinal(productoTransferencia);
            log("✔ Indexación completada exitosamente.");
        }

        log("--------------------------------------------------", 200);
        log(QString("⏱ TIEMPO TOTAL DE OPERACIÓN: %1 segundos.").arg(tiempoTotalSimulado));
        log(QString("💰 COSTO TOTAL DE TRÁNSITO: Q%1").arg(costoTotalSimulado)); // <--- NUEVO
        log("🎉 TRANSFERENCIA LOGÍSTICA FINALIZADA 🎉", 0);

        QMessageBox::information(this, "Operación Exitosa",
            QString("El producto completó su ruta y ya está disponible en su destino.\n\n"
                    "⏱ Tiempo total invertido: %1s\n"
                    "💰 Gasto total de ruta: Q%2")
                    .arg(tiempoTotalSimulado).arg(costoTotalSimulado));

        ui->txtDespachoCodigo->clear();

        ui->btnIniciarDespacho->setEnabled(true);
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
            pixmapOriginal = QPixmap(QString::fromStdString(nombrePng));
            if (!pixmapOriginal.isNull()) {
                escalaActual = 1.0; // Resetear zoom al generar nuevo reporte
                refrescarVistaZoom();
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

    void MainWindow::refrescarVistaZoom() {
        if (pixmapOriginal.isNull()) return;

        QSize nuevoTamano = pixmapOriginal.size() * escalaActual;
        QPixmap escalada = pixmapOriginal.scaled(nuevoTamano, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->lblImagenReporte->setPixmap(escalada);
        ui->lblImagenReporte->resize(nuevoTamano);
        ui->scrollArea->widget()->resize(nuevoTamano);
        ui->scrollArea->widget()->setMinimumSize(nuevoTamano);
    }

    void MainWindow::on_btnZoomIn_clicked() {
        escalaActual *= 1.2; // Aumentar 20%
        refrescarVistaZoom();
    }

    void MainWindow::on_btnZoomOut_clicked() {
        escalaActual /= 1.2; // Reducir 20%
        if (escalaActual < 0.1) escalaActual = 0.1; // Límite mínimo
        refrescarVistaZoom();
    }

    bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
        if (obj == ui->scrollArea && event->type() == QEvent::Wheel) {
            QWheelEvent *we = static_cast<QWheelEvent*>(event);

            // Verificamos si la tecla Ctrl está presionada para escalar
            if (we->modifiers() & Qt::ControlModifier) {
                if (we->angleDelta().y() > 0)
                    on_btnZoomIn_clicked();
                else
                    on_btnZoomOut_clicked();
                return true; // Bloqueamos el desplazamiento vertical para priorizar el zoom
            }
        }
        // Llamada obligatoria a la clase base
        return QMainWindow::eventFilter(obj, event);
    }