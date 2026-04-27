#ifndef PROYECTO2_BTREEPLUS_H
#define PROYECTO2_BTREEPLUS_H

#include "../models/Product.h"
#include "LinkedList.h"
#include "Queue.h"
#include <fstream>
#include <iostream>
#include <string>

struct CategoryKey
{
    std::string category;
    LinkedList* list;

    CategoryKey() : category(""), list(nullptr) {};
};

class BPlusNode
{
    public:
    CategoryKey* keys;
    BPlusNode** C;
    int n;
    int d;
    bool leaf;

    BPlusNode* next;

    BPlusNode(int _d, bool _leaf);
    ~BPlusNode();
};

struct BPlusSplit {
    std::string promotedCategory;
    BPlusNode* rightNode;
    bool isSplit;
};

class BPlusTree
{
private:
    BPlusNode* root;
    int d;

    BPlusSplit insertarRecursivo(BPlusNode* node, const Product& k);
    void dividirHoja(BPlusNode* node, BPlusSplit& splitResult);
    void dividirInterno(BPlusNode* node, BPlusSplit& splitResult);
    void eliminarEstructural(BPlusNode* node, const std::string& categoria);
    void prestarDeHojaIzquierda(BPlusNode* padre, int idx);
    void prestarDeHojaDerecha(BPlusNode* padre, int idx);
    void fusionarHojas(BPlusNode* padre, int idx);
    void fusionarInternos(BPlusNode* padre, int idx);

public:
    BPlusTree(int _d);
    ~BPlusTree();

    void insertar(const Product& k);
    bool eliminarProducto(const Product& k);
    LinkedList* buscarCategoria(const std::string& categoria);
    void generarReporte(const std::string& nombreArchivo);
};


#endif //PROYECTO2_BTREEPLUS_H