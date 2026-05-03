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

void BPlusTree::obtenerTodaLaLista(LinkedList& resultados) {
    if (root == nullptr) return;

    BPlusNode* actual = root;
    while (!actual->leaf) {
        actual = actual->C[0];
    }

    while (actual != nullptr) {
        for (int i = 0; i < actual->n; i++) {
            if (actual->keys[i].list != nullptr && !actual->keys[i].list->estaVacia()) {
                ListNode* prod = actual->keys[i].list->getInicio();
                while (prod != nullptr) {
                    resultados.insertarFinal(prod->data);
                    prod = prod->next;
                }
            }
        }
        actual = actual->next;
    }
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
                    temp->leaf = true;
                    temp->n = 0;
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

    der->leaf = true;
    der->n = 0;
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

    der->leaf = true;
    der->n = 0;
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
    if (!archivo.is_open()) return;

    archivo << "digraph BPlusTree {\n";
    archivo << "  rankdir=TB;\n";
    archivo << "  node [shape=plain, fontname=\"Arial\"];\n";
    archivo << "  edge [fontname=\"Arial\", fontsize=10];\n\n";

    if (root == nullptr) {
        archivo << "  vacio [label=\"Árbol B+ Vacío\", shape=ellipse];\n}\n";
        archivo.close();
        return;
    }

    ColaNodosBPlus* frente = new ColaNodosBPlus(root);
    ColaNodosBPlus* final = frente;

    while (frente != nullptr) {
        BPlusNode* actual = frente->nodoArbol;
        ColaNodosBPlus* temp = frente;
        frente = frente->siguiente;
        if (frente == nullptr) final = nullptr;
        delete temp;

        archivo << "  node" << actual << " [label=<\n";
        archivo << "    <TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\" BGCOLOR=\"#E3F2FD\">\n";
        archivo << "      <TR>\n";

        if (actual->leaf) {
            // --- NODO HOJA ---
            for (int i = 0; i < actual->n; i++) {
                std::string cat = actual->keys[i].category;
                for (char &c : cat) if (c == '<' || c == '>' || c == '&') c = ' ';

                archivo << "        <TD ALIGN=\"CENTER\" BGCOLOR=\"#BBDEFB\"><B>" << cat << "</B><BR/>";

                if (actual->keys[i].list && !actual->keys[i].list->estaVacia()) {
                    ListNode* prod = actual->keys[i].list->getInicio();
                    while (prod) {
                        std::string nom = prod->data.name;
                        for (char &c : nom) if (c == '<' || c == '>' || c == '&') c = ' ';
                        archivo << "<FONT POINT-SIZE=\"10\">- " << nom << "</FONT><BR/>";
                        prod = prod->next;
                    }
                }
                archivo << "</TD>\n";
            }
            archivo << "        <TD PORT=\"next\" BGCOLOR=\"#FFCDD2\"><I>Next</I></TD>\n";
        } else {
            // --- NODO INTERNO ---
            for (int i = 0; i <= actual->n; i++) {
                archivo << "        <TD PORT=\"c" << i << "\" BGCOLOR=\"#E3F2FD\"> </TD>\n";
                if (i < actual->n) {
                    archivo << "        <TD BGCOLOR=\"#BBDEFB\"><B>" << actual->keys[i].category << "</B></TD>\n";
                }
            }
        }

        archivo << "      </TR>\n";
        archivo << "    </TABLE>>];\n\n";

        // --- GENERAR CONEXIONES ---
        if (actual->leaf) {
            if (actual->next != nullptr) {




                archivo << "  node" << actual << ":next -> node" << actual->next << " [color=red, constraint=false, tailport=e, headport=w];\n";
            }
        } else {
            for (int i = 0; i <= actual->n; i++) {
                if (actual->C[i] != nullptr) {
                    archivo << "  node" << actual << ":c" << i << " -> node" << actual->C[i] << ";\n";
                    ColaNodosBPlus* nuevo = new ColaNodosBPlus(actual->C[i]);
                    if (final == nullptr) frente = final = nuevo;
                    else { final->siguiente = nuevo; final = nuevo; }
                }
            }
        }
    }
    archivo << "}\n";
    archivo.close();
}