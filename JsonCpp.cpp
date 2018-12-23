#include "JsonCpp.h"
#include <sstream>
#include <cassert>
#include <iostream>
#include <cerrno>   // errno
#include <cfloat>   // DBL_MAX
//#include <cstdio>


namespace JsonCpp
{
/**********************************************************
 *                                                        *
 *                                                        *
 *                        API                             *
 *                                                        *
 *                                                        *
 * ********************************************************/
int Json_Parse(const std::string &json, Value &value)
{
    Parser parser(json);
    return parser.run(value);
}

int Json_Generate(std::string &json, const Value &value)
{
    Generator generator(json);
    return generator.run(value);
}

void Json_Print(std::ostream &os, const std::string &json)
{
    int depth = 0;
    size_t len = json.length();
    for (int i = 0; i != len; ++i){
        if(json[i] == '{' || json[i] == '['){
            os << json[i];
            os << "\n";
            depth++;
            for (int i = 0; i != depth; ++i){
                os << "\t";
            }
            continue;
        }
        if(json[i] == '}' || json[i] == ']'){
            depth--;
            os << "\n";
            for (int i = 0; i != depth; ++i){
                os << "\t";
            }
            os << json[i];
            continue;
        }

        if(json[i] == ':'){
            os << json[i];
            os << " ";
            continue;
        }
        if(json[i] == ','){
           os << json[i];
           os << "\n";
           for (int i = 0; i != depth; ++i){
                os << "\t";
            }
            continue; 
        }
        os << json[i];
    }
}

/**********************************************************
 *                                                        *
 *                                                        *
 *                     Parser                             *
 *                                                        *
 *                                                        *
 * ********************************************************/
int Parser::run(Value &v)
{
    parse_whitespace();
    int ret = parse_value(v);
    if(ret == PARSE_OK){
        parse_whitespace();
        if(pos != json.length()){
            v.set_null();
            ret = PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

void Parser::parse_whitespace()
{
    while(json[pos] == ' ' || json[pos] == '\t' || json[pos] == '\n' || json[pos] == '\r')
    {
        ++pos;
    }
}

int Parser::parse_value(Value &v)
{
    if(pos == json.length()){
        return PARSE_EXPECT_VALUE;
    }
    switch(json[pos])
    {
        case 'n':
            return parse_literal(v, "null");
        case 'f':
            return parse_literal(v, "false");
        case 't':
            return parse_literal(v, "true");
        case '\"':
            return parse_string(v);
        case '[':
            return parse_array(v);
        case '{':
            return parse_object(v);
        default:
            return parse_number(v);
    }
}

int Parser::parse_object(Value &v)
{
    int ret;
    pos++;
    parse_whitespace();
    if(json[pos] == '}'){
        pos++;
        v.type = JSON_OBJECT;
        v.object = nullptr;
        return PARSE_OK;
    }
    std::unordered_map<std::string, Value> tmp_object;
    while(1){
        std::string tmp_key;
        Value tmp_value;


       /**********Parse Key***************/
        if(json[pos] != '\"'){
            ret = PARSE_MISS_KEY;
            break;
        }   

        int tmp = parse_string_raw(tmp_key);
        if(tmp != PARSE_OK){
            ret = PARSE_MISS_KEY;
            break;
        } 
        parse_whitespace();
        if(json[pos] != ':'){
            ret = PARSE_MISS_COLON;
            break;
        }
        pos++;
        parse_whitespace();  
        if((ret = parse_value(tmp_value)) != PARSE_OK){
            break;
        }

        tmp_object[tmp_key] = tmp_value;

        parse_whitespace(); 
        if(json[pos] == ','){
            pos++;
            parse_whitespace();
        }
        else if(json[pos] ==  '}'){
            pos++;
            v.type = JSON_OBJECT;
            v.object = new std::unordered_map<std::string, Value>(tmp_object);
            ret = PARSE_OK;
            break;
        }
        else{
            ret = PARSE_MISS_COMMA_OR_CURLY_BRACKET;
            break;
        }
    }
    return ret;
}

int Parser::parse_array(Value &v)
{
    size_t size = 0;
    int ret;
    pos++;
    parse_whitespace();
    if(json[pos] == ']')
    {
        pos++;
        v.type = JSON_ARRAY;
        v.array = nullptr;
        return PARSE_OK;
    }
    std::vector<Value> tmp;
    while(1)
    {
        Value e;
        if((ret = parse_value(e)) != PARSE_OK)
        {
            break;
        }
        tmp.push_back(std::move(e)); 
        parse_whitespace();
        if(json[pos] == ','){
            pos++;
            parse_whitespace();
        }
        else if(json[pos] == ']'){
            pos++;
            v.type = JSON_ARRAY;
            v.array = new std::vector<Value>(tmp);
            return PARSE_OK;
        }
        else{
            ret = PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
            break;
        }
    }
    return ret;
}

int Parser::parse_number(Value &v)
{
    size_t head = pos;
    if(json[pos] == '-'){
        pos++;
    }
    if(json[pos] == '0'){
        pos++;
    }
    else{
        if(!ISDIGIT09(json[pos])){
            return PARSE_INVALID_VALUE;         // 非数字
        }
        for (pos++; ISDIGIT(json[pos]); pos++);
    }

    if(json[pos] == '.'){
        pos++;
        if(!ISDIGIT(json[pos])){
            return PARSE_INVALID_VALUE;
        }
        for(pos++; ISDIGIT(json[pos]); pos++);
    }

    if(json[pos] == 'e' || json[pos] == 'E'){
        pos++;
        if(json[pos] == '+' || json[pos] == '-'){
            pos++;
        }
        if(!ISDIGIT(json[pos])){
            return PARSE_INVALID_VALUE;
        }
        for (pos++; ISDIGIT(json[pos]); pos++);
    }
    size_t size = pos - head;
    errno = 0;
    double tmp;
    std::stringstream ss(json.substr(head, size));
    ss >> tmp;

    if(errno == ERANGE && (tmp == DBL_MAX || tmp == -DBL_MAX))
    {
        return PARSE_NUMBER_TOO_BIG;
    }

    v.set_number(tmp);
    return PARSE_OK;
}

int Parser::parse_literal(Value &v, const std::string &s)
{
    int len = s.length();
    for (int i = 0; i != len; ++i){
        if(s[i] != json[pos + i]){
            return PARSE_INVALID_VALUE;
        }
    }
    pos += len;
    switch(s[0]){
        case 'n':
            v.set_null();
            break;
        case 'f':
            v.set_false();
            break;
        case 't':
            v.set_true();
            break;
        default:
            break;
    }
    return PARSE_OK;
}

#define STRING_ERROR(ret) \
    do                    \
    {                     \
        buf.pop(len);     \
        return ret;       \
    }while(0)

int Parser::parse_string(Value &v)
{
    std::string tmp;
    int ret = parse_string_raw(tmp);
    if(ret == PARSE_OK){
        v.set_string(tmp);
    }
    return ret;
}

int Parser::parse_string_raw(std::string &s)
{
    unsigned u;
    unsigned u2;
    pos++;
    size_t head = buf.top;
    size_t len = 0;
    while(1){
        char ch = json[pos++];
        switch(ch)
        {
            case '\"':
                len = buf.top - head;       
                s.append((char*)buf.pop(len), len);
                return PARSE_OK;
            case '\0':
                buf.top = head;
                return PARSE_MISS_QUOTATION_MARK;
            case '\\':
                switch(json[pos++])
                {
                    case '\\':
                        buf.put_char('\\');
                        break;
                    case '/':
                        buf.put_char('/');
                        break;
                    case 'b':  
                        buf.put_char('\b');
                        break;
                    case 'f':  
                        buf.put_char('\f');
                        break;
                    case 'n':  
                        buf.put_char('\n');
                        break;
                    case 'r':  
                        buf.put_char('\r');
                        break;
                    case 't':  
                        buf.put_char('\t');
                        break;
                    case '\"': 
                        buf.put_char('\"');
                        break;
                    case 'u':
                        if(!parse_hex4(u)){
                            STRING_ERROR(PARSE_INVALID_UNICODE_HEX);
                        }
                        if(u >= 0xD800 && u <= 0xDBFF)
                        {
                            if(json[pos++] != '\\'){
                                STRING_ERROR(PARSE_INVALID_UNICODE_SURROGATE);
                            }
                            if(json[pos++] != 'u'){
                                STRING_ERROR(PARSE_INVALID_UNICODE_SURROGATE);
                            }
                            if(!parse_hex4(u2)){
                                STRING_ERROR(PARSE_INVALID_UNICODE_HEX);
                            }
                            if(u2 < 0xDC00 || u2 > 0xDFFF){
                                STRING_ERROR(PARSE_INVALID_UNICODE_SURROGATE);
                            }
                            u = 0x10000 + (u - 0xD800) * 0x400 + (u2 - 0xDC00);
                    }
                    encode_utf8(u);
                    break;
                default:
                    STRING_ERROR(PARSE_INVALID_STRING_ESCAPE);
                }
                break;
            default:
                if((unsigned char)ch < 0x20){
                    STRING_ERROR(PARSE_INVALID_STRING_CHAR);
                }
                buf.put_char(ch);
        }
    }
}

void Parser::encode_utf8(unsigned u)
{
    if(u <= 0x007F){
        buf.put_char(u & 0xFF);
    }
    else if(u <= 0x7FF){
        buf.put_char(0xC0 | (u >> 6) & 0xFF);
        buf.put_char(0x80 | u & 0x3F);
    }
    else if(u <= 0xFFFF){
        buf.put_char(0xE0 | ((u >> 12) & 0xFF));
        buf.put_char(0x80 | ((u >> 6) & 0x3F));
        buf.put_char(0x80 | u & 0x3F);
    }
    else{
        assert(u <= 0x10FFFF);
        buf.put_char(0xF0 | (u >> 18) & 0xFF);
        buf.put_char(0x80 | (u >> 12) & 0x3F);
        buf.put_char(0x80 | (u >> 6) & 0x3F);
        buf.put_char(0x80 | u & 0x3F);
    }
}

bool Parser::parse_hex4(unsigned &u)
{
    u = 0;
    for (int i = 0; i != 4; ++i){
        char ch = json[pos++];
        u <<= 4;
        if(ch >= '0' && ch <= '9'){
            u |= (ch - '0');
        }
        else if(ch >= 'A' && ch <= 'F'){
            u |= (ch - 'A' + 10);
        }
        else if(ch >= 'a' && ch <= 'f'){
            u |= (ch - 'a' + 10);
        }
        else{
            return false;
        }
    }
    return true;
}

/**********************************************************
 *                                                        *
 *                                                        *
 *                     Generator                          *
 *                                                        *
 *                                                        *
 * ********************************************************/
int Generator::run(const Value &v)
{
    int ret = stringify_value(v);
    if(ret != GENERATE_OK)
    {
        buf.clear();
        return ret;
    }
    json.append(buf.stack, buf.top);
    return GENERATE_OK;
}


int Generator::stringify_value(const Value &v)
{
    char *tmp_buffer;
    size_t tmp_length;
    switch(v.type)
    {
        case JSON_NULL:
            buf.put_string("null", 4);
            break;
        case JSON_FALSE:
            buf.put_string("false", 5);
            break;
        case JSON_TRUE:
            buf.put_string("true", 4);
            break;
        case JSON_NUMBER:
            tmp_buffer = (char*)buf.push(32);
            tmp_length = sprintf(tmp_buffer, "%.17g", *(v.num));
            buf.top -= 32 - tmp_length;
            break;
        case JSON_STRING:
            stringify_string(v);
            break;
        case JSON_ARRAY:
            buf.put_char('[');
            if(v.array){
                for (int i = 0; i != v.array->size(); ++i){
                    stringify_value((*(v.array))[i]);
                    if(i != v.array->size() - 1){
                        buf.put_char(',');
                    }
                }
            }
            buf.put_char(']');
            break;
        case JSON_OBJECT:
            buf.put_char('{');
            if(v.object){
                size_t cnt = 0;
                for(auto & p : *(v.object)){
                    stringify_string(p.first);
                    buf.put_char(':');
                    stringify_value(p.second);
                    if(cnt != v.object->size() - 1){
                        buf.put_char(',');
                    }
                    ++cnt;
                }
            }
            buf.put_char('}');
            break;
        default:
            break;
    }
    return GENERATE_OK;
}

void Generator::stringify_string(const Value &v)
{
    const char hex_digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    char *head;
    char *p;
    size_t len = v.str->length();
    size_t size = len * 6 + 2;
    p = head = (char*)buf.push(size);
    *p++ = '"';
    for (size_t i = 0; i != len; ++i)
    {
        unsigned char ch = (unsigned char)(*(v.str))[i];
        switch(ch){
            case '\"': *p++ = '\\'; *p++ = '\"'; break;
            case '\\': *p++ = '\\'; *p++ = '\\'; break;
            case '\b': *p++ = '\\'; *p++ = 'b';  break;
            case '\f': *p++ = '\\'; *p++ = 'f';  break;
            case '\n': *p++ = '\\'; *p++ = 'n';  break;
            case '\r': *p++ = '\\'; *p++ = 'r';  break;
            case '\t': *p++ = '\\'; *p++ = 't';  break;
            default:
                if(ch < 0x20){
                    *p++ = '\\'; 
                    *p++ = 'u';
                    *p++ = '0';
                    *p++ = '0';
                    *p++ = hex_digits[ch >> 4];
                    *p++ = hex_digits[ch & 15];
                }  
                else{
                    *p++ = (*(v.str))[i];
                }            
        }
    }
    *p++ = '"';
    buf.top -= size - (p - head);
}


/**********************************************************
 *                                                        *
 *                                                        *
 *                      Buffer                            *
 *                                                        *
 *                                                        *
 * ********************************************************/
void* Buffer::push(size_t s)
{
    void *ret;
    assert(s > 0);
    if(top + s >= size){
        if(size == 0){
            size = 256;
        }
        while(top + s >= size){
            size += size >> 1;
        }
        stack = (char *)realloc(stack, size);
    }
    ret = stack + top;
    top += s;
    return ret;
}

void* Buffer::pop(size_t s)
{
    assert(top >= s);
    top -= s;
    return stack + top;
}

void Buffer::put_char(char ch)
{
    *(char *)push(sizeof(char)) = ch;
}

void Buffer::put_string(const char* s, int len)
{
    memcpy((char *)push(sizeof(char) * len), s, len);
}

void Buffer::clear()
{
    free(stack);
    stack = nullptr;
    size = 0;
    top = 0;
}



/**********************************************************
 *                                                        *
 *                                                        *
 *                        Value                           *
 *                                                        *
 *                                                        *
 * ********************************************************/

Value::Value(const Value &v)
{
    type = v.type;
    switch(type){
        case JSON_NUMBER:
            num = new double(*(v.num));
            break;
        case JSON_STRING:
            str = new std::string(*(v.str));
            break;
        case JSON_ARRAY:
            if(v.array){
                array = new std::vector<Value>(*(v.array));
            }
            else{
                array = nullptr;
            }
            break;
        case JSON_OBJECT:
            if(v.object){
                object = new std::unordered_map<std::string, Value>(*(v.object));
            }
            else{
                object = nullptr;
            }
            break;
    }
}

Value::Value(Value &&v) noexcept
{
    /*调试*/
    
    //std::cout<<"Use Move Constructer";
    
    /* */


    type = v.type;
    switch (type)
    {
        case JSON_NULL:break;
        case JSON_FALSE:break;
        case JSON_TRUE:break;
        case JSON_NUMBER:
            num = v.num;
            v.num = nullptr;
            break;
        case JSON_STRING:
            str = v.str;
            v.str = nullptr;
            break;
        case JSON_ARRAY:
            array = v.array;
            v.array = nullptr;
            break;
        case JSON_OBJECT:
            object = v.object;
            v.object = nullptr;
            break;
    }
}

Value::Value(const double num)
{
    set_number(num);
}

Value::Value(const std::string& str)
{
    set_string(str);
}



void Value::free()
{
        switch(type){
            case JSON_NULL:
                break;
            case JSON_FALSE:
                break;
            case JSON_TRUE:
                break;
            case JSON_NUMBER:
                delete num;
                num = nullptr;
                break;
            case JSON_STRING:
                delete str;
                str = nullptr;
                break;
            case JSON_ARRAY:
                delete array;
                array = nullptr;
                break;
            case JSON_OBJECT:
                delete object;
                object = nullptr;
                break;
            default:
                break;
        }
}

void Value::set_null()
{
    free();
    type = JSON_NULL;
}
void Value::set_true()
{
    free();
    type = JSON_TRUE;
}
void Value::set_false()
{
    free();
    type = JSON_FALSE;
}
void Value::set_number(double n)
{
    free();
    type = JSON_NUMBER;
    num = new double(n);
}

void Value::set_string(const std::string &s)
{
    free();
    type = JSON_STRING;
    str = new std::string(s);
}

int Value::get_type() const
{
    return type;
}

double Value::get_number() const
{
    assert(type == JSON_NUMBER);
    return *num;
}

std::string Value::get_string() const
{
    assert(type == JSON_STRING);
    return *str;
}

int Value::get_array_size() const
{
    assert(type == JSON_ARRAY);
    if(array){
        return array->size();
    }
    else{
        return 0;
    }
}

Value* Value::get_array_element(size_t index) const
{
    assert(type == JSON_ARRAY);
    assert(index < array->size());
    return &(*array)[index];
}

int Value::get_object_size() const
{
    assert(type == JSON_OBJECT);
    if(object == nullptr){
        return 0;
    }
    else{
        return object->size();
    }
}

void Value::erase_array_element(size_t index, size_t count)
{
    assert(type == JSON_ARRAY);
    assert(index + count <= array->size());
    array->erase(array->begin() + index, array->begin() + index + count);
}

void Value::clear_array()
{
    assert(type == JSON_ARRAY);
    if(array){
        array->clear();
    }
}

void Value::insert_array_element(Value &v, size_t index)
{
    assert(type == JSON_ARRAY);
    assert(index < array->size());
    array->insert(array->begin() + index, v);
}

std::vector<Value> Value::get_array()
{
    assert(type == JSON_ARRAY);
    return *array;
}

std::unordered_map<std::string, Value> Value::get_object()
{
    assert(type == JSON_OBJECT);
    return *object;
}

Value* Value::get_object_value(const std::string &key) const
{
    assert(type == JSON_OBJECT);
    for(auto & p : *object){
        if(p.first == key){
            return &(p.second);
        }
    }
    return nullptr;
}

void Value::set_object_value(const std::string &key, Value &v)
{
    assert(type == JSON_OBJECT);
    Value *tmp = get_object_value(key);
    if(tmp){
        *tmp = v;
    }
    else{
        (*(object))[key] = v;
    }
}

void Value::remove_object_value(const std::string &key)
{
    assert(type == JSON_OBJECT);
    auto iter = object->find(key);
    object->erase(iter);
}

bool Value::find_object_value(const std::string &key) const
{
    assert(type == JSON_OBJECT);
    for(auto & p : *(object)){
        if(p.first == key){
            return true;
        }
    }
    return false;
}


Value& Value::operator=(const Value &rhs)
{   
    free();
    type = rhs.type;
    switch(type){
        case JSON_NUMBER:
            num = new double(*(rhs.num));
            break;
        case JSON_STRING:
            str = new std::string(*(rhs.str));
            break;
        case JSON_ARRAY:
            if(rhs.array){
                array = new std::vector<Value>(*(rhs.array));
            }
            break;
        case JSON_OBJECT:
            if(rhs.object){
                object = new std::unordered_map<std::string, Value>(*(rhs.object));
            }
            break;
    }
    return *this;
}

Value& Value::operator=(Value &&rhs) noexcept
{

    /*调试*/ 
    //std::cout<<"Use Move =";
    /* */

    if(this != &rhs){
        free();
        type = rhs.type;
        switch (type)
        {
            case JSON_NULL:break;
            case JSON_FALSE:break;
            case JSON_TRUE:break;
            case JSON_NUMBER:
                num = rhs.num;
                rhs.num = nullptr;
                break;
            case JSON_STRING:
                str = rhs.str;
                rhs.str = nullptr;
                break;
            case JSON_ARRAY:
                array = rhs.array;
                rhs.array = nullptr;
                break;
            case JSON_OBJECT:
                object = rhs.object;
                rhs.object = nullptr;
                break;
        }
    }
}


Value& Value::operator=(const std::string &str)
{
    set_string(str);
}

Value& Value::operator=(const double num)
{
    set_number(num);
}



Value& Value::operator[](size_t index)
{
    assert(type == JSON_ARRAY);
    assert(index < array->size());
    return (*(array))[index];
}

Value& Value::operator[](const std::string &str)
{
    assert(type == JSON_OBJECT);
    assert(find_object_value(str) == true);
    return (*(object))[str];
}

bool operator==(const Value &lhs, const Value &rhs)
{
    if(lhs.type != rhs.type){
        return false;
    }
    if(lhs.type == JSON_STRING){
        return *(lhs.str) == *(rhs.str);
    }
    else if(lhs.type == JSON_ARRAY){
        return *(lhs.array) == *(rhs.array);
    }
    else if(lhs.type == JSON_OBJECT){
        return *(lhs.object) == *(rhs.object);
    }
}

bool operator!=(const Value &lhs, const Value &rhs)
{
    return !(lhs == rhs);
}

std::ostream &operator<<(std::ostream &os, const Value &v)
{
    if(v.type == JSON_NUMBER){
        os << v.get_number();
    }
    if(v.type == JSON_STRING){
        os << v.get_string();
    }
    if(v.type == JSON_NULL){
        os << "NULL";
    }
    if(v.type == JSON_FALSE){
        os << "FALSE";
    }
    if(v.type == JSON_TRUE){
        os << "TRUE";
    }
    return os;
}

}  // namespace JsonCpp
