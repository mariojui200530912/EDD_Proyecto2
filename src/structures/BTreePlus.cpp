#include "BTreePlus.h"

BPlusNode::BPlusNode(int _d, bool _leaf) {
    d = _d;
    leaf = _leaf;
    n = 0;
    next = nullptr;

    keys = new CategoryKey[2 * d + 1];
    C = new BPlusNode*[2 * d + 2];

    for(int i = 0; i < 2 * d + 2; i++) {
        C[i] = nullptr;
    }
}

BPlusNode::~BPlusNode() {
    if (leaf) {
        for (int i = 0; i < n; i++) {
            delete keys[i].list;
        }
    }
    delete[] keys;

    if (!leaf) {
        for (int i = 0; i <= n; i++) {
            delete C[i];
        }
    }
    delete[] C;
}

// Implementacion ARBOL B+
BPlusTree::BPlusTree(int _d) {
    root = nullptr;
    d = _d;
}

BPlusTree::~BPlusTree() {
    delete root;
}

LinkedList* BPlusTree::buscarCategoria(const std::string& categoria) {
    if (root == nullptr) return nullptr;

    BPlusNode* current = root;

    while (!current->leaf) {
        int i = 0;
        while (i < current->n && categoria >= current->keys[i].category) {
            i++;
        }
        current = current->C[i];
    }

    for (int i = 0; i < current->n; i++) {
        if (current->keys[i].category == categoria) {
            return current->keys[i].list;
        }
    }

    return nullptr; // No existe la categoría
}

void BPlusTree::insertar(const Product& k) {
    if (root == nullptr) {
        root = new BPlusNode(d, true);
        root->keys[0].category = k.category;
        root->keys[0].list = new LinkedList();
        root->keys[0].list->insertarFinal(k);
        root->n = 1;
        return;
    }

    BPlusSplit resultado = insertarRecursivo(root, k);

    if (resultado.isSplit) {
        BPlusNode* nuevaRaiz = new BPlusNode(d, false);
        nuevaRaiz->keys[0].category = resultado.promotedCategory;
        nuevaRaiz->C[0] = root;
        nuevaRaiz->C[1] = resultado.rightNode;
        nuevaRaiz->n = 1;
        root = nuevaRaiz;
    }
}

BPlusSplit BPlusTree::insertarRecursivo(BPlusNode* node, const Product& k) {
    BPlusSplit resultado = {"", nullptr, false};

    if (node->leaf) {
        for (int j = 0; j < node->n; j++) {
            if (node->keys[j].category == k.category) {
                node->keys[j].list->insertarFinal(k);
                return resultado;
            }
        }

        int i = node->n - 1;
        while (i >= 0 && node->keys[i].category > k.category) {
            node->keys[i + 1] = node->keys[i];
            i--;
        }
        node->keys[i + 1].category = k.category;
        node->keys[i + 1].list = new LinkedList();
        node->keys[i + 1].list->insertarFinal(k);
        node->n++;

        if (node->n > 2 * d) dividirHoja(node, resultado);

    } else {
        int i = node->n - 1;
        while (i >= 0 && node->keys[i].category > k.category) i--;
        i++;

        BPlusSplit hijoResultado = insertarRecursivo(node->C[i], k);

        if (hijoResultado.isSplit) {
            int j = node->n - 1;
            while (j >= i) {
                node->keys[j + 1] = node->keys[j];
                node->C[j + 2] = node->C[j + 1];
                j--;
            }
            node->keys[i].category = hijoResultado.promotedCategory;
            node->C[i + 1] = hijoResultado.rightNode;
            node->n++;

            if (node->n > 2 * d) dividirInterno(node, resultado);
        }
    }
    return resultado;
}

void BPlusTree::dividirHoja(BPlusNode* node, BPlusSplit& splitResult) {
    int medianaIndex = d;
    BPlusNode* nuevoNodo = new BPlusNode(d, true);

    nuevoNodo->n = d + 1;
    for (int j = 0; j <= d; j++) {
        nuevoNodo->keys[j] = node->keys[medianaIndex + j];
    }

    node->n = d;

    nuevoNodo->next = node->next;
    node->next = nuevoNodo;

    splitResult.promotedCategory = nuevoNodo->keys[0].category;
    splitResult.rightNode = nuevoNodo;
    splitResult.isSplit = true;
}

