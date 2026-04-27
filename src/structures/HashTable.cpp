#include "HashTable.h"

HashTable::HashTable() {
}

int HashTable::hashFunction(const std::string& barcode) {
    long long hashVal = 0;
    for (char c : barcode) {
        hashVal = (hashVal * 31 + c) % TABLE_SIZE;
    }
    return hashVal;
}

bool HashTable::insertar(const Product& producto) {
    int index = hashFunction(producto.barcode);

    if (table[index].buscarPorCodigo(producto.barcode) != nullptr)
    {
        return false;
    }
    table[index].insertarInicio(producto);
    return true;
}

Product* HashTable::buscar(const std::string& barcode)
{
    int index = hashFunction(barcode);
    return table[index].buscarPorCodigo(barcode);
}

bool HashTable::eliminar(const std::string& barcode)
{
    int index = hashFunction(barcode);
    return table[index].eliminarPorCodigo(barcode);
}

void HashTable::imprimirDistribucion() {
    int totalProductos = 0;
    int celdasOcupadas = 0;

    for (int i = 0; i < TABLE_SIZE; ++i) {
        int tamanoLista = table[i].obtenerTamano();
        if (tamanoLista > 0) {
            celdasOcupadas++;
            totalProductos += tamanoLista;
        }
    }

    std::cout << "--- Estadisticas de Tabla Hash Refactorizada ---\n";
    std::cout << "Productos totales: " << totalProductos << "\n";
    std::cout << "Factor de carga: " << (float)totalProductos / TABLE_SIZE << "\n";
    std::cout << "Celdas con colisiones: " << (totalProductos - celdasOcupadas) << "\n";
}

void HashTable::generarReporte(const std::string& nombreArchivo) {
    std::ofstream archivo(nombreArchivo);
    if (!archivo.is_open()) {
        std::cout << "Error al abrir el archivo para el reporte Hash.\n";
        return;
    }

    int totalProductos = 0;
    int celdasOcupadas = 0;

    for (int i = 0; i < TABLE_SIZE; ++i) {
        int tamanoLista = table[i].obtenerTamano();
        if (tamanoLista > 0) {
            celdasOcupadas++;
            totalProductos += tamanoLista;
        }
    }

    int colisiones = totalProductos - celdasOcupadas;
    float factorCarga = (float)totalProductos / TABLE_SIZE;

    archivo << "digraph HashTable {\n";
    archivo << "  rankdir=LR;\n"; // De izquierda a derecha
    archivo << "  node [shape=record, fontname=\"Arial\"];\n\n";

    archivo << "  stats [shape=Mrecord, fillcolor=\"#E3F2FD\", style=filled, label=\"ESTADÍSTICAS TABLA HASH\\n";
    archivo << "Capacidad Total: " << TABLE_SIZE << "\\n";
    archivo << "Productos Almacenados: " << totalProductos << "\\n";
    archivo << "Colisiones Totales: " << colisiones << "\\n";
    archivo << "Factor de Carga: " << factorCarga << "\"];\n\n";

    for (int i = 0; i < TABLE_SIZE; ++i) {
        if (table[i].obtenerTamano() > 0) {

            archivo << "  bucket_" << i << " [shape=box, style=filled, fillcolor=\"#FFCDD2\", label=\"Índice " << i << "\"];\n";

            ListNode* actual = table[i].getInicio();
            int idxNodo = 0;

            while (actual != nullptr) {
                // Dibujar el nodo del producto
                archivo << "  node_" << i << "_" << idxNodo
                        << " [label=\"{ " << actual->data.barcode << " | " << actual->data.name << " }\"];\n";

                // Conectar el índice con el primer nodo, o los nodos entre sí
                if (idxNodo == 0) {
                    archivo << "  bucket_" << i << " -> node_" << i << "_" << idxNodo << " [color=\"#D32F2F\"];\n";
                } else {
                    archivo << "  node_" << i << "_" << (idxNodo - 1) << " -> node_" << i << "_" << idxNodo << ";\n";
                }

                actual = actual->next;
                idxNodo++;
            }
        }
    }

    archivo << "}\n";
    archivo.close();
    std::cout << "Reporte Hash generado exitosamente en: " << nombreArchivo << "\n";
}