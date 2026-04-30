#include "BTree.h"

BTreeNode::BTreeNode(int _d, bool _leaf) {
    d = _d;
    leaf = _leaf;
    n = 0;

    keys = new DateKey[2 * d + 1];
    children = new BTreeNode*[2 * d + 2];

    for(int i = 0; i < 2 * d + 2; i++) {
        children[i] = nullptr;
    }
}

BTreeNode::~BTreeNode() {
    for (int i = 0; i < n; i++) {
        delete keys[i].list;
    }
    delete[] keys;

    if (!leaf) {
        for (int i = 0; i <= n; i++) {
            delete children[i];
        }
    }
    delete[] children;
}

BTreeNode* BTree::buscarNodo(BTreeNode* node, const std::string& date) {
    if (node == nullptr) return nullptr;
    int i = 0;
    while (i < node->n && date > node->keys[i].date) i++;

    if (i < node->n && node->keys[i].date == date) return node;
    if (node->leaf) return nullptr;
    return buscarNodo(node->children[i], date);
}

void BTreeNode::buscarRangoRecursivo(const std::string& fechaInicio, const std::string& fechaFin, LinkedList& resultados) {
    int i = 0;
    for (i = 0; i < n; i++) {
        if (!leaf && keys[i].date >= fechaInicio) {
            children[i]->buscarRangoRecursivo(fechaInicio, fechaFin, resultados);
        }

        // Si la fecha está en el rango, copiamos todos los productos de esa fecha a la lista de resultados
        if (keys[i].date >= fechaInicio && keys[i].date <= fechaFin) {
            ListNode* actual = keys[i].list->getInicio();
            while (actual != nullptr) {
                resultados.insertarFinal(actual->data);
                actual = actual->next;
            }
        }
    }
    if (!leaf && i > 0 && keys[i-1].date <= fechaFin) {
        children[i]->buscarRangoRecursivo(fechaInicio, fechaFin, resultados);
    }
}


BTree::BTree(int _d) {
    root = nullptr;
    d = _d;
}

BTree::~BTree() {
    delete root;
}

void BTree::insertar(const Product& k) {
    if (root == nullptr) {
        root = new BTreeNode(d, true);
        root->keys[0].date = k.expiry_date;
        root->keys[0].list = new LinkedList();
        root->keys[0].list->insertarFinal(k);
        root->n = 1;
        return;
    }

    BTreeSplit resultado = insertarRecursivo(root, k);

    if (resultado.isSplit) {
        BTreeNode* nuevaRaiz = new BTreeNode(d, false);
        nuevaRaiz->keys[0] = resultado.promotedKey;
        nuevaRaiz->children[0] = root;
        nuevaRaiz->children[1] = resultado.rightNode;
        nuevaRaiz->n = 1;
        root = nuevaRaiz;
    }
}

BTreeSplit BTree::insertarRecursivo(BTreeNode* node, const Product& k) {
    BTreeSplit resultado = {DateKey(), nullptr, false};

    for (int j = 0; j < node->n; j++) {
        if (node->keys[j].date == k.expiry_date) {
            node->keys[j].list->insertarFinal(k); // Añadimos a la lista existente
            return resultado;
        }
    }

    int i = node->n - 1;

    if (node->leaf) {
        while (i >= 0 && node->keys[i].date > k.expiry_date) {
            node->keys[i + 1] = node->keys[i];
            i--;
        }
        node->keys[i + 1].date = k.expiry_date;
        node->keys[i + 1].list = new LinkedList();
        node->keys[i + 1].list->insertarFinal(k);
        node->n++;
    } else {
        while (i >= 0 && node->keys[i].date > k.expiry_date) {
            i--;
        }
        i++;

        BTreeSplit hijoResultado = insertarRecursivo(node->children[i], k);

        if (hijoResultado.isSplit) {
            int j = node->n - 1;
            while (j >= i) {
                node->keys[j + 1] = node->keys[j];
                node->children[j + 2] = node->children[j + 1];
                j--;
            }
            node->keys[i] = hijoResultado.promotedKey;
            node->children[i + 1] = hijoResultado.rightNode;
            node->n++;
        }
    }

    if (node->n > 2 * d) {
        dividirNodo(node, resultado);
    }

    return resultado;
}

void BTree::dividirNodo(BTreeNode* node, BTreeSplit& splitResult) {
    int medianaIndex = d;

    BTreeNode* nuevoNodo = new BTreeNode(d, node->leaf);
    nuevoNodo->n = d;
    for (int j = 0; j < d; j++) {
        nuevoNodo->keys[j] = node->keys[medianaIndex + 1 + j];
    }

    if (!node->leaf) {
        for (int j = 0; j <= d; j++) {
            nuevoNodo->children[j] = node->children[medianaIndex + 1 + j];
        }
    }
    node->n = d;
    splitResult.promotedKey = node->keys[medianaIndex];
    splitResult.rightNode = nuevoNodo;
    splitResult.isSplit = true;
}

