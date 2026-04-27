#ifndef PROYECTO2_DATAVALIDATOR_H
#define PROYECTO2_DATAVALIDATOR_H

#include <string>

class DataValidator
{
    public:
    static bool esNumeroValido(const std::string& str, bool admiteDecimal);
    static bool esFechaValida(const std::string& str);
};


#endif //PROYECTO2_DATAVALIDATOR_H