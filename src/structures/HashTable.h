#ifndef PROYECTO2_HASHTABLE_H
#define PROYECTO2_HASHTABLE_H

#include "../models/Product.h"
#include "LinkedList.h"
#include <fstream>
#include <iostream>

class HashTable
{
private:
    int capacidad;
    int numElementos;
    LinkedList** tabla;

    bool esPrimo(int n);
    int siguientePrimo(int n);
    void rehash(int nuevaCapacidad);

public:
    HashTable(int tamano = 13);
    ~HashTable();
    void asegurarCapacidad(int elementosNuevos);
    int funcionHash(const std::string& barcode);
    bool insertar(const Product& producto, int cantidadIngresar = 1);
    Product* buscar(const std::string& barcode);
    bool eliminar(const std::string& barcode);
    int getCapacidad() const { return capacidad; }
    LinkedList* getListaEnPosicion(int index) { return tabla[index]; }
    void imprimirDistribucion();
    void generarReporte(const std::string& nombreArchivo);
};


#endif //PROYECTO2_HASHTABLE_H