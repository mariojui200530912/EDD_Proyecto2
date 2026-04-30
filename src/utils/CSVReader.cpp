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
void CSVReader::cargarSucursales(const std::string& rutaArchivo, Graph& graph) {
    std::ifstream archivo(rutaArchivo);

    if (!archivo.is_open()) {
        logError("ERROR CRÍTICO: No se pudo abrir el archivo de sucursales: " + rutaArchivo);
        std::cout << "Error al abrir el archivo. Revisa errors.log\n";
        return;
    }

    std::string linea;
    bool primeraLinea = true;
    int numeroLinea = 0;

    const int MAX_CAMPOS = 6;
    std::string campos[MAX_CAMPOS];

    while (std::getline(archivo, linea)) {
        numeroLinea++;

        if (primeraLinea) {
            primeraLinea = false;
            continue;
        }

        if (linea.empty() || linea == "\r") continue;

        int numCamposEncontrados = separarLineaCSV(linea, campos, MAX_CAMPOS);

        if (numCamposEncontrados != MAX_CAMPOS) {
            logError("Linea " + std::to_string(numeroLinea) + " mal formateada (columnas incorrectas): " + linea);
            continue;
        }

        if (!DataValidator::esNumeroValido(campos[0], false) ||
            !DataValidator::esNumeroValido(campos[3], false) ||
            !DataValidator::esNumeroValido(campos[4], false) ||
            !DataValidator::esNumeroValido(campos[5], false)) {
            logError("Sucursal Linea " + std::to_string(numeroLinea) + " contiene texto en campos numéricos.");
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

            // CORRECCIÓN: Inserción directa en el Grafo
            graph.insertarSucursal(nuevaSucursal);
            std::cout << "Exito: Sucursal " << nuevaSucursal.nombre << " cargada.\n";

        } catch (const std::exception& e) {
            logError("Sucursal Linea " + std::to_string(numeroLinea) + " error en conversión de tipos.");
        }
    }
    archivo.close();
}


// -- Carga de conexiones --
void CSVReader::cargarConexiones(const std::string& rutaArchivo, Graph& graph) {
    std::ifstream archivo(rutaArchivo);
    if (!archivo.is_open()) {
        logError("ERROR CRÍTICO: No se pudo abrir conexiones: " + rutaArchivo);
        return;
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

        int numCampos = separarLineaCSV(linea, campos, MAX_CAMPOS);

        if (numCampos != MAX_CAMPOS) {
            logError("Conexion Linea " + std::to_string(numeroLinea) + " columnas incorrectas: " + linea);
            continue;
        }

        if (!DataValidator::esNumeroValido(campos[0], false) ||
            !DataValidator::esNumeroValido(campos[1], false) ||
            !DataValidator::esNumeroValido(campos[2], false) ||
            !DataValidator::esNumeroValido(campos[3], true)) {
            logError("Conexion Linea " + std::to_string(numeroLinea) + " contiene texto en campos numéricos.");
            continue;
        }

        try {
            Connection nuevaConexion;
            nuevaConexion.origen_id = std::stoi(campos[0]);
            nuevaConexion.destino_id = std::stoi(campos[1]);
            nuevaConexion.tiempo = std::stoi(campos[2]);
            nuevaConexion.costo = std::stod(campos[3]);

            graph.insertarConexion(nuevaConexion, false);

            std::cout << "Exito: Conexion " << nuevaConexion.origen_id << " -> " << nuevaConexion.destino_id << " leida.\n";

        } catch (const std::exception& e) {
            logError("Conexion Linea " + std::to_string(numeroLinea) + " error de tipos: " + linea);
        }
    }
    archivo.close();
}

// -- Carga de Productos (Index y Rollback) ---

void CSVReader::cargarProductos(const std::string& rutaArchivo, Graph& redNacional, int idSucursalDestino) {
    // Validar la sucursal ANTES de procesar el archivo
    VertexNode* nodoSucursal = redNacional.buscarVertice(idSucursalDestino);

    if (nodoSucursal == nullptr) {
        logError("Carga abortada: La sucursal destino " + std::to_string(idSucursalDestino) + " no existe en el mapa.");
        std::cout << "Error: Sucursal destino no encontrada.\n";
        return;
    }

    // Extraemos la referencia a la sucursal
    Sucursal& miSucursal = nodoSucursal->sucursal;

    std::ifstream archivo(rutaArchivo);
    if (!archivo.is_open()) {
        logError("ERROR CRÍTICO: No se pudo abrir productos: " + rutaArchivo);
        return;
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

        int numCampos = separarLineaCSV(linea, campos, MAX_CAMPOS);

        if (numCampos != MAX_CAMPOS) {
            logError("Producto Linea " + std::to_string(numeroLinea) + " columnas incorrectas: " + linea);
            continue;
        }

        bool hayCamposVacios = false;
        for (int i = 0; i < MAX_CAMPOS; i++) {
            if (campos[i].empty()) {
                hayCamposVacios = true;
                break;
            }
        }
        if (hayCamposVacios) {
            logError("Producto Linea " + std::to_string(numeroLinea) + " omitido por contener campos vacíos.");
            continue;
        }

        if (campos[1].length() != 10) {
            logError("Producto Linea " + std::to_string(numeroLinea) + " barcode inválido (debe tener 10 caracteres): " + campos[1]);
            continue;
        }

        if (!DataValidator::esFechaValida(campos[3])) {
            logError("Producto Linea " + std::to_string(numeroLinea) + " fecha inválida (formato esperado YYYY-MM-DD): " + campos[3]);
            continue;
        }

        if (!DataValidator::esNumeroValido(campos[5], true) ||
            !DataValidator::esNumeroValido(campos[6], false)) {
            logError("Producto Linea " + std::to_string(numeroLinea) + " contiene texto en campos numéricos.");
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

            // Inserción Atomizada en el inventario LOCAL
            if (!miSucursal.inventarioHash.insertar(nuevoProducto)) {
                logError("Producto omitido por duplicado: " + nuevoProducto.barcode);
                continue;
            }

            try {
                miSucursal.inventarioAVL.insertar(nuevoProducto);
                miSucursal.inventarioB.insertar(nuevoProducto);
                miSucursal.inventarioBPlus.insertar(nuevoProducto);
                miSucursal.inventarioLista.insertarFinal(nuevoProducto);

                // Lo guardamos en la pila local para Ctrl+Z
                miSucursal.pilaRollback.apilar(nuevoProducto);

                std::cout << "Exito: " << nuevoProducto.name << " indexado en Sucursal " << miSucursal.nombre << ".\n";
            } catch (...) {
                miSucursal.inventarioHash.eliminar(nuevoProducto.barcode);
                miSucursal.inventarioAVL.eliminar(nuevoProducto.name);
                miSucursal.inventarioB.eliminarProducto(nuevoProducto);
                miSucursal.inventarioBPlus.eliminarProducto(nuevoProducto);
                miSucursal.inventarioLista.eliminarPorCodigo(nuevoProducto.barcode);
            }

        } catch (const std::exception& e) {
            logError("Producto Linea " + std::to_string(numeroLinea) + " error de tipos: " + linea);
        }
    }
    archivo.close();
}