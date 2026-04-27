#include "AVLTree.h"

AVLTree::AVLTree() : root(nullptr) {}

AVLTree::~AVLTree()
{
    destruirArbol(root);
}

void AVLTree::destruirArbol(AVLNode* node) {
    if (node != nullptr) {
        destruirArbol(node->left);
        destruirArbol(node->right);
        delete node;
    }
}

int AVLTree::obtenerAltura(AVLNode* node)
{
    if (node == nullptr) return 0;
    return node->height;
}

int AVLTree::obtenerFactorEquilibrio(AVLNode* node)
{
    if (node == nullptr) return 0;
    return obtenerAltura(node->left) - obtenerAltura(node->right);
}

// Rotacion Derecha
AVLNode* AVLTree::rotacionDerecha(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;

    x->right = y;
    y->left = T2;

    y->height = std::max(obtenerAltura(y->left), obtenerAltura(y->right)) + 1;
    x->height = std::max(obtenerAltura(x->left), obtenerAltura(x->right)) + 1;

    return x;
}

// Rotacion Izquierda
AVLNode* AVLTree::rotacionIzquierda(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;

    y->left = x;
    x->right = T2;

    x->height = std::max(obtenerAltura(x->left), obtenerAltura(x->right)) + 1;
    y->height = std::max(obtenerAltura(y->left), obtenerAltura(y->right)) + 1;

    return y;
}

AVLNode* AVLTree::insertarRecursivo(AVLNode* node, const Product& producto) {
    if (node == nullptr) {
        return new AVLNode(producto);
    }

    if (producto.name < node->data.name) {
        node->left = insertarRecursivo(node->left, producto);
    } else if (producto.name > node->data.name) {
        node->right = insertarRecursivo(node->right, producto);
    } else {
        // si nombre esta duplicado lo enviamos a la derecha
        node->right = insertarRecursivo(node->right, producto);
    }

    node->height = 1 + std::max(obtenerAltura(node->left), obtenerAltura(node->right));

    int balance = obtenerFactorEquilibrio(node);

    // Nodo desbalanceado
    // Caso Izquierda-Izquierda (LL)
    if (balance > 1 && producto.name < node->left->data.name) {
        return rotacionDerecha(node);
    }
    // Caso Derecha-Derecha (RR)
    if (balance < -1 && producto.name > node->right->data.name) {
        return rotacionIzquierda(node);
    }
    // Caso Izquierda-Derecha (LR)
    if (balance > 1 && producto.name > node->left->data.name) {
        node->left = rotacionIzquierda(node->left);
        return rotacionDerecha(node);
    }
    // Caso Derecha-Izquierda (RL)
    if (balance < -1 && producto.name < node->right->data.name) {
        node->right = rotacionDerecha(node->right);
        return rotacionIzquierda(node);
    }

    return node;
}

void AVLTree::insertar(const Product& producto) {
    root = insertarRecursivo(root, producto);
}

AVLNode* AVLTree::buscarRecursivo(AVLNode* node, const std::string& nombre) {
    if (node == nullptr || node->data.name == nombre) {
        return node;
    }

    if (nombre < node->data.name) {
        return buscarRecursivo(node->left, nombre);
    }
    return buscarRecursivo(node->right, nombre);
}

Product* AVLTree::buscar(const std::string& nombre) {
    AVLNode* resultado = buscarRecursivo(root, nombre);
    if (resultado != nullptr) {
        return &(resultado->data);
    }
    return nullptr;
}

void AVLTree::listarInOrder(AVLNode* node) {
    if (node != nullptr) {
        listarInOrder(node->left);
        std::cout << "- " << node->data.name << " (Stock: " << node->data.stock << ")\n";
        listarInOrder(node->right);
    }
}

void AVLTree::listarAlfabeticamente() {
    std::cout << "\n--- Catálogo Alfabético (Generado por AVL) ---\n";
    listarInOrder(root);
}

AVLNode* AVLTree::nodoMinimo(AVLNode* node) {
    AVLNode* current = node;
    // Bajamos hacia la izquierda
    while (current->left != nullptr) {
        current = current->left;
    }
    return current;
}

