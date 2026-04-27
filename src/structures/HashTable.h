#ifndef PROYECTO2_HASHTABLE_H
#define PROYECTO2_HASHTABLE_H

#include "../models/Product.h"
#include "LinkedList.h"
#include <fstream>
#include <iostream>

class HashTable
{
private:
    static const int TABLE_SIZE = 2003;
    LinkedList table[TABLE_SIZE];
    int hashFunction(const std::string& barcode);

public:
    HashTable();
    ~HashTable() = default;
    bool insertar(const Product& producto);
    Product* buscar(const std::string& barcode);
    bool eliminar(const std::string& barcode);
    int getCapacidad() const { return TABLE_SIZE; }
    LinkedList* getListaEnPosicion(int index) { return &table[index]; }
    void imprimirDistribucion();
    void generarReporte(const std::string& nombreArchivo);
};


#endif //PROYECTO2_HASHTABLE_H