#ifndef JSONTOKENI_H 
#define JSONTOKENI_H 1

#include <unicode/unistr.h>
#include <unicode/ustream.h>
#include <list>


enum class JSONTokenType {
  String, Number, Boolean, OpenCBracket, ClosedCBracket, Comma, Colon, OpenBracket, ClosedBracket, Null
};

struct JSONToken {
    const JSONTokenType type;
    const union {
        icu::UnicodeString* stringValue;
        double numberValue;
        bool booleanValue;
    }value;
  
    JSONToken(JSONTokenType t): type(t), value{}{}
    JSONToken(JSONTokenType t, bool b): type(t), value{.booleanValue = b}{}
    JSONToken(JSONTokenType t, icu::UnicodeString* str): type(t), value{.stringValue = str}{}
    JSONToken(JSONTokenType t, double d): type(t), value{.numberValue = d}{}
    ~JSONToken(){
        if (type == JSONTokenType::String){
            delete value.stringValue;
        }
    }
};

std::list<JSONToken*>* tokenize(const icu::UnicodeString&);
std::ostream& operator<<(std::ostream&, const JSONTokenType);
std::ostream& operator<<(std::ostream&, const JSONToken&);



#endif