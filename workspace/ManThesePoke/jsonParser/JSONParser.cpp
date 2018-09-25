#include "JSONParser.h"

static JSONObject* parseObject(auto&, const auto&);
static JSONArray* parseArray(auto&, const auto&);
static JSONValue* parseValue(auto&, const auto&);
static bool check(auto&, const auto&, JSONTokenType);
static bool expect(auto&, const auto&, JSONTokenType);

static JSONTokenType getType(auto&);

JSONValue* JSONObject::get(const icu::UnicodeString& str) const{
    for (auto i : children){
        if (i.first->compare(str) == 0)
            return i.second;
    }
    
    return nullptr;
}

JSONValue* JSONObject::get(const char* str) const{
    return get(icu::UnicodeString(str, "ASCII"));
}

JSONValue* JSONObject::operator[](const icu::UnicodeString& str) const{
    return get(str);
}

JSONValue* JSONObject::operator[](const char* str) const{
    return get(str);
}

JSONValue* JSONArray::get(std::size_t i) const{
    return children[i];
}

JSONValue* JSONArray::operator[](std::size_t i) const{
    return children[i];
}

JSONValue* parseJSON(const icu::UnicodeString& source){
    std::list<JSONToken*>* tokens = tokenize(source);
    
    if (tokens == nullptr) return nullptr;
    
    auto b = tokens->begin();
    auto e = tokens->end();
    JSONValue* rootObj = parseValue(b, e);
    
    delete tokens;
    
    return rootObj;
}

static JSONArray* parseArray(auto& iterator, const auto& end){
    JSONArray* array = new JSONArray();
    JSONValue* valueTemp = nullptr;
    if (!expect(iterator, end, JSONTokenType::OpenBracket)){
        delete array;
        array = nullptr;
        return array;
    }
    
    
    iterator++;
    
    while (true) {
        
        if (check(iterator, end, JSONTokenType::ClosedBracket)){
            iterator++;
            break;
        }
        
        
        if (iterator == end) {
            std::cout << "Error parsing JSON: Premature exit" << std::endl;
            delete array;
            array = nullptr;
            return array;
        }
        
        valueTemp = parseValue(iterator, end);
        
        if (valueTemp == nullptr){
            std::cout << "Error parsing JSON: Unknown values" << std::endl;
            delete array;
            array = nullptr;
            return array;
        }
        
        array->children.push_back(valueTemp);
        
        if (check(iterator, end, JSONTokenType::Comma))
            iterator++;
        
    }
    
    return array;
}

static JSONObject* parseObject(auto& iterator, const auto& end){
    JSONObject* obj = new JSONObject();
    JSONToken* keyTemp = nullptr;
    
    JSONValue* valueTemp = nullptr;
    
    
    if (!expect(iterator, end, JSONTokenType::OpenCBracket)){
        delete obj;
        obj = nullptr;
        return obj;
    }
    
    iterator++;
    
    while (true) {
        if (check(iterator, end, JSONTokenType::ClosedCBracket)){
            iterator++;
            break;
        }
        
        if (!expect(iterator, end, JSONTokenType::String)){
            delete obj;
            obj = nullptr;
            break;
        }
        
        keyTemp = *iterator++;
        
        if (!expect(iterator, end, JSONTokenType::Colon)){
            delete obj;
            obj = nullptr;
            break;
        }
        
        iterator++;
        valueTemp = parseValue(iterator, end);
        
        obj->children[new icu::UnicodeString(*keyTemp->value.stringValue)] = valueTemp;
        if (check(iterator, end, JSONTokenType::Comma))
            iterator++;
        
    }
    return obj;
}


static JSONValue* parseValue(auto& iterator, const auto& end){
    if (iterator == end) return nullptr;
    
    JSONToken* temp = *iterator;
    switch (getType(iterator)){
        case JSONTokenType::String:
            iterator++;
            return new JSONValue(temp->value.stringValue);
            break;
        case JSONTokenType::Boolean:
            iterator++;
            return new JSONValue(temp->value.booleanValue);
            break;
        case JSONTokenType::Number:
            iterator++;
            return new JSONValue(temp->value.numberValue);
            break;
        case JSONTokenType::OpenCBracket: {
            return new JSONValue(parseObject(iterator, end));;
            break;
        }
        case JSONTokenType::OpenBracket:
            return new JSONValue(parseArray(iterator, end));
            break;
        default:
            return nullptr;
            break;
    }
}

inline static JSONTokenType getType(auto& iterator){
    return (*iterator)->type;
}
            
static bool check(auto& iterator, const auto& end, JSONTokenType type){
    if (iterator == end) return false;
    
    if (getType(iterator) != type) return false;
    
    return true;
}
            
static bool expect(auto& iterator, const auto& end, JSONTokenType type){
    if (iterator == end){
        std::cout << "Error parsing JSON: premature exit" << std::endl;
        return false;
    }
    
    JSONTokenType temp = getType(iterator);
    if (temp != type){
        std::cout << "Error parsing JSON: expected " << type << " got " << temp << std::endl;
        return false;
    }
    
    return true;
}