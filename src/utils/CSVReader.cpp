#include "CSVReader.h"

// Funciones auxiliares
void CSVReader::logError(const std::string& mensaje) {
    std::ofstream archivoLog("errors.log", std::ios_base::app);
    if (archivoLog.is_open()) {
        archivoLog << mensaje << "\n";
        archivoLog.close();
    }
}

std::string CSVReader::limpiarCampo(std::string campo) {
    std::string resultado = "";
    for (char c : campo) {
        if (c != '"' && c != '\r' && c != '\n') {
            resultado += c;
        }
    }
    return resultado;
}

int CSVReader::separarLineaCSV(const std::string& linea, std::string campos[], int maxCampos) {
    int contador = 0;
    std::string actual = "";
    bool enComillas = false;

    for (char c : linea) {
        if (c == '"') {
            enComillas = !enComillas;
        } else if (c == ',' && !enComillas) {
            if (contador < maxCampos) {
                campos[contador] = limpiarCampo(actual);
                contador++;
                actual = "";
            }
        } else {
            actual += c;
        }
    }

    if (contador < maxCampos) {
        campos[contador] = limpiarCampo(actual);
        contador++;
    }
    return contador;
}

// -- Cargar Sucursales --
ReporteCarga CSVReader::cargarSucursales(const std::string& rutaArchivo, Graph& graph) {
    ReporteCarga reporte;

    std::ofstream clearLog("errors.log", std::ios_base::trunc);
    clearLog.close();

    std::ifstream archivo(rutaArchivo);
    if (!archivo.is_open()) {
        reporte.errores++;
        reporte.detalleErrores = "Error: No se pudo abrir el archivo de sucursales.";
        return reporte;
    }

    std::string linea;
    bool primeraLinea = true;
    int numeroLinea = 0;
    const int MAX_CAMPOS = 6;
    std::string campos[MAX_CAMPOS];

    while (std::getline(archivo, linea)) {
        numeroLinea++;
        if (primeraLinea) { primeraLinea = false; continue; }
        if (linea.empty() || linea == "\r") continue;

        reporte.totalLeidos++;
        int numCamposEncontrados = separarLineaCSV(linea, campos, MAX_CAMPOS);

        if (numCamposEncontrados != MAX_CAMPOS) {
            std::string msg = "Linea " + std::to_string(numeroLinea) + " mal formateada (columnas incorrectas): " + linea;
            reporte.errores++; reporte.detalleErrores += msg + "\n"; logError(msg);
            continue;
        }

        if (!DataValidator::esNumeroValido(campos[0], false) ||
            !DataValidator::esNumeroValido(campos[3], false) ||
            !DataValidator::esNumeroValido(campos[4], false) ||
            !DataValidator::esNumeroValido(campos[5], false)) {
            std::string msg = "Sucursal Linea " + std::to_string(numeroLinea) + " contiene texto en campos numéricos.";
            reporte.errores++; reporte.detalleErrores += msg + "\n"; logError(msg);
            continue;
        }

        try {
            Sucursal nuevaSucursal;
            nuevaSucursal.id = std::stoi(campos[0]);
            nuevaSucursal.nombre = campos[1];
            nuevaSucursal.ubicacion = campos[2];
            nuevaSucursal.t_ingreso = std::stoi(campos[3]);
            nuevaSucursal.t_traspaso = std::stoi(campos[4]);
            nuevaSucursal.t_despacho = std::stoi(campos[5]);

            if (graph.buscarVertice(nuevaSucursal.id) == nullptr) {
                graph.insertarSucursal(nuevaSucursal);
                reporte.ingresados++;
            } else {
                std::string msg = "Sucursal Linea " + std::to_string(numeroLinea) + " ID Duplicado: " + campos[0];
                reporte.errores++; reporte.detalleErrores += msg + "\n"; logError(msg);
            }
        } catch (const std::exception& e) {
            std::string msg = "Sucursal Linea " + std::to_string(numeroLinea) + " error en conversión de tipos.";
            reporte.errores++; reporte.detalleErrores += msg + "\n"; logError(msg);
        }
    }
    archivo.close();
    return reporte;
}


