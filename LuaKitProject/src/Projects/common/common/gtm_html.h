#include <iostream>

namespace gtm {

    // Remove html tag , decode html entity
    extern void Html2Text(const std::string& src, std::string& data, bool newline = false);
    // Replace newlines with <br /> tags.
    extern void Text2Html(const std::string& src, std::string& data);
    // Encode html entity
    extern void EncodeHtml(const std::string& src, std::string& data, bool unicode = false);
    // Decode html entity
    extern void DecodeHtml(const std::string& src, std::string& data);

}
