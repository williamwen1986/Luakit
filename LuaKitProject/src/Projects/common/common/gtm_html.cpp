#include "gtm_html.h"
#include <sstream>
#include <algorithm>

namespace gtm {

typedef struct {
    std::string escapeSequence;
    unsigned short uchar;
} HTMLEscapeMap;

// Taken from http://www.w3.org/TR/xhtml1/dtds.html#a_dtd_Special_characters
// Ordered by uchar lowest to highest for bsearching
static HTMLEscapeMap gAsciiHTMLEscapeMap[] = {
    // A.2.2. Special characters
    { "&quot;", 34 },
    { "&amp;", 38 },
    //	{ "&apos;", 39 },
    { "&lt;", 60 },
    { "&gt;", 62 },
    
    // A.2.1. Latin-1 characters
    { "&nbsp;", 160 },
    { "&iexcl;", 161 },
    { "&cent;", 162 },
    { "&pound;", 163 },
    { "&curren;", 164 },
    { "&yen;", 165 },
    { "&brvbar;", 166 },
    { "&sect;", 167 },
    { "&uml;", 168 },
    { "&copy;", 169 },
    { "&ordf;", 170 },
    { "&laquo;", 171 },
    { "&not;", 172 },
    { "&shy;", 173 },
    { "&reg;", 174 },
    { "&macr;", 175 },
    { "&deg;", 176 },
    { "&plusmn;", 177 },
    { "&sup2;", 178 },
    { "&sup3;", 179 },
    { "&acute;", 180 },
    { "&micro;", 181 },
    { "&para;", 182 },
    { "&middot;", 183 },
    { "&cedil;", 184 },
    { "&sup1;", 185 },
    { "&ordm;", 186 },
    { "&raquo;", 187 },
    { "&frac14;", 188 },
    { "&frac12;", 189 },
    { "&frac34;", 190 },
    { "&iquest;", 191 },
    { "&Agrave;", 192 },
    { "&Aacute;", 193 },
    { "&Acirc;", 194 },
    { "&Atilde;", 195 },
    { "&Auml;", 196 },
    { "&Aring;", 197 },
    { "&AElig;", 198 },
    { "&Ccedil;", 199 },
    { "&Egrave;", 200 },
    { "&Eacute;", 201 },
    { "&Ecirc;", 202 },
    { "&Euml;", 203 },
    { "&Igrave;", 204 },
    { "&Iacute;", 205 },
    { "&Icirc;", 206 },
    { "&Iuml;", 207 },
    { "&ETH;", 208 },
    { "&Ntilde;", 209 },
    { "&Ograve;", 210 },
    { "&Oacute;", 211 },
    { "&Ocirc;", 212 },
    { "&Otilde;", 213 },
    { "&Ouml;", 214 },
    { "&times;", 215 },
    { "&Oslash;", 216 },
    { "&Ugrave;", 217 },
    { "&Uacute;", 218 },
    { "&Ucirc;", 219 },
    { "&Uuml;", 220 },
    { "&Yacute;", 221 },
    { "&THORN;", 222 },
    { "&szlig;", 223 },
    { "&agrave;", 224 },
    { "&aacute;", 225 },
    { "&acirc;", 226 },
    { "&atilde;", 227 },
    { "&auml;", 228 },
    { "&aring;", 229 },
    { "&aelig;", 230 },
    { "&ccedil;", 231 },
    { "&egrave;", 232 },
    { "&eacute;", 233 },
    { "&ecirc;", 234 },
    { "&euml;", 235 },
    { "&igrave;", 236 },
    { "&iacute;", 237 },
    { "&icirc;", 238 },
    { "&iuml;", 239 },
    { "&eth;", 240 },
    { "&ntilde;", 241 },
    { "&ograve;", 242 },
    { "&oacute;", 243 },
    { "&ocirc;", 244 },
    { "&otilde;", 245 },
    { "&ouml;", 246 },
    { "&divide;", 247 },
    { "&oslash;", 248 },
    { "&ugrave;", 249 },
    { "&uacute;", 250 },
    { "&ucirc;", 251 },
    { "&uuml;", 252 },
    { "&yacute;", 253 },
    { "&thorn;", 254 },
    { "&yuml;", 255 },
    
    // A.2.2. Special characters cont'd
    { "&OElig;", 338 },
    { "&oelig;", 339 },
    { "&Scaron;", 352 },
    { "&scaron;", 353 },
    { "&Yuml;", 376 },
    
    // A.2.3. Symbols
    { "&fnof;", 402 },
    
    // A.2.2. Special characters cont'd
    { "&circ;", 710 },
    { "&tilde;", 732 },
    
    // A.2.3. Symbols cont'd
    { "&Alpha;", 913 },
    { "&Beta;", 914 },
    { "&Gamma;", 915 },
    { "&Delta;", 916 },
    { "&Epsilon;", 917 },
    { "&Zeta;", 918 },
    { "&Eta;", 919 },
    { "&Theta;", 920 },
    { "&Iota;", 921 },
    { "&Kappa;", 922 },
    { "&Lambda;", 923 },
    { "&Mu;", 924 },
    { "&Nu;", 925 },
    { "&Xi;", 926 },
    { "&Omicron;", 927 },
    { "&Pi;", 928 },
    { "&Rho;", 929 },
    { "&Sigma;", 931 },
    { "&Tau;", 932 },
    { "&Upsilon;", 933 },
    { "&Phi;", 934 },
    { "&Chi;", 935 },
    { "&Psi;", 936 },
    { "&Omega;", 937 },
    { "&alpha;", 945 },
    { "&beta;", 946 },
    { "&gamma;", 947 },
    { "&delta;", 948 },
    { "&epsilon;", 949 },
    { "&zeta;", 950 },
    { "&eta;", 951 },
    { "&theta;", 952 },
    { "&iota;", 953 },
    { "&kappa;", 954 },
    { "&lambda;", 955 },
    { "&mu;", 956 },
    { "&nu;", 957 },
    { "&xi;", 958 },
    { "&omicron;", 959 },
    { "&pi;", 960 },
    { "&rho;", 961 },
    { "&sigmaf;", 962 },
    { "&sigma;", 963 },
    { "&tau;", 964 },
    { "&upsilon;", 965 },
    { "&phi;", 966 },
    { "&chi;", 967 },
    { "&psi;", 968 },
    { "&omega;", 969 },
    { "&thetasym;", 977 },
    { "&upsih;", 978 },
    { "&piv;", 982 },
    
    // A.2.2. Special characters cont'd
    { "&ensp;", 8194 },
    { "&emsp;", 8195 },
    { "&thinsp;", 8201 },
    { "&zwnj;", 8204 },
    { "&zwj;", 8205 },
    { "&lrm;", 8206 },
    { "&rlm;", 8207 },
    { "&ndash;", 8211 },
    { "&mdash;", 8212 },
    { "&lsquo;", 8216 },
    { "&rsquo;", 8217 },
    { "&sbquo;", 8218 },
    { "&ldquo;", 8220 },
    { "&rdquo;", 8221 },
    { "&bdquo;", 8222 },
    { "&dagger;", 8224 },
    { "&Dagger;", 8225 },
    // A.2.3. Symbols cont'd
    { "&bull;", 8226 },
    { "&hellip;", 8230 },
    
    // A.2.2. Special characters cont'd
    { "&permil;", 8240 },
    
    // A.2.3. Symbols cont'd
    { "&prime;", 8242 },
    { "&Prime;", 8243 },
    
    // A.2.2. Special characters cont'd
    { "&lsaquo;", 8249 },
    { "&rsaquo;", 8250 },
    
    // A.2.3. Symbols cont'd
    { "&oline;", 8254 },
    { "&frasl;", 8260 },
    
    // A.2.2. Special characters cont'd
    { "&euro;", 8364 },
    
    // A.2.3. Symbols cont'd
    { "&image;", 8465 },
    { "&weierp;", 8472 },
    { "&real;", 8476 },
    { "&trade;", 8482 },
    { "&alefsym;", 8501 },
    { "&larr;", 8592 },
    { "&uarr;", 8593 },
    { "&rarr;", 8594 },
    { "&darr;", 8595 },
    { "&harr;", 8596 },
    { "&crarr;", 8629 },
    { "&lArr;", 8656 },
    { "&uArr;", 8657 },
    { "&rArr;", 8658 },
    { "&dArr;", 8659 },
    { "&hArr;", 8660 },
    { "&forall;", 8704 },
    { "&part;", 8706 },
    { "&exist;", 8707 },
    { "&empty;", 8709 },
    { "&nabla;", 8711 },
    { "&isin;", 8712 },
    { "&notin;", 8713 },
    { "&ni;", 8715 },
    { "&prod;", 8719 },
    { "&sum;", 8721 },
    { "&minus;", 8722 },
    { "&lowast;", 8727 },
    { "&radic;", 8730 },
    { "&prop;", 8733 },
    { "&infin;", 8734 },
    { "&ang;", 8736 },
    { "&and;", 8743 },
    { "&or;", 8744 },
    { "&cap;", 8745 },
    { "&cup;", 8746 },
    { "&int;", 8747 },
    { "&there4;", 8756 },
    { "&sim;", 8764 },
    { "&cong;", 8773 },
    { "&asymp;", 8776 },
    { "&ne;", 8800 },
    { "&equiv;", 8801 },
    { "&le;", 8804 },
    { "&ge;", 8805 },
    { "&sub;", 8834 },
    { "&sup;", 8835 },
    { "&nsub;", 8836 },
    { "&sube;", 8838 },
    { "&supe;", 8839 },
    { "&oplus;", 8853 },
    { "&otimes;", 8855 },
    { "&perp;", 8869 },
    { "&sdot;", 8901 },
    { "&lceil;", 8968 },
    { "&rceil;", 8969 },
    { "&lfloor;", 8970 },
    { "&rfloor;", 8971 },
    { "&lang;", 9001 },
    { "&rang;", 9002 },
    { "&loz;", 9674 },
    { "&spades;", 9824 },
    { "&clubs;", 9827 },
    { "&hearts;", 9829 },
    { "&diams;", 9830 }
};

// Taken from http://www.w3.org/TR/xhtml1/dtds.html#a_dtd_Special_characters
// This is table A.2.2 Special Characters
static HTMLEscapeMap gUnicodeHTMLEscapeMap[] = {
    
    // C0 Controls and Basic Latin
    { "&quot;", 34 },
    { "&amp;", 38 },
    //{ "&apos;", 39 },
    { "&lt;", 60 },
    { "&gt;", 62 },
    
    // Latin Extended-A
    { "&OElig;", 338 },
    { "&oelig;", 339 },
    { "&Scaron;", 352 },
    { "&scaron;", 353 },
    { "&Yuml;", 376 },
    
    // Spacing Modifier Letters
    { "&circ;", 710 },
    { "&tilde;", 732 },
    
    // General Punctuation
    { "&ensp;", 8194 },
    { "&emsp;", 8195 },
    { "&thinsp;", 8201 },
    { "&zwnj;", 8204 },
    { "&zwj;", 8205 },
    { "&lrm;", 8206 },
    { "&rlm;", 8207 },
    { "&ndash;", 8211 },
    { "&mdash;", 8212 },
    { "&lsquo;", 8216 },
    { "&rsquo;", 8217 },
    { "&sbquo;", 8218 },
    { "&ldquo;", 8220 },
    { "&rdquo;", 8221 },
    { "&bdquo;", 8222 },
    { "&dagger;", 8224 },
    { "&Dagger;", 8225 },
    { "&permil;", 8240 },
    { "&lsaquo;", 8249 },
    { "&rsaquo;", 8250 },
    { "&euro;", 8364 },
};


// Utility function for Bsearching table above
static int EscapeMapCompare(const void *ucharVoid, const void *mapVoid) {
    const unsigned char *uchar = (const unsigned char*)ucharVoid;
    const HTMLEscapeMap *map = (const HTMLEscapeMap*)mapVoid;
    int val;
    if (*uchar > map->uchar) {
        val = 1;
    } else if (*uchar < map->uchar) {
        val = -1;
    } else {
        val = 0;
    }
    return val;
}

static void ScanHexInt(const std::string&src, unsigned int& value){
    std::stringstream ss;
    ss << std::hex << src;
    ss >> value;
}
static void ScanInt(const std::string&src, unsigned int& value){
    std::stringstream ss;
    ss << src;
    ss >> value;
}

static std::string LowercaseString(const std::string & str)
{
    std::string ret;
    ret.resize(str.size());
    std::transform(str.begin(), str.end(), ret.begin(), ::tolower);
    return ret;
}
    
static void ReplaceChar(std::string& data, int pos, int endPos, unsigned short escapeChar)
{
    if (escapeChar < 32 || escapeChar > 126) escapeChar = ' ';
    char* value = (char*)&escapeChar;
    data.replace(pos, endPos - pos + 1, value);
}
    
void gtm_escaping_html(const std::string& src, std::string& data, HTMLEscapeMap* table, unsigned int size, bool escapeUnicode) {
    
    std::string text;
    for (int i = 0; i < src.length(); ++i) {
        HTMLEscapeMap *val = (HTMLEscapeMap*)bsearch(&src[i], table,
                                                     size / sizeof(HTMLEscapeMap),
                                                     sizeof(HTMLEscapeMap), EscapeMapCompare);
        if (val || (escapeUnicode && src[i] > 127)) {
            if (!text.empty()) {
                data += text;
                text.clear();
            }
            if (val) {
                data += val->escapeSequence;
            } else {
				//assert(escapeUnicode && buffer[i] > 127, @"Illegal Character");
                char cbuf[32];
                memset(cbuf, 0, sizeof(cbuf));
                snprintf(cbuf, sizeof(cbuf), "&#%d;", src[i]);
                data += cbuf;
            }
        } else {
            text += src[i];
        }
    }
    if (!text.empty()) data += text;
    
}
void gtm_unescaping_html(const std::string& src, std::string& data) {
    
    data = src;
    // if no ampersands, we've got a quick way out
    size_t pos = std::string::npos;
    if ( (pos = src.find_last_of("&") ) == std::string::npos) return ;
    
    do {
        size_t endPos = std::string::npos;
        if ( (endPos = src.find(";", pos)) == std::string::npos) continue;
        std::string escapeString = src.substr(pos, endPos - pos + 1);
        if (escapeString.length() > 3 && escapeString.length() < 11) {
            if (escapeString[1] == '#') {
                if (escapeString[2] == 'x' || escapeString[2] == 'X') {
                    // Hex escape squences &#xa3;
                    std::string hexSequence = escapeString.substr(3, escapeString.length() - 4);
                    unsigned int value = -1;
                    ScanHexInt(hexSequence, value);
                    if (value < USHRT_MAX && value > 0) {
                        ReplaceChar(data, pos, endPos, value);
                    }
                    
                }else {
                    // Decimal Sequences &#123;
                    std::string  numberSequence = escapeString.substr(2, escapeString.length() - 3);
                    unsigned int value = -1;
                    ScanInt(numberSequence, value);
                    if (value < USHRT_MAX && value > 0) {
                        ReplaceChar(data, pos, endPos, value);
                    }
                }
            }else {
                // "standard" sequences
                for (unsigned i = 0; i < sizeof(gAsciiHTMLEscapeMap) / sizeof(HTMLEscapeMap); ++i) {
                    if (escapeString == gAsciiHTMLEscapeMap[i].escapeSequence) {
                        ReplaceChar(data, pos, endPos, gAsciiHTMLEscapeMap[i].uchar);
                        break;
                    }
                }
            }
        }
    } while ( pos > 0 && (pos = src.find_last_of("&", pos - 1) ) != std::string::npos);
    
}

void escaping_unicode_html(const std::string& src, std::string& data) {
    gtm_escaping_html(src, data, gUnicodeHTMLEscapeMap, sizeof(gUnicodeHTMLEscapeMap),false);
}

void escaping_ascii_html(const std::string& src, std::string& data) {
    gtm_escaping_html(src, data, gAsciiHTMLEscapeMap, sizeof(gAsciiHTMLEscapeMap),true);
}

void unescaping_html(const std::string& src, std::string& data) {
    gtm_unescaping_html(src, data);
}

void EncodeHtml(const std::string& src, std::string& data, bool unicode) {
    unicode? escaping_unicode_html(src, data) : escaping_ascii_html(src, data);
}

void DecodeHtml(const std::string& src, std::string& data) {
    unescaping_html(src, data);
}

void merge_whitespace(const std::string& src, std::string& data) {
    for (int i = 0; i < src.length(); ++i) {
        if (src[i] == ' ' && data.back() == ' ') continue;
        data += src[i];
    }
}
    
void append_whitespace(std::string& result){
    if (!result.empty() && result.back() == ' ') return ;
    result += ' ';
}
void append_newline(std::string& result){
    if (!result.empty() && result.back() == '\n') return ;
    result += '\n';
}
    
void Html2Text(const std::string& src, std::string& result, bool newline) {
    
    std::string stopCharacters = "< \t\n\r";
    //stopCharacters += (unsigned char)0x0085;
    stopCharacters += (unsigned char)0x000C;
    stopCharacters += (unsigned char)0x2028;
    stopCharacters += (unsigned char)0x2029;
    
    std::string newLineAndWhitespaceCharacters = " \t\n\r";
    //newLineAndWhitespaceCharacters += (unsigned char)0x0085;
    newLineAndWhitespaceCharacters += (unsigned char)0x000C;
    newLineAndWhitespaceCharacters += (unsigned char)0x2028;
    newLineAndWhitespaceCharacters += (unsigned char)0x2029;
    
    std::string tagNameCharacters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string tagName,tagText;
    
    bool dontReplaceTagWithSpace = false;
    int i = 0;//总是指向下一个
    while (i < src.length()) {
        //scan up to stop char
        tagText.clear();
        while (i < src.length() && stopCharacters.find(src[i]) == std::string::npos) {
            if (tagName == "style" || tagName == "script" || tagName == "noscript" || tagName == "title") ++i;
            else tagText += src[i++];
        };
        result += tagText;
        if (i >= src.length()) break;
        // Check if we've stopped at a tag/comment or whitespace
        if (src[i] == '<') {
            ++i; if (i >= src.length()) break;
            // Stopped at a comment or tag
            if (src.substr(i, 3) == "!--") {
                // Comment
                size_t pos = src.find("-->" , i);
                if (pos != std::string::npos) {
                    i += pos - i + 3;
                    if (i >= src.length()) break;
                }else
                    break;
            }else{
                // Tag - remove and replace with space unless it's
                // a closing inline tag then dont replace with a space
                tagName.clear();
                if (src[i] == '/') {
                    // Closing tag - replace with space unless it's inline
                    ++i; if (i >= src.length()) break;
                    dontReplaceTagWithSpace = false;
                    while (i < src.length() && tagNameCharacters.find(src[i]) != std::string::npos) {
                        tagName += src[i++];
                    };
                    tagName = LowercaseString(tagName);
                    dontReplaceTagWithSpace = (tagName == "a" ||
                                               tagName == "b" ||
                                               tagName == "i" ||
                                               tagName == "q" ||
                                               tagName == "span" ||
                                               tagName == "em" ||
                                               tagName == "strong" ||
                                               tagName == "cite" ||
                                               tagName == "abbr" ||
                                               tagName == "acronym" ||
                                               tagName == "label");
                    
                    if (!dontReplaceTagWithSpace && result.length() > 0 && i < src.length() - 1) append_whitespace(result);
                    if (i >= src.length()) break;
                }else{
                    // Scan past tag
                    while (i < src.length() && tagNameCharacters.find(src[i]) != std::string::npos) {
                        tagName += src[i++];
                    };
                    tagName = LowercaseString(tagName);
                    if (tagName == "br") {
                        newline? append_newline(result):append_whitespace(result);
                    }
                    if (i >= src.length()) break;
                }
                
                // Scan to tag end
                while (i < src.length() && src[i++] != '>') ;
                if (i >= src.length()) break;
            }
            
        }else{
            // Stopped at whitespace - replace all whitespace and newlines with a space
            while (i < src.length() && newLineAndWhitespaceCharacters.find(src[i++]) == std::string::npos);
            if (result.length() > 0 && i < src.length() - 1) append_whitespace(result);
            if (i >= src.length()) break;
        }
    }
    std::string decode_result;
    DecodeHtml(result, decode_result);
    std::string merge_result;
    merge_whitespace(decode_result, merge_result);//merge decoded &nbsp;
    result = merge_result;
}

void Text2Html(const std::string& src, std::string& data) {
    
}

}