bool AVLTree::eliminar(const std::string& nombre) {
    bool eliminado = false;
    root = eliminarRecursivo(root, nombre, eliminado);
    return eliminado;
}

AVLNode* AVLTree::eliminarRecursivo(AVLNode* node, const std::string& nombre, bool& eliminado) {
    // Busqueda normal
    if (node == nullptr) {
        return node;
    }

    if (nombre < node->data.name) {
        node->left = eliminarRecursivo(node->left, nombre, eliminado);
    } else if (nombre > node->data.name) {
        node->right = eliminarRecursivo(node->right, nombre, eliminado);
    } else {
        eliminado = true;

        // Caso A y B: El nodo tiene 0 o 1 hijo
        if ((node->left == nullptr) || (node->right == nullptr)) {
            AVLNode* temp = node->left ? node->left : node->right;

            if (temp == nullptr) {
                // Caso A: No tiene hijos (Es una hoja)
                temp = node;
                node = nullptr;
            } else {
                // Caso B: Tiene un hijo
                *node = *temp;
            }
            delete temp;
        } else {
            // Caso C: El nodo tiene 2 hijos
            // Buscamos el sucesor inorder (el más pequeño del subárbol derecho)
            AVLNode* temp = nodoMinimo(node->right);

            node->data = temp->data;

            node->right = eliminarRecursivo(node->right, temp->data.name, eliminado);
        }
    }

    if (node == nullptr) {
        return node;
    }

    // REBALANCEO

    // Actualizamos la altura del nodo actual
    node->height = 1 + std::max(obtenerAltura(node->left), obtenerAltura(node->right));

    // Obtenemos el factor de equilibrio
    int balance = obtenerFactorEquilibrio(node);

    // Caso Izquierda-Izquierda (LL)
    if (balance > 1 && obtenerFactorEquilibrio(node->left) >= 0) {
        return rotacionDerecha(node);
    }

    // Caso Izquierda-Derecha (LR)
    if (balance > 1 && obtenerFactorEquilibrio(node->left) < 0) {
        node->left = rotacionIzquierda(node->left);
        return rotacionDerecha(node);
    }

    // Caso Derecha-Derecha (RR)
    if (balance < -1 && obtenerFactorEquilibrio(node->right) <= 0) {
        return rotacionIzquierda(node);
    }

    // Caso Derecha-Izquierda (RL)
    if (balance < -1 && obtenerFactorEquilibrio(node->right) > 0) {
        node->right = rotacionDerecha(node->right);
        return rotacionIzquierda(node);
    }

    // Si no hubo desbalanceo, retornamos el nodo intacto
    return node;
}

void AVLTree::generarReporte(const std::string& nombreArchivo) {
    std::ofstream archivo(nombreArchivo);
    if (!archivo.is_open()) {
        std::cout << "Error al crear el archivo del reporte AVL.\n";
        return;
    }

    archivo << "digraph AVLTree {\n";
    archivo << "  node [shape=record, style=filled, fillcolor=\"#E8F5E9\", fontname=\"Arial\"];\n";
    archivo << "  edge [color=\"#2E7D32\"];\n\n";

    if (root != nullptr) {
        generarReporteRecursivo(root, archivo);
    } else {
        archivo << "  vacio [label=\"Árbol AVL Vacío\"];\n";
    }

    archivo << "}\n";
    archivo.close();
    std::cout << "Reporte AVL generado exitosamente en: " << nombreArchivo << "\n";
}

void AVLTree::generarReporteRecursivo(AVLNode* node, std::ofstream& archivo) {
    if (node == nullptr) return;

    int bf = obtenerFactorEquilibrio(node);

    archivo << "  node" << node << " [label=\"<f0> |<f1> " << node->data.name
            << "\\nBF: " << bf << " |<f2> \"];\n";

    if (node->left != nullptr) {
        archivo << "  node" << node << ":f0 -> node" << node->left << ":f1;\n";
        generarReporteRecursivo(node->left, archivo);
    }

    if (node->right != nullptr) {
        archivo << "  node" << node << ":f2 -> node" << node->right << ":f1;\n";
        generarReporteRecursivo(node->right, archivo);
    }
}