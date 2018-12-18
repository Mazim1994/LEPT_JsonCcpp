/*
*
*
*       JsonCpp测试
*
* 
*/
#include "JsonCpp.h"
#include <iostream>
#include <fstream>

using namespace JsonCpp;
using namespace std;

static int test_count = 0;
static int test_pass = 0;

template<typename T1, typename T2>
inline static bool CHECK_BASE(const T1& expect, const T2& actual)
{
    test_count++;
    if(expect == actual){
        test_pass++;
        return true;
    }
    else{
        return false;
    }
}

// 内联函数无法正确显示行号，故采用宏定义的方式将其封装
#define CHECK(expect, actual)            \
    do                                   \
    {                                    \
        if (!CHECK_BASE(expect, actual)) \
            cout << "expect: " << expect << " actual: " << actual << " file: " << __FILE__ << " line: " << __LINE__ << endl;\
    }while(0)

#define CHECK_ERROR(error, json)            \
    do{                                     \
        Value v;                            \
        CHECK(error, Json_Parse(json, v));  \
        CHECK(JSON_NULL, v.get_type());     \
    }while(0)

#define CHECK_NUMBER(expect, json)            \
    do                                        \
    {                                         \
        Value v;                              \
        CHECK(PARSE_OK, Json_Parse(json, v)); \
        CHECK(JSON_NUMBER, v.get_type());     \
        CHECK(expect, v.get_number());        \
    }while(0)
#define CHECK_LITERAL(expect, json)           \
    do                                        \
    {                                         \
        Value v;                              \
        CHECK(PARSE_OK, Json_Parse(json, v)); \
        CHECK(expect, v.get_type());          \
    }while(0)
#define CHECK_STRING(expect, json)            \
    do                                        \
    {                                         \
        Value v;                              \
        CHECK(PARSE_OK, Json_Parse(json, v)); \
        CHECK(JSON_STRING, v.get_type());     \
        CHECK(expect, v.get_string());        \
    }while(0)
#define CHECK_ROUNDTRIP(json)                  \
    do                                        \
    {                                         \
        Value v;                              \
        std::string json2;                    \
        CHECK(PARSE_OK, Json_Parse(json, v)); \
        Json_Generate(json2, v);              \
        CHECK(json, json2);                   \
    }while(0)

/*********************************************/

// 测试解析NULL/FALSE/TRUE
static void test_parse_literal()
{
    CHECK_LITERAL(JSON_FALSE, "false");
    CHECK_LITERAL(JSON_NULL, "null");
    CHECK_LITERAL(JSON_TRUE, "true");
}

// 测试解析数字
static void test_parse_number()
{
    CHECK_NUMBER(0.0, "0");
    CHECK_NUMBER(0.0, "0");
    CHECK_NUMBER(0.0, "-0");
    CHECK_NUMBER(0.0, "-0.0");
    CHECK_NUMBER(1.0, "1");
    CHECK_NUMBER(-1.0, "-1");
    CHECK_NUMBER(1.5, "1.5");
    CHECK_NUMBER(-1.5, "-1.5");
    CHECK_NUMBER(3.1416, "3.1416");
    CHECK_NUMBER(1E10, "1E10");
    CHECK_NUMBER(1e10, "1e10");
    CHECK_NUMBER(1E+10, "1E+10");
    CHECK_NUMBER(1E-10, "1E-10");
    CHECK_NUMBER(-1E10, "-1E10");
    CHECK_NUMBER(-1e10, "-1e10");
    CHECK_NUMBER(-1E+10, "-1E+10");
    CHECK_NUMBER(-1E-10, "-1E-10");
    CHECK_NUMBER(1.234E+10, "1.234E+10");
    CHECK_NUMBER(1.234E-10, "1.234E-10");
    CHECK_NUMBER(0.0, "1e-10000"); 
    CHECK_NUMBER(1.0000000000000002, "1.0000000000000002");
    CHECK_NUMBER( 4.9406564584124654e-324, "4.9406564584124654e-324");
    CHECK_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    CHECK_NUMBER( 2.2250738585072009e-308, "2.2250738585072009e-308"); 
    CHECK_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
    CHECK_NUMBER( 2.2250738585072014e-308, "2.2250738585072014e-308"); 
    CHECK_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    CHECK_NUMBER( 1.7976931348623157e+308, "1.7976931348623157e+308"); 
    CHECK_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
    
}

