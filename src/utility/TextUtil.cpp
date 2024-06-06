#include "TextUtil.h"

std::string TextUtil::getCommaString(const int num) {
    std::stringstream ss;
    const std::locale comma_locale(std::locale(), new comma_numpunct());
    ss.imbue(comma_locale);
    ss << std::fixed << num;
    return ss.str();
}
