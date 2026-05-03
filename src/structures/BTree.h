#ifndef PROYECTO2_BTREE_H
#define PROYECTO2_BTREE_H

#include "../models/Product.h"
#include "LinkedList.h"
#include <iostream>
#include <fstream>
#include <string>

struct DateKey
{
    std::string date;
    LinkedList* list;

    DateKey() : date(""), list(NULL) {}
};

class BTreeNode
{
    public:
    DateKey* keys;
    BTreeNode** children;
    int n;
    int d;
    bool leaf;

    BTreeNode(int _d, bool _leaf);
    ~BTreeNode();
    int encontrarClave(const std::string& date);
    void eliminarEstructural(const std::string& date);
    void removerDeHoja(int idx);
    void removerDeNoHoja(int idx);
    DateKey obtenerPredecesor(int idx);
    DateKey obtenerSucesor(int idx);
    void llenarNodo(int idx);
    void pedirPrestadoAnterior(int idx);
    void pedirPrestadoSiguiente(int idx);
    void fusionar(int idx);

    void buscarRangoRecursivo(const std::string& fechaInicio, const std::string& fechaFin, LinkedList& resultados);
};

struct BTreeSplit
{
    DateKey promotedKey;
    BTreeNode* rightNode;
    bool isSplit;
};

class BTree
{
    private:
    BTreeNode* root;
    int d;
    BTreeSplit insertarRecursivo(BTreeNode* node, const Product& producto);
    BTreeNode* buscarNodo(BTreeNode* node, const std::string& date);
    void listarPorFechaRecursivo(BTreeNode* node, LinkedList& resultados);
    void dividirNodo(BTreeNode* node, BTreeSplit& splitResult);

    public:
    BTree(int _d);
    ~BTree();
    void insertar(const Product& producto);
    bool eliminarProducto(const Product& k);
    void buscarPorRangoFechas(const std::string& fechaInicio, const std::string& fechaFin, LinkedList& resultados);
    void listarPorFecha(LinkedList& resultados);
    void generarReporte(const std::string& nombreArchivo);

};
#endif //PROYECTO2_BTREE_H