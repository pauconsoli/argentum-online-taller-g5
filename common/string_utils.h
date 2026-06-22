#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <algorithm>
#include <cctype>
#include <string>

// Convierte a minúsculas y elimina tildes del español (UTF-8).
// Ejemplo: "Báculo Engarzado" -> "baculo engarzado"
inline std::string normalize_name(const std::string& s) {
    std::string r;
    r.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        if (c == 0xC3 && i + 1 < s.size()) {
            unsigned char n = static_cast<unsigned char>(s[i + 1]);
            char base = '\0';
            switch (n) {
                case 0x80:
                case 0x81:
                    base = 'a';
                    break;  // À Á
                case 0x88:
                case 0x89:
                    base = 'e';
                    break;  // È É
                case 0x8C:
                case 0x8D:
                    base = 'i';
                    break;  // Ì Í
                case 0x92:
                case 0x93:
                    base = 'o';
                    break;  // Ò Ó
                case 0x99:
                case 0x9A:
                    base = 'u';
                    break;  // Ù Ú
                case 0x9C:
                    base = 'u';
                    break;  // Ü
                case 0x91:
                    base = 'n';
                    break;  // Ñ
                case 0xA0:
                case 0xA1:
                    base = 'a';
                    break;  // à á
                case 0xA8:
                case 0xA9:
                    base = 'e';
                    break;  // è é
                case 0xAC:
                case 0xAD:
                    base = 'i';
                    break;  // ì í
                case 0xB2:
                case 0xB3:
                    base = 'o';
                    break;  // ò ó
                case 0xB9:
                case 0xBA:
                    base = 'u';
                    break;  // ù ú
                case 0xBC:
                    base = 'u';
                    break;  // ü
                case 0xB1:
                    base = 'n';
                    break;  // ñ
                default:
                    break;
            }
            if (base != '\0') {
                r += base;
                ++i;
                continue;
            }
        }
        if (c < 0x80)
            r += static_cast<char>(std::tolower(c));
        else
            r += static_cast<char>(c);
    }
    return r;
}

#endif
