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

class CSVReader {
public:
    static void cargarSucursales(const std::string& rutaArchivo, Graph& graph);
    static void cargarProductos(const std::string& rutaArchivo, Graph& redNacional);
    static void cargarConexiones(const std::string& rutaArchivo, Graph& graph);

private:
    static void logError(const std::string& mensaje);
    static int separarLineaCSV(const std::string& linea, std::string campos[], int maxCampos);
    static std::string limpiarCampo(std::string campo);
};

#endif //PROYECTO2_CSVREADER_H