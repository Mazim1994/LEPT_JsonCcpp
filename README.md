# LEPT_JsonCpp
Lept_JsonCpp V1.0
Author: Mazim

Note:
    this shit is inspired by Miloyip.

解码函数:
    Json_Parse(const std::string &json, Value &v);
    将json字符串中的JSON文本解码到v中。
生成函数:
    Json_Parse(std::string &json, const Value &v);
    将v中保存的Json数据转换为JSON文本并保存在json字符串中
输出函数:
    Json_Print(std::ostream &os, std::string& json);
    将生成的JSON文本进行格式化输出

Value:
    每个Json值都储存为一个Value类

一 支持的Value构造方式：
Value(double n);
Value(const std::string &str);
Value(Value &v);

二 支持的Value运算符:
1. 用num赋值
    eg: Value v = 1;     
2. 用string赋值
    eg: Value v = "abc";
3. 用Value赋值
    eg: Value a;
        Value b = a;
4. 支持用于比较两个Value的 == 以及 !=
5. 当Value为数组类型，支持[]按下标访问。
    eg: v[1];     // 访问下表为1的元素
6. 当Value为对象类型，支持[]运算符
    eg: v["key"]; // 访问对象中键为key的值
7. 支持<<运算符，（不能直接输出对象和数组）

三 支持基于范围的for循环访问数组和对象
    for(auto & p : v.get_array());
    for(auto & p : v.get_object());


四 Value类提供所有的接口:
int get_type();                                             
void set_null();
void set_true();
void set_false();
void set_number(double n);
void set_string(const std::string &s);
double get_number();
std::string get_string(); 
std::vector<Value> get_array();
int get_array_size();
Value* get_array_element(size_t index);
void erase_array_element(size_t index, size_t count);
void clear_array();
void insert_array_element(Value &v, size_t index);
std::unordered_map<std::string, Value> get_object();    
int get_object_size();
bool find_object_value(const std::string &key);
Value *get_object_value(const std::string &key);
void set_object_value(const std::string &key, Value &v);
void remove_object_value(const std::string &key);
