//#ifndef ARDUINO
//#ifdef UNIT_TEST

#include <unity.h>
#include <ArduinoJson.h>
#include <vector>

#include <Address.hpp>
#include <Callback.hpp>

void setUp() {
    // set stuff up here
}

void tearDown() {
    // clean stuff up here
}

void test_address_defintions() {
    TEST_ASSERT_EQUAL(ADDR_AU.length, 2);
}

void test_conversion_int() {
    std::vector<uint8_t> buff;
    buff.push_back(0xF6);
    buff.push_back(0xFF);

    AddressValue v = { buff };

    int16_t signed_int;
    v.toInt(signed_int);
    TEST_ASSERT_EQUAL(-10, signed_int);

    uint16_t unsinged_int;
    v.toInt(unsinged_int);
    TEST_ASSERT_EQUAL(65526, unsinged_int);
}

void test_conversion_temp() {
    std::vector<uint8_t> reading;
    reading.push_back(0xF1);
    reading.push_back(0xFF);

    AddressValue v = { reading };

    StaticJsonDocument<50> json;
    Temperature temp = { "temp_xx", 0x0123 };
    temp.output(json, v);

    TEST_ASSERT_EQUAL_FLOAT(-1.5f, json["temp_xx"]);
}

void test_conversion_address() {
    std::vector<uint8_t> reading;
    reading.push_back(0xFF);
    reading.push_back(0xFF);

    AddressValue v = { reading };

    StaticJsonDocument<50> json;
    GenericInt4 xxx = { "temp_xx", 0x0123, 0 };
    xxx.output(json, v);

    TEST_ASSERT_EQUAL_FLOAT(65535, json["temp_xx"]);
}

void test_init_co4() {
    Address* addr = new GenericInt4("test", 0x0101, 0);

    TEST_ASSERT_EQUAL_STRING("test", addr->name.c_str());
    TEST_ASSERT_EQUAL_UINT16(257, addr->addr);
    TEST_ASSERT_EQUAL_INT(0, addr->length);

    delete addr;
}

void test_conversion_address_ptr() {
    std::vector<uint8_t> reading;
    reading.push_back(0xF1);
    reading.push_back(0xFF);

    AddressValue v = { reading };

    StaticJsonDocument<50> json;
    Temperature xxx = { "temp_xx", 0x0123 };
    Address *ptr = &xxx;

    ptr->output(json, v);

    TEST_ASSERT_EQUAL_FLOAT(-1.5f, json["temp_xx"]);
}

void test_parsemessage() {
    std::string message = "{ \"name\": \"abc\", \"addr\": \"257\" }";
    auto *addr = processMessageToAddress(message.c_str());

    TEST_ASSERT_EQUAL_STRING("abc", addr->name.c_str());
    TEST_ASSERT_EQUAL_UINT16(257, addr->addr);
    TEST_ASSERT_EQUAL_INT(4, addr->length);

    delete addr;
}

void test_parsemessage_len() {
    std::string message = "{ \"name\": \"test\", \"addr\": 257, \"len\": 2 }";
    auto *addr = processMessageToAddress(message.c_str());

    TEST_ASSERT_EQUAL_INT(2, addr->length);

    delete addr;
}

void test_parsemessage_witherr() {
    const char* message = "abc";
    
    const Address* addr = processMessageToAddress(message);
    TEST_ASSERT_NULL(addr);
}

void test_destructor() {
    Address* addr = new GenericInt4("test", 0x1234, 0);
    TEST_ASSERT_NOT_NULL(addr);

    delete addr;
}

int main(int argc, char *argv[]) {
    UNITY_BEGIN();
    
    RUN_TEST(test_address_defintions);
    RUN_TEST(test_conversion_int);
    RUN_TEST(test_conversion_temp);
    RUN_TEST(test_conversion_address);

    RUN_TEST(test_conversion_address_ptr);

    RUN_TEST(test_parsemessage);
    RUN_TEST(test_parsemessage_witherr);
    RUN_TEST(test_destructor);
    RUN_TEST(test_init_co4);
    RUN_TEST(test_parsemessage_len);

    UNITY_END();

    return 0;
}

//#endif
//#endif