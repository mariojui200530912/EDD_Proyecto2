#ifndef PROYECTO2_STACK_H
#define PROYECTO2_STACK_H

#include "../models/Product.h"
#include "LinkedList.h"
#include <iostream>

struct StackNode {
    Product data;
    StackNode* next;

    StackNode(Product p) : data(p), next(nullptr) {}
};

class Stack
{
private:
    StackNode* topNode;
    int size;

public:
    Stack();
    ~Stack();

    void apilar(const Product& producto); // Push
    bool desapilar(Product& productoRecuperado); // Pop

    Product* obtenerTope(); // Peek
    bool estaVacia() const;
    int obtenerTamano() const;
    void obtenerContenido(LinkedList& listaDestino);
    void vaciar();
};


#endif //PROYECTO2_STACK_H