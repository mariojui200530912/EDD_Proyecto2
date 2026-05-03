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

void HashTable::asegurarCapacidad(int elementosNuevos) {
    int elementosTotales = numElementos + elementosNuevos;
    float factorProyectado = (elementosTotales * 1.0f) / capacidad;

    if (factorProyectado >= 0.80f) {
        // Apuntamos a un factor de carga del 0.80
        int capacidadIdeal = (int)(elementosTotales / 0.80f);
        int nuevaCapacidad = siguientePrimo(capacidadIdeal);
        rehash(nuevaCapacidad);
    }
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

void HashTable::rehash(int nuevaCapacidad) {
    int viejaCapacidad = capacidad;
    LinkedList** tablaVieja = tabla;

    capacidad = nuevaCapacidad;

    // Creamos la nueva tabla
    tabla = new LinkedList*[capacidad];
    for (int i = 0; i < capacidad; i++) {
        tabla[i] = new LinkedList();
    }

    int contadorReal = 0;
    for (int i = 0; i < viejaCapacidad; i++) {
        if (tablaVieja[i] != nullptr) {
            ListNode* actual = tablaVieja[i]->getInicio();
            while (actual != nullptr) {
                int indice = funcionHash(actual->data.barcode);
                tabla[indice]->insertarFinal(actual->data);
                contadorReal++;
                actual = actual->next;
            }
            delete tablaVieja[i];
        }
    }
    delete[] tablaVieja;
    numElementos = contadorReal;

    std::cout << "Memoria reservada. Nueva capacidad exacta: " << capacidad << "\n";
}

bool HashTable::insertar(const Product& p, int cantidadIngresar) {
    // Validar el Factor de Carga ANTES de insertar
    asegurarCapacidad(cantidadIngresar);
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
    if (!archivo.is_open()) return;

    int totalProductos = 0;
    int celdasOcupadas = 0;
    for (int i = 0; i < capacidad; ++i) {
        if (tabla[i]->obtenerTamano() > 0) {
            celdasOcupadas++;
            totalProductos += tabla[i]->obtenerTamano();
        }
    }

    int colisiones = totalProductos - celdasOcupadas;
    float factorCarga = (capacidad > 0) ? (float)totalProductos / capacidad : 0;

    archivo << "digraph HashTable {\n";
    archivo << "  rankdir=LR;\n";
    archivo << "  node [fontname=\"Arial\"];\n\n";

    // --- BLOQUE DE ESTADÍSTICAS (ENCAPSULADO) ---
    // Usamos un nodo único dentro de un cluster para evitar duplicados
    archivo << "  subgraph cluster_stats {\n";
    archivo << "    label=\"INFORMACIÓN DEL SISTEMA\";\n";
    archivo << "    color=\"#1565C0\";\n";
    archivo << "    fontcolor=\"#1565C0\";\n";
    archivo << "    style=\"dashed,rounded\";\n";
    archivo << "    stats_node [shape=none, label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"8\" BGCOLOR=\"#F5F5F5\">\n";
    archivo << "      <TR><TD COLSPAN=\"2\" BGCOLOR=\"#1565C0\"><FONT COLOR=\"white\"><B>PANEL DE CONTROL - TABLA HASH</B></FONT></TD></TR>\n";
    archivo << "      <TR><TD ALIGN=\"LEFT\">Capacidad Actual:</TD><TD ALIGN=\"RIGHT\"><B>" << capacidad << "</B></TD></TR>\n";
    archivo << "      <TR><TD ALIGN=\"LEFT\">Total Productos:</TD><TD ALIGN=\"RIGHT\"><B>" << totalProductos << "</B></TD></TR>\n";
    archivo << "      <TR><TD ALIGN=\"LEFT\">Colisiones:</TD><TD ALIGN=\"RIGHT\"><FONT COLOR=\"#C62828\">" << colisiones << "</FONT></TD></TR>\n";
    archivo << "      <TR><TD ALIGN=\"LEFT\">Factor de Carga:</TD><TD ALIGN=\"RIGHT\">" << factorCarga << "</TD></TR>\n";
    archivo << "    </TABLE>>];\n";
    archivo << "  }\n\n";

    // Estilo para los buckets e índices
    archivo << "  node [shape=record, style=\"filled,rounded\"];\n";

    // --- ALINEACIÓN DE ÍNDICES ---
    archivo << "  subgraph cluster_indices {\n";
    archivo << "    style=invis;\n";
    for (int i = 0; i < capacidad; ++i) {
        if (tabla[i]->obtenerTamano() > 0) {
            archivo << "    bucket_" << i << " [shape=box, fillcolor=\"#FFCDD2\", color=\"#C62828\", label=\"Índice " << i << "\"];\n";
        }
    }
    archivo << "  }\n";

    // --- DIBUJADO DE PRODUCTOS ---
    for (int i = 0; i < capacidad; ++i) {
        if (tabla[i]->obtenerTamano() > 0) {
            ListNode* actual = tabla[i]->getInicio();
            int idx = 0;
            while (actual) {
                std::string nom = actual->data.name;
                // Limpieza de seguridad
                for (char& c : nom) {
                    if (c == '"' || c == '|' || c == '{' || c == '}' || c == '<' || c == '>') c = ' ';
                }

                archivo << "  node_" << i << "_" << idx << " [fillcolor=\"#E8F5E9\", color=\"#2E7D32\", label=\"{ " << actual->data.barcode << " | " << nom << " }\"];\n";

                if (idx == 0) {
                    archivo << "  bucket_" << i << " -> node_" << i << "_" << idx << " [color=\"#D32F2F\"];\n";
                } else {
                    archivo << "  node_" << i << "_" << (idx - 1) << " -> node_" << i << "_" << idx << " [color=\"#4CAF50\"];\n";
                }

                actual = actual->next;
                idx++;
            }
        }
    }

    // Forzamos que las estadísticas aparezcan arriba de los índices
    if (celdasOcupadas > 0) {
        // Buscamos el primer índice que exista para conectarlo invisiblemente
        for (int i = 0; i < capacidad; ++i) {
            if (tabla[i]->obtenerTamano() > 0) {
                archivo << "  stats_node -> bucket_" << i << " [style=invis];\n";
                break;
            }
        }
    }

    archivo << "}\n";
    archivo.close();
}