// 测试解析越界数字
// 测试解析越界数字
static void test_parse_number_too_big()
{
    CHECK_ERROR(PARSE_NUMBER_TOO_BIG, "1e309");
    CHECK_ERROR(PARSE_NUMBER_TOO_BIG, "-1e309");
}

// 测试异常
static void test_parse_root_not_singular()
{
    CHECK_ERROR(PARSE_ROOT_NOT_SINGULAR, "null x");

    CHECK_ERROR(PARSE_ROOT_NOT_SINGULAR, "0123");
    CHECK_ERROR(PARSE_ROOT_NOT_SINGULAR, "0x0");
    CHECK_ERROR(PARSE_ROOT_NOT_SINGULAR, "0x123");
}

// 测试异常
static void test_parse_invalid_value()
{
    CHECK_ERROR(PARSE_INVALID_VALUE, "nul");
    CHECK_ERROR(PARSE_INVALID_VALUE, "?");

    /* 无效数字 */
    CHECK_ERROR(PARSE_INVALID_VALUE, "+0");
    CHECK_ERROR(PARSE_INVALID_VALUE, "+1");
    CHECK_ERROR(PARSE_INVALID_VALUE, ".123");
    CHECK_ERROR(PARSE_INVALID_VALUE, "1.");
    CHECK_ERROR(PARSE_INVALID_VALUE, "INF");
    CHECK_ERROR(PARSE_INVALID_VALUE, "inf");
    CHECK_ERROR(PARSE_INVALID_VALUE, "NAN");
    CHECK_ERROR(PARSE_INVALID_VALUE, "nan");
}

static void test_parse_expect_value()
{
    CHECK_ERROR(PARSE_EXPECT_VALUE, "");
    CHECK_ERROR(PARSE_EXPECT_VALUE, " ");
}

static void test_parse_string()
{
    CHECK_STRING("", "\"\"");
    CHECK_STRING("Hello", "\"Hello\"");
    CHECK_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
    CHECK_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");

/*  
    I don't know wtf with this test case
    It seems like when the result of encoding \\u00000 is put in the buffer,
    it will be regarded as a terminator.
    so the rest will not be put in the buffer until it is popped ????? 
    :(
    CHECK_STRING("Hello\0World", "\"Hello\\u0000World\"");
*/

    CHECK_STRING("\x24", "\"\\u0024\"");         /* Dollar sign U+0024 */
    CHECK_STRING("\xC2\xA2", "\"\\u00A2\"");     /* Cents sign U+00A2 */
    CHECK_STRING("\xE2\x82\xAC", "\"\\u20AC\""); /* Euro sign U+20AC */
    CHECK_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");  /* G clef sign U+1D11E */
    CHECK_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");  /* G clef sign U+1D11E */
}

static void test_parse_missing_quotation_mark() {
    CHECK_ERROR(PARSE_MISS_QUOTATION_MARK, "\"");
    CHECK_ERROR(PARSE_MISS_QUOTATION_MARK, "\"abc");
}

static void test_parse_invalid_string_escape() {
    CHECK_ERROR(PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
    CHECK_ERROR(PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
    CHECK_ERROR(PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
    CHECK_ERROR(PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
}

static void test_parse_invalid_string_char() {
    CHECK_ERROR(PARSE_INVALID_STRING_CHAR, "\"\x01\"");
    CHECK_ERROR(PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
}

static void test_parse_invalid_unicode_hex() {
    CHECK_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u\"");
    CHECK_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u0\"");
    CHECK_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u01\"");
    CHECK_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u012\"");
    CHECK_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u/000\"");
    CHECK_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\uG000\"");
    CHECK_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
    CHECK_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u0G00\"");
    CHECK_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
    CHECK_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u00G0\"");
    CHECK_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u000/\"");
    CHECK_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u000G\"");
    CHECK_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u 123\"");
}

