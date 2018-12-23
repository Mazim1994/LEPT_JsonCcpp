Lept_JsonCpp V1.0 Author: Mazim

Note: this shit is inspired by Miloyip.

API： 
1.解码函数:
Json_Parse(const std::string &json, Value &v);
将json字符串中的JSON文本解码到v中。
2.生成函数:
Json_Parse(std::string &json, const Value &v);
将v中保存的Json数据转换为JSON文本并保存在json字符串中
3.输出函数:
Json_Print(std::ostream &os, std::string& json);
将生成的JSON文本进行格式化输出

Value:
每个Json值都储存为一个Value类

一 支持的Value构造方式：
Value(double n);
Value(const std::string &str);
Value(Value &v);

二 支持的Value操作:

用num赋值
eg: Value v = 1;
用string赋值
eg: Value v = "abc";
用Value赋值
eg: Value a;
Value b = a;
支持用于比较两个Value的 == 以及 !=
当Value为数组类型，支持[]按下标访问。
eg: v[1]; // 访问下表为1的元素
当Value为对象类型，支持[]运算符
eg: v["key"]; // 访问对象中键为key的值
支持<<运算符，（不能直接输出对象和数组）
三 支持基于范围的for循环访问数组和对象
for(auto & p : v.get_array());
for(auto & p : v.get_object());

四 Value类提供所有的接口:
1.int get_type();
2.void set_null();
3.void set_true();
4.void set_false();
5.void set_number(double n);
6.void set_string(const std::string &s);
7.double get_number();
8.std::string get_string();
9.std::vector get_array();
10.int get_array_size();
11.Value* get_array_element(size_t index);
12.void erase_array_element(size_t index, size_t count);
13.void clear_array();
14.void insert_array_element(Value &v, size_t index);
15.std::unordered_map<std::string, Value> get_object();
16.int get_object_size();
17.bool find_object_value(const std::string &key);
18.Value *get_object_value(const std::string &key);
19.void set_object_value(const std::string &key, Value &v);
20.void remove_object_value(const std::string &key);


Note:
2018.12.13:
    新添加了移动构造函数和移动赋值运算符
