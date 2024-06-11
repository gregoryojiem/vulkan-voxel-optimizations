#include "TextUtil.h"

template <typename T>
std::string TextUtil::getCommaString(const T& num) {
    std::stringstream ss;
    const std::locale comma_locale(std::locale(), new comma_numpunct());
    ss.imbue(comma_locale);
    ss << std::fixed << num;
    return ss.str();
}

template std::string TextUtil::getCommaString<unsigned int>(const unsigned int&);


