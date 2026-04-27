#include "Queue.h"

Queue::Queue() {
    frontNode = nullptr;
    rearNode = nullptr;
    size = 0;
}

Queue::~Queue() {
    QueueNode* current = frontNode;
    while (current != nullptr) {
        QueueNode* nextNode = current->next;
        delete current;
        current = nextNode;
    }
}

void Queue::encolar(const Product& producto) {
    QueueNode* nuevoNodo = new QueueNode(producto);

    if (estaVacia()) {
        frontNode = nuevoNodo;
        rearNode = nuevoNodo;
    } else {
        rearNode->next = nuevoNodo;
        rearNode = nuevoNodo;
    }
    size++;
}

bool Queue::desencolar(Product& productoDesencolada) {
    if (estaVacia()) {
        return false; // No hay nada que desencolar
    }

    QueueNode* nodoBorrar = frontNode;
    productoDesencolada = nodoBorrar->data;

    frontNode = frontNode->next;

    if (frontNode == nullptr) {
        rearNode = nullptr;
    }

    delete nodoBorrar;
    size--;
    return true;
}

Product* Queue::obtenerFrente() {
    if (estaVacia()) {
        return nullptr;
    }
    return &(frontNode->data);
}

bool Queue::estaVacia() const {
    return frontNode == nullptr;
}

int Queue::obtenerTamano() const {
    return size;
}

void Queue::imprimirCola(const std::string& nombreCola) const {
    std::cout << "--- " << nombreCola << " (Total: " << size << ") ---\n";
    QueueNode* current = frontNode;
    int posicion = 1;
    while (current != nullptr) {
        std::cout << posicion << ". [" << current->data.barcode << "] " << current->data.name << "\n";
        current = current->next;
        posicion++;
    }
    if (estaVacia()) std::cout << "(Vacía)\n";
}