void BPlusTree::dividirInterno(BPlusNode* node, BPlusSplit& splitResult) {
    int medianaIndex = d;
    BPlusNode* nuevoNodo = new BPlusNode(d, false);
    nuevoNodo->n = d;

    for (int j = 0; j < d; j++) nuevoNodo->keys[j] = node->keys[medianaIndex + 1 + j];
    for (int j = 0; j <= d; j++) nuevoNodo->C[j] = node->C[medianaIndex + 1 + j];

    node->n = d;

    splitResult.promotedCategory = node->keys[medianaIndex].category;
    splitResult.rightNode = nuevoNodo;
    splitResult.isSplit = true;
}

bool BPlusTree::eliminarProducto(const Product& k) {
    if (root == nullptr) return false;

    BPlusNode* current = root;
    while (!current->leaf) {
        int i = 0;
        while (i < current->n && k.category >= current->keys[i].category) i++;
        current = current->C[i];
    }

    for (int i = 0; i < current->n; i++) {
        if (current->keys[i].category == k.category) {
            bool borrado = current->keys[i].list->eliminarPorCodigo(k.barcode);

            if (borrado && current->keys[i].list->estaVacia()) {
                delete current->keys[i].list;
                current->keys[i].list = nullptr;

                eliminarEstructural(root, k.category);

                if (root != nullptr && root->n == 0) {
                    BPlusNode* temp = root;
                    if (root->leaf) {
                        root = nullptr;
                    } else {
                        root = root->C[0];
                    }
                    delete temp;
                }
            }
            return true;
        }
    }
    return false;
}

void BPlusTree::eliminarEstructural(BPlusNode* node, const std::string& categoria) {
    int idx = 0;
    while (idx < node->n && categoria > node->keys[idx].category) idx++;

    if (node->leaf) {
        if (idx < node->n && node->keys[idx].category == categoria) {
            for (int i = idx + 1; i < node->n; ++i) {
                node->keys[i - 1] = node->keys[i];
            }
            node->n--;
        }
        return;
    }

    BPlusNode* hijo = node->C[idx];
    eliminarEstructural(hijo, categoria);

    if (hijo->n < d) {
        if (idx > 0 && node->C[idx - 1]->n > d) {
            if (hijo->leaf) prestarDeHojaIzquierda(node, idx);
            else
                 ;
        } else if (idx < node->n && node->C[idx + 1]->n > d) {
            if (hijo->leaf) prestarDeHojaDerecha(node, idx);
        } else {
            if (idx < node->n) {
                if (hijo->leaf) fusionarHojas(node, idx);
                else fusionarInternos(node, idx);
            } else {
                if (hijo->leaf) fusionarHojas(node, idx - 1);
                else fusionarInternos(node, idx - 1);
            }
        }
    }
}

void BPlusTree::prestarDeHojaIzquierda(BPlusNode* padre, int idx) {
    BPlusNode* hijo = padre->C[idx];
    BPlusNode* hermano = padre->C[idx - 1];

    for (int i = hijo->n - 1; i >= 0; --i) hijo->keys[i + 1] = hijo->keys[i];

    hijo->keys[0] = hermano->keys[hermano->n - 1];

    hijo->n++;
    hermano->n--;

    padre->keys[idx - 1].category = hijo->keys[0].category;
}

void BPlusTree::prestarDeHojaDerecha(BPlusNode* padre, int idx) {
    BPlusNode* hijo = padre->C[idx];
    BPlusNode* hermano = padre->C[idx + 1];

    hijo->keys[hijo->n] = hermano->keys[0];

    for (int i = 1; i < hermano->n; ++i) hermano->keys[i - 1] = hermano->keys[i];

    hijo->n++;
    hermano->n--;

    padre->keys[idx].category = hermano->keys[0].category;
}