bool BTree::eliminarProducto(const Product& k) {
    if (root == nullptr) return false;

    // Buscamos el nodo que contiene la fecha
    BTreeNode* nodoObjetivo = buscarNodo(root, k.expiry_date);

    if (nodoObjetivo != nullptr) {
        int i = nodoObjetivo->encontrarClave(k.expiry_date);

        // Eliminamos el producto de la lista enlazada
        bool productoBorrado = nodoObjetivo->keys[i].list->eliminarPorCodigo(k.barcode);

        if (productoBorrado) {
            // Si la lista quedo vacia, borramos la estructura
            if (nodoObjetivo->keys[i].list->estaVacia()) {
                delete nodoObjetivo->keys[i].list;
                nodoObjetivo->keys[i].list = nullptr;

                root->eliminarEstructural(k.expiry_date);

                // Si la raiz se quedo sin claves, su primer hijo es la nueva raiz
                if (root->n == 0) {
                    BTreeNode* tmp = root;
                    if (root->leaf) {
                        root = nullptr;
                    } else {
                        root = root->children[0];
                    }
                    delete tmp;
                }
            }
            return true;
        }
    }
    return false;
}

void BTree::buscarPorRangoFechas(const std::string& fechaInicio, const std::string& fechaFin, LinkedList& resultados) {
    if (root != nullptr) {
        root->buscarRangoRecursivo(fechaInicio, fechaFin, resultados);
    }
}

int BTreeNode::encontrarClave(const std::string& date) {
    int idx = 0;
    while (idx < n && keys[idx].date < date) ++idx;
    return idx;
}

void BTreeNode::eliminarEstructural(const std::string& date) {
    int idx = encontrarClave(date);

    if (idx < n && keys[idx].date == date) {
        if (leaf) {
            removerDeHoja(idx);
        } else {
            removerDeNoHoja(idx);
        }
    } else {
        if (leaf) return;

        bool esUltimoHijo = (idx == n);

        // Si el hijo donde vamos a bajar tiene menos del minimo (d), lo rellenamos
        if (children[idx]->n < d) {
            llenarNodo(idx);
        }

        // Si el ultimo hijo fue fusionado con el anterior, bajamos por el anterior
        if (esUltimoHijo && idx > n) {
            children[idx - 1]->eliminarEstructural(date);
        } else {
            children[idx]->eliminarEstructural(date);
        }
    }
}

void BTreeNode::removerDeHoja(int idx) {
    for (int i = idx + 1; i < n; ++i) {
        keys[i - 1] = keys[i];
    }
    n--;
}

void BTreeNode::removerDeNoHoja(int idx) {
    std::string date = keys[idx].date;

    // Caso 1: El hijo izquierdo tiene suficientes claves (>= d+1)
    if (children[idx]->n >= d + 1) {
        DateKey pred = obtenerPredecesor(idx);
        keys[idx] = pred;
        children[idx]->eliminarEstructural(pred.date);
    }
    // Caso 2: El hijo derecho tiene suficientes claves (>= d+1)
    else if (children[idx + 1]->n >= d + 1) {
        DateKey succ = obtenerSucesor(idx);
        keys[idx] = succ;
        children[idx + 1]->eliminarEstructural(succ.date);
    }
    // Caso 3: Ambos tienen solo d claves. Los fusionamos.
    else {
        fusionar(idx);
        children[idx]->eliminarEstructural(date);
    }
}

DateKey BTreeNode::obtenerPredecesor(int idx) {
    BTreeNode* cur = children[idx];
    while (!cur->leaf) cur = cur->children[cur->n];
    return cur->keys[cur->n - 1];
}

DateKey BTreeNode::obtenerSucesor(int idx) {
    BTreeNode* cur = children[idx + 1];
    while (!cur->leaf) cur = cur->children[0];
    return cur->keys[0];
}

void BTreeNode::llenarNodo(int idx) {
    if (idx != 0 && children[idx - 1]->n >= d + 1) {
        pedirPrestadoAnterior(idx);
    } else if (idx != n && children[idx + 1]->n >= d + 1) {
        pedirPrestadoSiguiente(idx);
    } else {
        if (idx != n) {
            fusionar(idx);
        } else {
            fusionar(idx - 1);
        }
    }
}

