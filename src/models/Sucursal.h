#ifndef PROYECTO2_SUCURSAL_H
#define PROYECTO2_SUCURSAL_H

#include "../structures/HashTable.h"
#include "../structures/AVLTree.h"
#include "../structures/BTree.h"
#include "../structures/BTreePlus.h"
#include "../structures/Stack.h"
#include <string>

struct Sucursal {
    int id;
    std::string nombre;
    std::string ubicacion;
    int t_ingreso;
    int t_traspaso;
    int t_despacho;

    HashTable inventarioHash;
    AVLTree inventarioAVL;
    BTree inventarioB;
    BPlusTree inventarioBPlus;
    LinkedList inventarioLista;

    // Pila devolucion
    Stack pilaRollback;

    Sucursal() : inventarioB(2), inventarioBPlus(2) {}
};


#endif //PROYECTO2_SUCURSAL_H