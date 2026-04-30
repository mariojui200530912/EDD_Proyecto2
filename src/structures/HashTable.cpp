#include "HashTable.h"

HashTable::HashTable(int tamano) {
    capacidad = tamano;
    numElementos = 0;
    tabla = new LinkedList*[capacidad];
    for (int i = 0; i < capacidad; i++) {
        tabla[i] = new LinkedList();
    }
}

HashTable::~HashTable() {
    for (int i = 0; i < capacidad; i++) {
        delete tabla[i];
    }
    delete[] tabla;
}

// Funcion HASH
int HashTable::funcionHash(const std::string& barcode) {
    unsigned long long valorHash = 0;
    const int primoBase = 31;

    for (char c : barcode) {
        valorHash = (valorHash * primoBase) + c;
    }
    return valorHash % capacidad;
}

// para el rehashing
bool HashTable::esPrimo(int n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (int i = 5; i * i <= n; i = i + 6) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    return true;
}

int HashTable::siguientePrimo(int n) {
    if (n <= 1) return 2;
    int primo = n;
    bool encontrado = false;
    while (!encontrado) {
        primo++;
        if (esPrimo(primo)) encontrado = true;
    }
    return primo;
}

void HashTable::rehash() {
    int viejaCapacidad = capacidad;
    LinkedList** tablaVieja = tabla;

    // Calculamos la nueva capacidad (El siguiente primo después del doble)
    capacidad = siguientePrimo(viejaCapacidad * 2);

    // Creamos la nueva tabla
    tabla = new LinkedList*[capacidad];
    for (int i = 0; i < capacidad; i++) {
        tabla[i] = new LinkedList();
    }

    // Reiniciamos el contador
    numElementos = 0;

    // Mudamos los productos
    for (int i = 0; i < viejaCapacidad; i++) {
        ListNode* actual = tablaVieja[i]->getInicio();
        while (actual != nullptr) {
            insertar(actual->data);
            actual = actual->next;
        }
        delete tablaVieja[i];
    }

    delete[] tablaVieja; // Destruimos el arreglo viejo
    std::cout << "Rehash ejecutado. Nueva capacidad de la tabla: " << capacidad << "\n";
}

bool HashTable::insertar(const Product& p) {
    // Validar el Factor de Carga ANTES de insertar
    float factorDeCarga = (float)numElementos / capacidad;
    if (factorDeCarga >= 0.8f) {
        rehash();
    }

    int indice = funcionHash(p.barcode);

    // Evitar duplicados
    ListNode* actual = tabla[indice]->getInicio();
    while (actual != nullptr) {
        if (actual->data.barcode == p.barcode) return false;
        actual = actual->next;
    }

    // Insertar
    tabla[indice]->insertarFinal(p);
    numElementos++; // Incrementamos el control
    return true;
}

Product* HashTable::buscar(const std::string& barcode) {
    int indice = funcionHash(barcode);
    ListNode* actual = tabla[indice]->getInicio();

    while (actual != nullptr) {
        if (actual->data.barcode == barcode) {
            return &(actual->data);
        }
        actual = actual->next;
    }
    return nullptr;
}

bool HashTable::eliminar(const std::string& barcode) {
    int indice = funcionHash(barcode);
    bool eliminado = tabla[indice]->eliminarPorCodigo(barcode);
    if (eliminado) {
        numElementos--; // Descontamos del control
    }
    return eliminado;
}

void HashTable::imprimirDistribucion() {
    int totalProductos = 0;
    int celdasOcupadas = 0;

    for (int i = 0; i < capacidad; ++i) {
        int tamanoLista = tabla[i]->obtenerTamano();
        if (tamanoLista > 0) {
            celdasOcupadas++;
            totalProductos += tamanoLista;
        }
    }

    std::cout << "--- Estadisticas de Tabla Hash Refactorizada ---\n";
    std::cout << "Productos totales: " << totalProductos << "\n";
    std::cout << "Factor de carga: " << (float)totalProductos / capacidad << "\n";
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

    for (int i = 0; i < capacidad; ++i) {
        int tamanoLista = tabla[i]->obtenerTamano();
        if (tamanoLista > 0) {
            celdasOcupadas++;
            totalProductos += tamanoLista;
        }
    }

    int colisiones = totalProductos - celdasOcupadas;
    float factorCarga = (float)totalProductos / capacidad;

    archivo << "digraph HashTable {\n";
    archivo << "  rankdir=LR;\n"; // De izquierda a derecha
    archivo << "  node [shape=record, fontname=\"Arial\"];\n\n";

    archivo << "  stats [shape=Mrecord, fillcolor=\"#E3F2FD\", style=filled, label=\"ESTADÍSTICAS TABLA HASH\\n";
    archivo << "Capacidad Total: " << capacidad << "\\n";
    archivo << "Productos Almacenados: " << totalProductos << "\\n";
    archivo << "Colisiones Totales: " << colisiones << "\\n";
    archivo << "Factor de Carga: " << factorCarga << "\"];\n\n";

    for (int i = 0; i < capacidad; ++i) {
        if (tabla[i]->obtenerTamano() > 0) {

            archivo << "  bucket_" << i << " [shape=box, style=filled, fillcolor=\"#FFCDD2\", label=\"Índice " << i << "\"];\n";

            ListNode* actual = tabla[i]->getInicio();
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