void BTreeNode::pedirPrestadoAnterior(int idx) {
    BTreeNode* hijo = children[idx];
    BTreeNode* hermano = children[idx - 1];

    for (int i = hijo->n - 1; i >= 0; --i) hijo->keys[i + 1] = hijo->keys[i];
    if (!hijo->leaf) {
        for (int i = hijo->n; i >= 0; --i) hijo->children[i + 1] = hijo->children[i];
    }

    hijo->keys[0] = keys[idx - 1];
    if (!hijo->leaf) hijo->children[0] = hermano->children[hermano->n];

    keys[idx - 1] = hermano->keys[hermano->n - 1];

    hijo->n += 1;
    hermano->n -= 1;
}

void BTreeNode::pedirPrestadoSiguiente(int idx) {
    BTreeNode* hijo = children[idx];
    BTreeNode* hermano = children[idx + 1];

    hijo->keys[(hijo->n)] = keys[idx];
    if (!hijo->leaf) hijo->children[(hijo->n) + 1] = hermano->children[0];

    keys[idx] = hermano->keys[0];

    for (int i = 1; i < hermano->n; ++i) hermano->keys[i - 1] = hermano->keys[i];
    if (!hermano->leaf) {
        for (int i = 1; i <= hermano->n; ++i) hermano->children[i - 1] = hermano->children[i];
    }

    hijo->n += 1;
    hermano->n -= 1;
}

void BTreeNode::fusionar(int idx) {
    BTreeNode* hijo = children[idx];
    BTreeNode* hermano = children[idx + 1];

    // Bajamos la clave del padre al hijo
    hijo->keys[d - 1] = keys[idx];

    // Copiamos las claves del hermano al hijo
    for (int i = 0; i < hermano->n; ++i) {
        hijo->keys[i + d] = hermano->keys[i];
    }

    // Copiamos los hijos del hermano
    if (!hijo->leaf) {
        for (int i = 0; i <= hermano->n; ++i) {
            hijo->children[i + d] = hermano->children[i];
        }
    }

    // Movemos las claves del padre para llenar el hueco
    for (int i = idx + 1; i < n; ++i) {
        keys[i - 1] = keys[i];
    }
    for (int i = idx + 2; i <= n; ++i) {
        children[i - 1] = children[i];
    }

    hijo->n += hermano->n + 1;
    n--;

    delete hermano;
}


// estructura auxiliar para generar reporte  debido a la busqueda por anchura(BFS)
struct ColaNodosB {
    BTreeNode* nodoArbol;
    ColaNodosB* siguiente;
    ColaNodosB(BTreeNode* n) : nodoArbol(n), siguiente(nullptr) {}
};

void BTree::generarReporte(const std::string& nombreArchivo) {
    std::ofstream archivo(nombreArchivo);
    if (!archivo.is_open()) {
        std::cout << "Error al abrir el archivo para el reporte del Árbol B.\n";
        return;
    }

    archivo << "digraph BTree {\n";
    archivo << "  node [shape=record, fontname=\"Arial\", style=filled, fillcolor=\"#FFF9C4\"];\n"; // Amarillo pastel
    archivo << "  edge [color=\"#FBC02D\"];\n\n";

    if (root == nullptr) {
        archivo << "  vacio [label=\"Árbol B Vacío\"];\n";
        archivo << "}\n";
        archivo.close();
        return;
    }

    // Inicializamos Cola manual (Punteros de frente y final)
    ColaNodosB* frente = new ColaNodosB(root);
    ColaNodosB* final = frente;

    // Recorrido BFS (Nivel por Nivel)
    while (frente != nullptr) {
        // Extraemos el nodo del frente (Desencolar)
        BTreeNode* actual = frente->nodoArbol;

        ColaNodosB* temp = frente;
        frente = frente->siguiente;
        if (frente == nullptr) {
            final = nullptr;
        }
        delete temp; // Liberamos la memoria del nodo de la cola

        // Dibujar el nodo actual con sus divisiones
        archivo << "  node" << actual << " [label=\"";
        for (int i = 0; i < actual->n; i++) {
            archivo << "<c" << i << "> | " << actual->keys[i].date << " | ";
        }
        archivo << "<c" << actual->n << ">\"];\n";

        // Conectar hijos y encolarlos
        if (!actual->leaf) {
            for (int i = 0; i <= actual->n; i++) {
                if (actual->children[i] != nullptr) {

                    archivo << "  node" << actual << ":c" << i << " -> node" << actual->children[i] << ";\n";

                    ColaNodosB* nuevoNodoCola = new ColaNodosB(actual->children[i]);
                    if (final == nullptr) {
                        frente = final = nuevoNodoCola;
                    } else {
                        final->siguiente = nuevoNodoCola;
                        final = nuevoNodoCola;
                    }
                }
            }
        }
    }

    archivo << "}\n";
    archivo.close();
    std::cout << "Reporte Árbol B generado exitosamente en: " << nombreArchivo << "\n";
}