// -- Carga de conexiones --
ReporteCarga CSVReader::cargarConexiones(const std::string& rutaArchivo, Graph& graph) {
    ReporteCarga reporte;
    std::ofstream clearLog("errors.log", std::ios_base::trunc); clearLog.close();

    std::ifstream archivo(rutaArchivo);
    if (!archivo.is_open()) {
        reporte.errores++;
        reporte.detalleErrores = "ERROR CRÍTICO: No se pudo abrir conexiones: " + rutaArchivo;
        return reporte;
    }

    std::string linea;
    bool primeraLinea = true;
    int numeroLinea = 0;

    const int MAX_CAMPOS = 4;
    std::string campos[MAX_CAMPOS];

    while (std::getline(archivo, linea)) {
        numeroLinea++;
        if (primeraLinea) { primeraLinea = false; continue; }
        if (linea.empty() || linea == "\r") continue;

        reporte.totalLeidos++;
        int numCampos = separarLineaCSV(linea, campos, MAX_CAMPOS);

        if (numCampos != 4) {
            std::string msg = "Conexion Linea " + std::to_string(numeroLinea) + " columnas incorrectas: " + linea;
            reporte.errores++; reporte.detalleErrores += msg + "\n"; logError(msg);
            continue;
        }

        if (!DataValidator::esNumeroValido(campos[0], false) ||
            !DataValidator::esNumeroValido(campos[1], false) ||
            !DataValidator::esNumeroValido(campos[2], false) ||
            !DataValidator::esNumeroValido(campos[3], true)) {
            std::string msg = "Conexion Linea " + std::to_string(numeroLinea) + " contiene texto en campos numéricos.";
            reporte.errores++; reporte.detalleErrores += msg + "\n"; logError(msg);
            continue;
        }

        try {
            Connection nuevaConexion;
            nuevaConexion.origen_id = std::stoi(campos[0]);
            nuevaConexion.destino_id = std::stoi(campos[1]);
            nuevaConexion.tiempo = std::stoi(campos[2]);
            nuevaConexion.costo = std::stod(campos[3]);

            bool esBidireccional = false;

            if (graph.buscarVertice(nuevaConexion.origen_id) != nullptr &&
                graph.buscarVertice(nuevaConexion.destino_id) != nullptr) {

                graph.insertarConexion(nuevaConexion, esBidireccional);
                reporte.ingresados++;
            } else {
                std::string msg = "Conexion Linea " + std::to_string(numeroLinea) + " - Sucursal origen o destino inexistente.";
                reporte.errores++; reporte.detalleErrores += msg + "\n"; logError(msg);
            }
        } catch (const std::exception& e) {
            std::string msg = "Conexion Linea " + std::to_string(numeroLinea) + " error de tipos: " + linea;
            reporte.errores++; reporte.detalleErrores += msg + "\n"; logError(msg);
        }
    }
    archivo.close();
    return reporte;
}


