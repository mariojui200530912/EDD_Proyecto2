#ifndef PROYECTO2_LINKEDLIST_H
#define PROYECTO2_LINKEDLIST_H
#include "../models/Product.h"
#include <iostream>
#include <cstdlib>

struct ListNode
{
    Product data;
    ListNode *next;

    ListNode(Product p): data(p), next(nullptr) {}
};

class LinkedList
{
private:
    ListNode* head;
    ListNode* tail;
    int size;
public:
    LinkedList();
    ~LinkedList();
    void insertarFinal(const Product& p);
    void insertarInicio(const Product& p);
    Product* buscarPorCodigo(const std::string& barcode);
    Product* buscarPorNombre(const std::string& name);
    bool eliminarPorCodigo(const std::string& barcode);
    bool eliminarPorNombre(const std::string& name);

    bool estaVacia() const;
    ListNode* getInicio() const { return head; }
    int obtenerTamano() const;
    void ordenarPorNombreBurbuja();
    Product obtenerProductoAleatorio();
    void imprimirLista() const;
};


#endif //PROYECTO2_LINKEDLIST_H