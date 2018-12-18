/******************************************************************

*******************************************************************/
#ifndef JsonCpp_H_
#define JsonCpp_H_

#include <string>
#include <cassert>
#include <cctype>
#include <vector>
#include <unordered_map>
#include <iostream>

namespace JsonCpp
{

/************Json数据类型*************/
enum value_type
{
    JSON_NULL,
    JSON_FALSE,
    JSON_TRUE,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
};

/****************结果返回值**************/
enum
{
    PARSE_OK,                   // 解析成功
    PARSE_EXPECT_VALUE,         // 只含有空白     
    PARSE_INVALID_VALUE,        // 无效值
    PARSE_ROOT_NOT_SINGULAR,     // 空白后还有其他内容
    PARSE_NUMBER_TOO_BIG,
    PARSE_MISS_QUOTATION_MARK,
    PARSE_INVALID_UNICODE_HEX,
    PARSE_INVALID_UNICODE_SURROGATE,
    PARSE_INVALID_STRING_ESCAPE,
    PARSE_INVALID_STRING_CHAR,
    PARSE_MISS_COMMA_OR_SQUARE_BRACKET,
    PARSE_MISS_KEY,
    PARSE_MISS_COLON,
    PARSE_MISS_COMMA_OR_CURLY_BRACKET,
    OBJECT_KEY_NOT_EXIST,
    GENERATE_OK
};

class Parser;
class Generator;

class Value{
    friend Parser;
    friend Generator;
    friend bool operator==(const Value &lhs, const Value &rhs);
    friend std::ostream &operator<<(std::ostream &os, const Value &v);
private:
    value_type type;

    // union中不要带有包含构造函数的类型（string，vector等等）
    // 否则在构造的时候编译器会很迷茫
    union{
        std::string* str;
        double* num;
        std::vector<Value>* array;
        std::unordered_map<std::string, Value>* object;
    };
public:
    Value() : type(JSON_NULL){}
    Value(const Value &v);
    Value(const double num);
    Value(const std::string &str);

    void set_null();
    void set_true();
    void set_false();
    void set_number(double n);
    void set_string(const std::string &s);
    int get_type() const;

    std::vector<Value> get_array();
    int get_array_size() const;
    Value* get_array_element(size_t index) const;
    void erase_array_element(size_t index, size_t count);
    void clear_array();
    void insert_array_element(Value &v, size_t index);

    std::unordered_map<std::string, Value> get_object();
    int get_object_size() const;
    bool find_object_value(const std::string &key) const;
    Value *get_object_value(const std::string &key) const;
    void set_object_value(const std::string &key, Value &v);
    void remove_object_value(const std::string &key);
    

    double get_number() const;
    std::string get_string() const;
    void free();

    Value& operator=(const Value &rhs);
    Value& operator=(const std::string &str);
    Value& operator=(const double num);
    Value& operator[](size_t index);
    Value& operator[](const std::string &s);
};

class Buffer{
    friend Parser;
    friend Generator;
private:
    char *stack = nullptr;
    size_t size = 0;
    size_t top = 0;
public:
    void *push(size_t size);
    void *pop(size_t size);
    void put_char(char ch);
    void put_string(const char* s, int len);
    void clear();
    ~Buffer() { clear(); }
};

class Parser{
    friend int Json_Parse(const std::string &json, Value &value);

private:
    const std::string &json;
    size_t pos;
    Buffer buf;

    Parser(const std::string &s):json(s), pos(0) {}
    int run(Value &v);
    int parse_value(Value &v);
    void parse_whitespace();
    int parse_literal(Value &v, const std::string &s);
    int parse_string(Value &v);
    int parse_string_raw(std::string &s);
    int parse_number(Value &v);
    bool parse_hex4(unsigned &u);
    void encode_utf8(unsigned u);
    int parse_array(Value &v);
    int parse_object(Value &v);

    inline bool ISDIGIT09(char ch){ return ch >= '1' && ch <= '9';}
    inline bool ISDIGIT(char ch){return ch >= '0' && ch <= '9';}
};

class Generator{
    friend int Json_Generate(std::string &json, const Value &value);
private:
    std::string &json;
    size_t pos = 0;
    Buffer buf;

    Generator(std::string &s) : json(s) {}
    int run(const Value &v);
    int stringify_value(const Value &v);
    void stringify_string(const Value &v);
};


bool operator==(const Value &lhs, const Value &rhs);
bool operator!=(const Value &lhs, const Value &rhs);
std::ostream &operator<<(std::ostream &os, const Value &v);


int Json_Parse(const std::string &json, Value &value);
int Json_Generate(std::string &json, const Value &value);
void Json_Print(std::ostream &os, const std::string &json);

}  // namespace JsonCpp
#endif