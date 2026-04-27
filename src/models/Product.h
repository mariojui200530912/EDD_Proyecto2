#ifndef PROYECTO2_PRODUCT_H
#define PROYECTO2_PRODUCT_H

#include <string>

struct Product {
    int sucursal_id;
    std::string name;
    std::string barcode;
    std::string category;
    std::string expiry_date;
    std::string brand;
    double price;
    int stock;
};


#endif //PROYECTO2_PRODUCT_H