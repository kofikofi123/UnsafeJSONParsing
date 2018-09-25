#include "JSONTokenizer.h"
#include <iostream>
#include <cmath>

template<class ... Args>
static JSONToken* createToken(JSONTokenType, Args&&...);

static UChar32 advance(const icu::UnicodeString&, int32_t&);
static bool parseString(const icu::UnicodeString&, int32_t&, icu::UnicodeString*&);
static bool parseBoolean(const icu::UnicodeString&, int32_t&, bool&);
static bool parseNumber(const icu::UnicodeString&, int32_t&, double&);

static bool isDigit(UChar32);
static bool isEOF(const icu::UnicodeString&, int32_t&);

static const char16_t _true[] = {0x74, 0x72, 0x75, 0x65};
static const char16_t _false[] = {0x66, 0x61, 0x6C, 0x73, 0x65};
static const char16_t _null[] = {0x6E, 0x75, 0x6C, 0x6C};


std::list<JSONToken*>* tokenize(const icu::UnicodeString& string){
    enum class JSONT_States {};
    std::list<JSONToken*>* tokens = new std::list<JSONToken*>();
    int32_t currentPosition = 0;
    UChar32 currentCharacter = 0;
    const int32_t length = string.length();
    JSONToken* token = nullptr;
    
    while (1){
        token = nullptr;
        currentCharacter = string.char32At(currentPosition);
        if (currentCharacter == 0x7B)
            token = createToken(JSONTokenType::OpenCBracket);
        else if (currentCharacter == 0x7D)
            token = createToken(JSONTokenType::ClosedCBracket);
        else if (currentCharacter == 0x3A)
            token = createToken(JSONTokenType::Colon);
        else if (currentCharacter == 0x5B)
            token = createToken(JSONTokenType::OpenBracket);
        else if (currentCharacter == 0x5D)
            token = createToken(JSONTokenType::ClosedBracket);
        else if (currentCharacter == 0x2C)
            token = createToken(JSONTokenType::Comma);
        else if (currentCharacter == 0x22){
            icu::UnicodeString* str = nullptr;
            
            if (!parseString(string, currentPosition, str)){
                delete tokens;
                tokens = nullptr;
                break;
            }
            token = createToken(JSONTokenType::String, str);
        }else if (currentCharacter == 0x74 || currentCharacter == 0x66){
            bool result = true;
            if (!parseBoolean(string, currentPosition, result)){
                delete tokens;
                tokens = nullptr;
                break;
            }
       
            token = createToken(JSONTokenType::Boolean, result);
        }else if (isDigit(currentCharacter) || currentCharacter == 0x2D || currentCharacter == 0x30){
            double result = 0;
            if (!parseNumber(string, currentPosition, result)){
                delete tokens;
                tokens = nullptr;
                break;
            }
            
            token = createToken(JSONTokenType::Number, result);
        }else if (currentCharacter == 0x6E){
            if (string.compare(currentPosition, 3, _null) != 0){
                delete tokens;
                tokens = nullptr;
                break;
            }
            
            token = createToken(JSONTokenType::Null);
        }else if (currentPosition >= length)
            break;
        
        if (token != nullptr)
            tokens->push_back(token);
        currentCharacter = advance(string, currentPosition);
    }
  
  return tokens;
}

inline static UChar32 advance(const icu::UnicodeString& string, int32_t& position){
  return string.char32At(++position);
}

static bool parseString(const icu::UnicodeString& string, int32_t& position, icu::UnicodeString*& output){
    position++;
    icu::UnicodeString* temp = new icu::UnicodeString();
    
  
    UChar32 currentInput = string.char32At(position);
    while (1){
        if (currentInput == 0x5C){
            currentInput = advance(string, position);
            switch (currentInput){
                case 0x5C:
                case 0x22:
                    break;
                case 0x62:
                    currentInput = 0x08;
                    break;
                case 0x66:
                    currentInput = 0x0C;
                    break;
                case 0x6E:
                    currentInput = 0x0A;
                    break;
                case 0x72:
                    currentInput = 0x72;
                    break;
                case 0x74:
                    currentInput = 0x09;
                    break;
                case 0x75:
                    currentInput = 0;
                    for (unsigned char i = 0; i < 4; i++){
                        currentInput = currentInput | advance(string, position);
                    }
                    break; 
            }
            temp->append(currentInput);
        }else if (currentInput == 0x22){
            break;
        }else if (isEOF(string, position)){
            delete temp;
            output = nullptr;
            return false;
        }else
            temp->append(currentInput);
        currentInput = advance(string, position);
    }
    output = temp;
    return true;
}

