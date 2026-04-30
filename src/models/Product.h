#ifndef PROYECTO2_PRODUCT_H
#define PROYECTO2_PRODUCT_H

#include <string>

struct Product {
    std::string name;
    std::string barcode;
    std::string category;
    std::string expiry_date;
    std::string brand;
    double price;
    int stock;

    std::string estado;

    Product() : price(0.0), stock(0), estado("Disponible") {}
};


#endif //PROYECTO2_PRODUCT_H