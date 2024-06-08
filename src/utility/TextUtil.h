#ifndef TEXTUTIL_H
#define TEXTUTIL_H

#include <locale>
#include <iostream>
#include <iomanip>

class comma_numpunct final : public std::numpunct<char>
{
protected:
    char do_thousands_sep() const override
    {
        return ',';
    }

    std::string do_grouping() const override
    {
        return "\03";
    }
};

class TextUtil {
public:
    template <typename T>
    static std::string getCommaString(const T& num);
};



#endif //TEXTUTIL_H