// -- Carga de Productos (Index y Rollback) ---
ReporteCarga CSVReader::cargarProductos(const std::string& rutaArchivo, Graph& redNacional, int idSucursalDestino) {
    ReporteCarga reporte;
    std::ofstream clearLog("errors.log", std::ios_base::trunc); clearLog.close();

    VertexNode* nodoSucursal = redNacional.buscarVertice(idSucursalDestino);

    if (nodoSucursal == nullptr) {
        std::string msg = "Carga abortada: La sucursal destino " + std::to_string(idSucursalDestino) + " no existe en el mapa.";
        reporte.errores++; reporte.detalleErrores = msg; logError(msg);
        return reporte;
    }

    Sucursal& miSucursal = nodoSucursal->sucursal;
    int totalProyectado = 0;
    std::ifstream archivoConteo(rutaArchivo);
    if (archivoConteo.is_open()) {
        std::string tempLinea;
        bool skipPrimera = true;
        while (std::getline(archivoConteo, tempLinea)) {
            if (skipPrimera) { skipPrimera = false; continue; }
            if (!tempLinea.empty() && tempLinea != "\r") {
                totalProyectado++;
            }
        }
        archivoConteo.close();

        if (totalProyectado > 0) {
            miSucursal.inventarioHash.asegurarCapacidad(totalProyectado);
        }
    }

    std::ifstream archivo(rutaArchivo);
    if (!archivo.is_open()) {
        std::string msg = "ERROR CRÍTICO: No se pudo abrir productos: " + rutaArchivo;
        reporte.errores++; reporte.detalleErrores = msg; logError(msg);
        return reporte;
    }

    std::string linea;
    bool primeraLinea = true;
    int numeroLinea = 0;
    const int MAX_CAMPOS = 7;
    std::string campos[MAX_CAMPOS];

    while (std::getline(archivo, linea)) {
        numeroLinea++;
        if (primeraLinea) { primeraLinea = false; continue; }
        if (linea.empty() || linea == "\r") continue;

        reporte.totalLeidos++;
        int numCampos = separarLineaCSV(linea, campos, MAX_CAMPOS);

        if (numCampos != MAX_CAMPOS) {
            std::string msg = "Producto Linea " + std::to_string(numeroLinea) + " columnas incorrectas: " + linea;
            reporte.errores++; reporte.detalleErrores += msg + "\n"; logError(msg);
            continue;
        }

        bool hayCamposVacios = false;
        for (int i = 0; i < MAX_CAMPOS; i++) {
            if (campos[i].empty()) { hayCamposVacios = true; break; }
        }
        if (hayCamposVacios) {
            std::string msg = "Producto Linea " + std::to_string(numeroLinea) + " omitido por contener campos vacíos.";
            reporte.errores++; reporte.detalleErrores += msg + "\n"; logError(msg);
            continue;
        }

        if (campos[1].length() != 10) {
            std::string msg = "Producto Linea " + std::to_string(numeroLinea) + " barcode inválido: " + campos[1];
            reporte.errores++; reporte.detalleErrores += msg + "\n"; logError(msg);
            continue;
        }

        if (!DataValidator::esFechaValida(campos[3])) {
            std::string msg = "Producto Linea " + std::to_string(numeroLinea) + " fecha inválida: " + campos[3];
            reporte.errores++; reporte.detalleErrores += msg + "\n"; logError(msg);
            continue;
        }

        if (!DataValidator::esNumeroValido(campos[5], true) ||
            !DataValidator::esNumeroValido(campos[6], false)) {
            std::string msg = "Producto Linea " + std::to_string(numeroLinea) + " contiene texto en campos numéricos.";
            reporte.errores++; reporte.detalleErrores += msg + "\n"; logError(msg);
            continue;
        }

        try {
            Product nuevoProducto;
            nuevoProducto.name = campos[0];
            nuevoProducto.barcode = campos[1];
            nuevoProducto.category = campos[2];
            nuevoProducto.expiry_date = campos[3];
            nuevoProducto.brand = campos[4];
            nuevoProducto.price = std::stod(campos[5]);
            nuevoProducto.stock = std::stoi(campos[6]);
            nuevoProducto.estado = "Disponible";

            // Tu Inserción Atomizada
            if (!miSucursal.inventarioHash.insertar(nuevoProducto)) {
                std::string msg = "Producto duplicado omitido: " + nuevoProducto.barcode;
                reporte.errores++; reporte.detalleErrores += msg + "\n"; logError(msg);
                continue;
            }

            try {
                miSucursal.inventarioAVL.insertar(nuevoProducto);
                miSucursal.inventarioB.insertar(nuevoProducto);
                miSucursal.inventarioBPlus.insertar(nuevoProducto);
                miSucursal.inventarioLista.insertarFinal(nuevoProducto);
                // Lo guardamos en la pila local para Ctrl+Z
                miSucursal.pilaRollback.apilar(nuevoProducto);
                reporte.ingresados++;

            } catch (...) {
                miSucursal.inventarioHash.eliminar(nuevoProducto.barcode);
                miSucursal.inventarioAVL.eliminar(nuevoProducto.name);
                miSucursal.inventarioB.eliminarProducto(nuevoProducto);
                miSucursal.inventarioBPlus.eliminarProducto(nuevoProducto);
                miSucursal.inventarioLista.eliminarPorCodigo(nuevoProducto.barcode);

                std::string msg = "Fallo en cascada de árboles para producto: " + nuevoProducto.barcode;
                reporte.errores++; reporte.detalleErrores += msg + "\n"; logError(msg);
            }

        } catch (const std::exception& e) {
            std::string msg = "Producto Linea " + std::to_string(numeroLinea) + " error de tipos: " + linea;
            reporte.errores++; reporte.detalleErrores += msg + "\n"; logError(msg);
        }
    }
    archivo.close();
    return reporte;
}