static void test_parse_invalid_unicode_surrogate() {
    CHECK_ERROR(PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
    CHECK_ERROR(PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
    CHECK_ERROR(PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
    CHECK_ERROR(PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
    CHECK_ERROR(PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
}


static void test_access_number()
{
    Value v;
    v.set_string("a");
    v.set_number(1234.5);
    CHECK(1234.5, v.get_number());
}

static void test_access_string()
{
    Value v;
    v.set_string("");
    CHECK("", v.get_string());
    v.set_string("Hello");
    CHECK("Hello", v.get_string());
}

static void test_parse_array() {
    Value v;

    CHECK(PARSE_OK, Json_Parse("[ ]", v));
    CHECK(JSON_ARRAY, v.get_type());
    CHECK(0, v.get_array_size());
    v.free();

    CHECK(PARSE_OK, Json_Parse("[ null , false , true , 123 , \"abc\" ]", v));
    CHECK(JSON_ARRAY, v.get_type());
    CHECK(5, v.get_array_size());
    CHECK(JSON_NULL,   v.get_array_element(0)->get_type());
    CHECK(JSON_FALSE,  v.get_array_element(1)->get_type());
    CHECK(JSON_TRUE,   v.get_array_element(2)->get_type());
    CHECK(JSON_NUMBER, v.get_array_element(3)->get_type());
    CHECK(JSON_STRING, v.get_array_element(4)->get_type());
    CHECK(123.0, v.get_array_element(3)->get_number());
    CHECK("abc", v.get_array_element(4)->get_string());
    v.free();

    CHECK(PARSE_OK, Json_Parse("[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]", v));
    CHECK(JSON_ARRAY, v.get_type());
    CHECK(4, v.get_array_size());
    for(int i = 0; i != 4; ++i){
        Value* tmp = v.get_array_element(i);
        CHECK(JSON_ARRAY, tmp->get_type());
        CHECK(i, tmp->get_array_size());
        for(int j = 0; j < i; ++j){
            CHECK(JSON_NUMBER, tmp->get_array_element(j)->get_type());
            CHECK((double)j, tmp->get_array_element(j)->get_number());
        }
    }
}

static void test_parse_object(){
    Value v;
    size_t i;
    CHECK(PARSE_OK, Json_Parse(" { }", v));
    CHECK(JSON_OBJECT, v.get_type());
    CHECK(0, v.get_object_size());

    v.free();

    CHECK(PARSE_OK, Json_Parse(
                        " { "
                        "\"n\" : null , "
                        "\"f\" : false , "
                        "\"t\" : true , "
                        "\"i\" : 123 , "
                        "\"s\" : \"abc\", "
                        "\"a\" : [ 1, 2, 3 ],"
                        "\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3 }"
                        " } ",
                        v));
    CHECK(JSON_OBJECT, v.get_type());
    CHECK(7, v.get_object_size());

    CHECK(JSON_NULL, v.get_object_value("n")->get_type());
    CHECK(JSON_FALSE, v.get_object_value("f")->get_type());
    CHECK(JSON_TRUE, v.get_object_value("t")->get_type());
    
    CHECK(JSON_NUMBER, v.get_object_value("i")->get_type());
    CHECK(123, v.get_object_value("i")->get_number());

    CHECK(JSON_STRING,v.get_object_value("s")->get_type());
    CHECK("abc", v.get_object_value("s")->get_string());

    CHECK(JSON_ARRAY, v.get_object_value("a")->get_type());
    for(int i = 0; i != 3; ++i){
        CHECK(JSON_NUMBER, v.get_object_value("a")->get_array_element(i)->get_type());
        CHECK(i + 1.0, v.get_object_value("a")->get_array_element(i)->get_number());
    }

    CHECK(JSON_OBJECT, v.get_object_value("o")->get_type());
    CHECK(JSON_NUMBER, v.get_object_value("o")->get_object_value("1")->get_type());
    CHECK(1, v.get_object_value("o")->get_object_value("1")->get_number());
    CHECK(JSON_NUMBER, v.get_object_value("o")->get_object_value("2")->get_type());
    CHECK(2, v.get_object_value("o")->get_object_value("2")->get_number());
    CHECK(JSON_NUMBER, v.get_object_value("o")->get_object_value("3")->get_type());
    CHECK(3, v.get_object_value("o")->get_object_value("3")->get_number());
}

static void test_parse_miss_key()
{
    CHECK_ERROR(PARSE_MISS_KEY, "{:1,");
    CHECK_ERROR(PARSE_MISS_KEY, "{1:1,");
    CHECK_ERROR(PARSE_MISS_KEY, "{true:1,");
    CHECK_ERROR(PARSE_MISS_KEY, "{false:1,");
    CHECK_ERROR(PARSE_MISS_KEY, "{null:1,");
    CHECK_ERROR(PARSE_MISS_KEY, "{[]:1,");
    CHECK_ERROR(PARSE_MISS_KEY, "{{}:1,");
}

static void test_parse_miss_colon() {
    CHECK_ERROR(PARSE_MISS_COLON, "{\"a\"}");
    CHECK_ERROR(PARSE_MISS_COLON, "{\"a\",\"b\"}");
}

static void test_parse_miss_comma_or_curly_bracket() {
    CHECK_ERROR(PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1");
    CHECK_ERROR(PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1]");
    CHECK_ERROR(PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1 \"b\"");
    CHECK_ERROR(PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":{}");
}

static void test_parse_miss_comma_or_square_bracket() {
    CHECK_ERROR(PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1");
    CHECK_ERROR(PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1}");
    CHECK_ERROR(PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1 2");
    CHECK_ERROR(PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[[]");
}

static void test_access_array()
{
    Value v;
    CHECK(PARSE_OK, Json_Parse("[ null , false , true , 123 , \"abc\" ]", v));
    
    /* test erase */
    v.erase_array_element(0,3);
    CHECK(2, v.get_array_size());   

    /* test insert */
    Value tmp;
    tmp.set_string("test");
    v.insert_array_element(tmp, 0);
    CHECK(JSON_STRING, v.get_array_element(0)->get_type());
    CHECK("test", v.get_array_element(0)->get_string());
    CHECK(3, v.get_array_size()); 

}
static void test_access_object()
{
    Value v;
    CHECK(PARSE_OK, Json_Parse(
                        " { "
                        "\"n\" : null , "
                        "\"f\" : false , "
                        "\"t\" : true , "
                        "\"i\" : 123 , "
                        "\"s\" : \"abc\", "
                        "\"a\" : [ 1, 2, 3 ],"
                        "\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3 }"
                        " } ",
                        v));
    CHECK(JSON_OBJECT, v.get_type());
    CHECK(7, v.get_object_size());

    /* test find */
    CHECK(true, v.find_object_value("n"));
    
    /* test set */
    Value tmp;
    tmp.set_string("test");
    v.set_object_value("s", tmp);
    CHECK("test", v.get_object_value("s")->get_string());

    /* test remove */
    v.remove_object_value("s");
    CHECK(false, v.find_object_value("s"));
}

static void test_operator()
{
    Value v;
    CHECK(PARSE_OK, Json_Parse("[ null , false , true , 123 , \"abc\" ]", v));
    Value tmp("abc");

    /* test [] for array */
    CHECK("abc", v[4].get_string());
    
    /* test == and !=*/
    CHECK(true, (tmp == v[4]));
    tmp = "def";
    CHECK(true, (tmp != v[4]));

    /* test = num*/
    tmp = 2;
    CHECK(2, tmp.get_number());

    /* test [] for object*/
    v.free();
    CHECK(PARSE_OK, Json_Parse(
                        " { "
                        "\"n\" : null , "
                        "\"f\" : false , "
                        "\"t\" : true , "
                        "\"i\" : 123 , "
                        "\"s\" : \"abc\", "
                        "\"a\" : [ 1, 2, 3 ],"
                        "\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3 }"
                        " } ",
                        v));
    CHECK("abc", v["s"].get_string());
    

    /* test << */
    /* Value v2;
    CHECK(PARSE_OK, Json_Parse("[ null , false , true , 123 , \"abc\" ]", v2));
    for(auto & p : v2.get_array()){
        cout << p;
    } */
}

static void test_parse()
{
    test_parse_literal();
    test_parse_number();
    test_parse_string();
    test_parse_array();

    test_parse_number_too_big();
    test_parse_root_not_singular();
    test_parse_invalid_value();
    test_parse_expect_value();

    test_parse_missing_quotation_mark();
    test_parse_invalid_string_escape();
    test_parse_invalid_string_char();

    test_parse_invalid_unicode_hex();
    test_parse_invalid_unicode_surrogate();

    test_parse_object();
    test_parse_miss_key();
    test_parse_miss_colon();
    test_parse_miss_comma_or_curly_bracket();
    test_parse_miss_comma_or_square_bracket();

    test_access_number();
    test_access_string();
    test_access_array();
    test_access_object();

    test_operator();
}

static void test_stringify_number()
{
    CHECK_ROUNDTRIP("0");
    CHECK_ROUNDTRIP("-0");
    CHECK_ROUNDTRIP("1");
    CHECK_ROUNDTRIP("-1");
    CHECK_ROUNDTRIP("1.5");
    CHECK_ROUNDTRIP("-1.5");
    CHECK_ROUNDTRIP("3.25");
    CHECK_ROUNDTRIP("1e+20");
    CHECK_ROUNDTRIP("1.234e+20");
    CHECK_ROUNDTRIP("1.234e-20");

    CHECK_ROUNDTRIP("1.0000000000000002"); /* the smallest number > 1 */
    CHECK_ROUNDTRIP("4.9406564584124654e-324"); /* minimum denormal */
    CHECK_ROUNDTRIP("-4.9406564584124654e-324");
    CHECK_ROUNDTRIP("2.2250738585072009e-308");  /* Max subnormal double */
    CHECK_ROUNDTRIP("-2.2250738585072009e-308");
    CHECK_ROUNDTRIP("2.2250738585072014e-308");  /* Min normal positive double */
    CHECK_ROUNDTRIP("-2.2250738585072014e-308");
    CHECK_ROUNDTRIP("1.7976931348623157e+308");  /* Max double */
    CHECK_ROUNDTRIP("-1.7976931348623157e+308");
}

static void test_stringify_string() {
    CHECK_ROUNDTRIP("\"\"");
    CHECK_ROUNDTRIP("\"Hello\"");
    CHECK_ROUNDTRIP("\"Hello\\nWorld\"");
    CHECK_ROUNDTRIP("\"\\\" \\\\ / \\b \\f \\n \\r \\t\"");
    CHECK_ROUNDTRIP("\"Hello\\u0000World\"");
}

static void test_stringify_array() {
    CHECK_ROUNDTRIP("[]");
    CHECK_ROUNDTRIP("[null,false,true,123,\"abc\",[1,2,3]]");
}

static void test_stringify_object() {
    CHECK_ROUNDTRIP("{}");
//  CHECK_ROUNDTRIP("{\"n\":null,\"f\":false,\"t\":true,\"i\":123,\"s\":\"abc\",\"a\":[1,2,3],\"o\":{\"1\":1,\"2\":2,\"3\":3}}");
//  由于采用了unordered_map, 故生成的顺序和解析的顺序不一致，但内容相同。
}


static void test_stringify() {
    CHECK_ROUNDTRIP("null");
    CHECK_ROUNDTRIP("false");
    CHECK_ROUNDTRIP("true");
    test_stringify_number();
    test_stringify_string();
    test_stringify_array();
    test_stringify_object();
}

static void test_print()
{
    ofstream output("output.json");
    string json = "{\"configurations\":[{\"name\":\"MinGW\",\"intelliSenseMode\":\"clang-x64\",\"compilerPath\":\"C:/Program Files (x86)/LLVM/bin/gcc.exe\",\"includePath\": [\"workspaceFolder\"],\"browse\": {\"path\": [\"workspaceFolder\"],\"limitSymbolsToIncludedHeaders\": true,\"databaseFilename\": \"\"},\"cStandard\": \"c99\",\"cppStandard\": \"c++11\"}],\"version\": 4}";
    string json2 = "{\"n\":null,\"f\":false,\"t\":true,\"i\":123,\"s\":\"abc\",\"a\":[1,2,3],\"o\":{\"1\":1,\"2\":2,\"3\":3}}";
    Json_Print(output, json);
    Json_Print(cout, json2);
}

int main()
{   
    test_parse();
    test_stringify();
    test_print();
    cout << test_pass << "/" << test_count << " (" << 100 * test_pass / test_count << "%)" << endl;
    cin.get();
    return 0;
}