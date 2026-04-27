#include "Stack.h"
Stack::Stack() {
    topNode = nullptr;
    size = 0;
}

Stack::~Stack() {
    vaciar();
}

void Stack::apilar(const Product& producto) {
    StackNode* nuevoNodo = new StackNode(producto);

    nuevoNodo->next = topNode;
    topNode = nuevoNodo;

    size++;
}

bool Stack::desapilar(Product& productoRecuperado) {
    if (estaVacia()) {
        return false;
    }

    StackNode* nodoABorrar = topNode;
    productoRecuperado = nodoABorrar->data;

    topNode = topNode->next;

    delete nodoABorrar;
    size--;

    return true;
}

Product* Stack::obtenerTope() {
    if (estaVacia()) {
        return nullptr;
    }
    return &(topNode->data);
}

bool Stack::estaVacia() const {
    return topNode == nullptr;
}

int Stack::obtenerTamano() const {
    return size;
}

void Stack::vaciar() {
    StackNode* current = topNode;
    while (current != nullptr) {
        StackNode* nextNode = current->next;
        delete current;
        current = nextNode;
    }
    topNode = nullptr;
    size = 0;
}