void BPlusTree::fusionarHojas(BPlusNode* padre, int idx) {
    BPlusNode* izq = padre->C[idx];
    BPlusNode* der = padre->C[idx + 1];

    for (int i = 0; i < der->n; ++i) {
        izq->keys[izq->n + i] = der->keys[i];
    }
    izq->n += der->n;

    izq->next = der->next;

    for (int i = idx + 1; i < padre->n; ++i) padre->keys[i - 1] = padre->keys[i];
    for (int i = idx + 2; i <= padre->n; ++i) padre->C[i - 1] = padre->C[i];
    padre->n--;

    delete der;
}

void BPlusTree::fusionarInternos(BPlusNode* padre, int idx) {
    BPlusNode* izq = padre->C[idx];
    BPlusNode* der = padre->C[idx + 1];

    izq->keys[izq->n] = padre->keys[idx];

    for (int i = 0; i < der->n; ++i) izq->keys[izq->n + 1 + i] = der->keys[i];
    for (int i = 0; i <= der->n; ++i) izq->C[izq->n + 1 + i] = der->C[i];
    izq->n += der->n + 1;

    for (int i = idx + 1; i < padre->n; ++i) padre->keys[i - 1] = padre->keys[i];
    for (int i = idx + 2; i <= padre->n; ++i) padre->C[i - 1] = padre->C[i];
    padre->n--;

    delete der;
}

// estructura auxiliar para generar grafico
struct ColaNodosBPlus {
    BPlusNode* nodoArbol;
    ColaNodosBPlus* siguiente;
    ColaNodosBPlus(BPlusNode* n) : nodoArbol(n), siguiente(nullptr) {}
};

void BPlusTree::generarReporte(const std::string& nombreArchivo) {
    std::ofstream archivo(nombreArchivo);
    if (!archivo.is_open()) {
        std::cout << "Error al abrir el archivo para el reporte del Árbol B+.\n";
        return;
    }

    archivo << "digraph BPlusTree {\n";
    archivo << "  node [shape=record, fontname=\"Arial\", style=filled, fillcolor=\"#E3F2FD\"];\n"; // Azul pastel
    archivo << "  edge [color=\"#1565C0\"];\n\n";

    if (root == nullptr) {
        archivo << "  vacio [label=\"Árbol B+ Vacío\"];\n";
        archivo << "}\n";
        archivo.close();
        return;
    }

    // Inicializamos Cola manual (BFS)
    ColaNodosBPlus* frente = new ColaNodosBPlus(root);
    ColaNodosBPlus* final = frente;

    // Recorrido Nivel por Nivel
    while (frente != nullptr) {
        // Desencolar
        BPlusNode* actual = frente->nodoArbol;

        ColaNodosBPlus* temp = frente;
        frente = frente->siguiente;
        if (frente == nullptr) final = nullptr;
        delete temp;

        archivo << "  node" << actual << " [label=\"";

        if (actual->leaf) {
            // NODO HOJA: Mostramos solo categorías y un puerto <next>
            for (int i = 0; i < actual->n; i++) {
                archivo << " " << actual->keys[i].category << " |";
            }
            archivo << "<next> \"];\n"; // El puerto para la lista enlazada

            // DIBUJAR LA FLECHA DE LA LISTA ENLAZADA
            if (actual->next != nullptr) {
                // constraint=false es vital para que Graphviz dibuje esto horizontalmente
                archivo << "  node" << actual << ":next -> node" << actual->next
                        << " [color=\"#D32F2F\", style=\"dashed\", constraint=false, label=\" next\"];\n";
            }
        } else {
            // NODO INTERNO: Puertos <c_i> para los hijos y categorías como separadores
            for (int i = 0; i < actual->n; i++) {
                archivo << "<c" << i << "> | " << actual->keys[i].category << " | ";
            }
            archivo << "<c" << actual->n << ">\"];\n";

            // Encolar y conectar hijos
            for (int i = 0; i <= actual->n; i++) {
                if (actual->C[i] != nullptr) {
                    // Conectar puerto <c_i> con el nodo hijo correspondiente
                    archivo << "  node" << actual << ":c" << i << " -> node" << actual->C[i] << ";\n";

                    // Encolar hijo
                    ColaNodosBPlus* nuevoNodoCola = new ColaNodosBPlus(actual->C[i]);
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
    std::cout << "Reporte Árbol B+ generado exitosamente en: " << nombreArchivo << "\n";
}