template<class ... Args>
inline JSONToken* createToken(JSONTokenType type, Args&&... args){
  return new JSONToken(type, std::forward<Args>(args)...);
}


static bool parseBoolean(const icu::UnicodeString& string, int32_t& position, bool& output){
    bool result = false;
    if (string.compare(position, 3, _true) == 0){
        output = true;
        position = position + 3;
        result = true;
    }else if (string.compare(position, 5, _false) == 0){
        output = false;
        position = position + 4;
        result = true;
    }
    
    return result;
}

static bool parseNumber(const icu::UnicodeString& string, int32_t& position, double& output){
    double temp = 0;
    double temp2 = 0;
    int16_t temp3 = 0;
    uint8_t sign = 0;
    uint8_t deci = 0;
    
    UChar32 currentInput = string.char32At(position);
    
    uint64_t* uTemp;
    
    if (currentInput == 0x2D){
        sign = 1;
        currentInput = advance(string, position);
    }
    
    
    if (currentInput != 0x30){ 
        if (!isDigit(currentInput)) return false;
        while (isDigit(currentInput)){
            temp = (temp * 10) + (currentInput - 0x30);
            currentInput = advance(string, position);
        }
    }else
        currentInput = advance(string, position);
    
    if (currentInput == 0x2E){
        currentInput = advance(string, position);
        if (!isDigit(currentInput)) return false;
        
        while (isDigit(currentInput)){
            temp2 = (temp2 * 10) + (currentInput - 0x30);
            currentInput = advance(string, position);
            deci++;
        }
    }
    
    if (currentInput == 0x65 || currentInput == 0x45){
        currentInput = advance(string, position);
        
        uint8_t sign2 = 0;
        if (currentInput == 0x2B || currentInput == 0x2D){
            sign2 = (currentInput == 0x2D ? 1 : 0);
            currentInput = advance(string, position);
        }
        
        while (isDigit(currentInput)){
            temp3 = (temp3 * 10) + (currentInput - 0x30);
            currentInput = advance(string, position);
        }
        
        if (sign2)
            temp3 = temp3 | (1 << 15);
    }
    
    if (sign){
        uTemp = reinterpret_cast<uint64_t*>(&temp);
        *uTemp = *uTemp | ((uint64_t)1 << 63);
    }
    
    output = (temp + (temp2 / pow(10, deci))) * pow(10, temp3);
    
    position--;
    
    
    return true;
}
               
inline static bool isEOF(const icu::UnicodeString& string, int32_t& position){
    return (position >= string.length());
}

inline static bool isDigit(UChar32 c){
    return (c > 0x30 && c <= 0x39);
}
std::ostream& operator<<(std::ostream& stream, JSONTokenType type){
    switch(type){
        case JSONTokenType::String:
            stream << "<STRING>";
            break;
        case JSONTokenType::Number:
            stream << "<NUMBER>";
            break;
        case JSONTokenType::Boolean:
            stream << "<BOOL>";
            break;
        case JSONTokenType::OpenCBracket:
            stream << "<OPEN_C_BRACKET>";
            break;
        case JSONTokenType::ClosedCBracket:
            stream << "<CLOSED_C_BRACKET>";
            break;
        case JSONTokenType::Comma:
            stream << "<COMMA>";
            break;
        case JSONTokenType::Colon:
            stream << "<COLON>";
            break;
        default:
            stream << "<UNKNOWN>";
            break;
    }
    return stream;
}
std::ostream& operator<<(std::ostream& stream, const JSONToken& token){
    JSONTokenType type = token.type;
    stream << "{Type: " << type << " Value: {";
    
    
    switch (type){
        case JSONTokenType::String:
            stream << *token.value.stringValue;
            break;
        case JSONTokenType::Boolean:
            stream << (token.value.booleanValue == true ? "true" : "false");
            break;
        case JSONTokenType::Number:
            stream << token.value.numberValue;
            break;
        default:
            stream << "???";
            break;
    }
    
    stream << "}}";
    
    return stream;
}