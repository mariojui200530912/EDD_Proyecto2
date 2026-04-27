#ifndef PROYECTO2_AVLTREE_H
#define PROYECTO2_AVLTREE_H

#include "../models/Product.h"
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>

struct AVLNode
{
    Product data;
    AVLNode* left, *right;
    int height;

    AVLNode(Product p) : data(p), left(NULL), right(NULL), height(1) {}
};

class AVLTree
{
private:
    AVLNode* root;
    int obtenerAltura(AVLNode* node);
    int obtenerFactorEquilibrio(AVLNode* node);

    AVLNode* rotacionDerecha(AVLNode* y);
    AVLNode* rotacionIzquierda(AVLNode* x);

    AVLNode* insertarRecursivo(AVLNode* node, const Product& producto);
    AVLNode* buscarRecursivo(AVLNode* node, const std::string& nombre);
    AVLNode* nodoMinimo(AVLNode* node);
    AVLNode* eliminarRecursivo(AVLNode* node, const std::string& nombre, bool& eliminado);
    void listarInOrder(AVLNode* node);
    void destruirArbol(AVLNode* node);
    void generarReporteRecursivo(AVLNode* node, std::ofstream& nombreArchivo);

public:
    AVLTree();
    ~AVLTree();

    void insertar(const Product& producto);
    Product* buscar(const std::string& nombre);
    bool eliminar(const std::string& nombre);
    void listarAlfabeticamente();
    void generarReporte(const std::string& nombreArchivo);
};


#endif //PROYECTO2_AVLTREE_H