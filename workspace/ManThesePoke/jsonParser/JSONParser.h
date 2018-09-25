#ifndef JSONParser_H
#define JSONParser_H 1

#include "JSONTokenizer.h"
#include <memory>
#include <map>
#include <vector>
#include <unicode/unistr.h>
/*
root
|
|
a
  \
   \
    2
    



*/

enum class JSONType {String, Number, Boolean, Null, Object, Array};

class JSONValue;

class JSONArray {
    friend JSONArray* parseArray(auto&, const auto&);
    std::vector<JSONValue*> children;
public:
    JSONArray(){}
    ~JSONArray(){
        for (auto& i : children){
            delete i;
        }
    }
    
    JSONValue* get(std::size_t) const;
    JSONValue* operator[](std::size_t) const;
    
    std::size_t getChildrenSize(){return children.size();}
    auto begin(){return children.begin();}
    auto end(){return children.end();}
};

class JSONObject {
    friend JSONObject* parseObject(auto&, const auto&);
    std::map<icu::UnicodeString*, JSONValue*> children;
public:
    JSONObject(){}
    ~JSONObject(){
        for (auto& i : children){
            delete i.first;
            delete i.second;
        }
    }
    std::size_t getChildrenSize(){return children.size();}
    
    JSONValue* get(const icu::UnicodeString&) const;
    JSONValue* get(const char*) const;
    JSONValue* operator[](const icu::UnicodeString&) const;
    JSONValue* operator[](const char*) const;
    
    
    auto begin(){return children.begin();}
    auto end(){return children.end();}
    
};

class JSONValue {
    friend JSONObject* parseObject(auto&, const auto&);
    union {
        double number;
        bool boolean;
        icu::UnicodeString* string;
        JSONArray* array;
        JSONObject* object;
    } value; //:(
public:
    const JSONType type;
    
    JSONValue(): type(JSONType::Null){}
    JSONValue(double v): value {.number=v}, type(JSONType::Number){}
    JSONValue(bool b):  value {.boolean=b}, type(JSONType::Boolean){}
    JSONValue(icu::UnicodeString* str): value {.string=str}, type(JSONType::String){}
    JSONValue(JSONArray* a): value {.array=a}, type(JSONType::Array){}
    JSONValue(JSONObject* o): value {.object=o}, type(JSONType::Object){}
    
    ~JSONValue(){
        switch(type){
            case JSONType::String:
                delete value.string;
                break;
            case JSONType::array:
                delete value.array;
                break;
            case JSONType::object:
                delete value.object;
                break;
            default:
                break;
        }
    }
    
    const double getNumber() const{return value.number;}
    const bool getBoolean() const{return value.boolean;}
    const icu::UnicodeString* getString() const{return value.string;}
    const JSONArray* getArray() const{return value.array;}
    const JSONObject* getObject() const{return value.object;}
   
};

JSONValue* parseJSON(const icu::UnicodeString&);

#endif
