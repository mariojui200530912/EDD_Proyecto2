#include "DataValidator.h"
#include <cctype>

bool DataValidator::esNumeroValido(const std::string& str, bool admiteDecimal)
{
    if (str.empty()) return false;
    int puntosDecimales = 0;
    for (char c : str)
    {
        if (c == '.')
        {
            puntosDecimales++;
            if (!admiteDecimal || puntosDecimales > 1) return false;
        }else if (!std::isdigit(c))
        {
            return false;
        }
    }
    return true;
}

bool DataValidator::esFechaValida(const std::string& str)
{
    if (str.length() != 10) return false;
    if (str[4] != '-' || str[7] != '-') return false;
    for (int i = 0; i < 10; i++)
    {
        if (i == 4 || i == 7) continue;
        if (!std::isdigit(str[i])) return false;
    }
    return true;
}
