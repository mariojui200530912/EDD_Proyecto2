#ifndef PROYECTO2_QUEUE_H
#define PROYECTO2_QUEUE_H

#include "../models/Product.h"
#include <iostream>

struct QueueNode {
    Product data;
    QueueNode* next;

    QueueNode(Product p) : data(p), next(nullptr) {}
};

class Queue
{
private:
    QueueNode* frontNode; // Puntero al primer elemento
    QueueNode* rearNode;  // Puntero al ultimo elemento
    int size;

public:
    Queue();
    ~Queue();

    void encolar(const Product& producto);
    bool desencolar(Product& productoDesencolada);

    Product* obtenerFrente();
    bool estaVacia() const;
    int obtenerTamano() const;
    void imprimirCola(const std::string& nombreCola) const;
};


#endif //PROYECTO2_QUEUE_H