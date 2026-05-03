#ifndef PROYECTO2_CSVREADER_H
#define PROYECTO2_CSVREADER_H

#include <string>
#include <fstream>
#include <iostream>
#include "../utils/DataValidator.h"
#include "../models/Sucursal.h"
#include "../models/Product.h"
#include "../models/Connection.h"
#include "../structures/HashTable.h"
#include "../structures/AVLTree.h"
#include "../structures/BTree.h"
#include "../structures/BTreePlus.h"
#include "../structures/Stack.h"
#include "../structures/Graph.h"

struct ReporteCarga
{
    int totalLeidos = 0;
    int ingresados = 0;
    int errores = 0;
    std::string detalleErrores = "";
};

class CSVReader {
public:
    static ReporteCarga cargarSucursales(const std::string& rutaArchivo, Graph& graph);
    static ReporteCarga cargarProductos(const std::string& rutaArchivo, Graph& redNacional, int idSucursalDestino);
    static ReporteCarga cargarConexiones(const std::string& rutaArchivo, Graph& graph);

private:
    static void logError(const std::string& mensaje);
    static int separarLineaCSV(const std::string& linea, std::string campos[], int maxCampos);
    static std::string limpiarCampo(std::string campo);
};

#endif //PROYECTO2_CSVREADER_H