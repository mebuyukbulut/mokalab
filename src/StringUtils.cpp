#include "StringUtils.h"
#include <sstream>

std::vector<std::string> StringUtils::split(const std::string &str, const char& delimiter)
{
    std::vector<std::string> sonuc;
    std::stringstream ss(str);
    std::string parca;
    while (std::getline(ss, parca, delimiter)) {
        sonuc.push_back(parca);
    }
    return sonuc;
}