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
    CO_4 xxx = { "temp_xx", 0x0123 };
    xxx.output(json, v);

    TEST_ASSERT_EQUAL_FLOAT(65535, json["temp_xx"]);
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

int main(int argc, char *argv[]) {
    UNITY_BEGIN();
    
    RUN_TEST(test_address_defintions);
    RUN_TEST(test_conversion_int);
    RUN_TEST(test_conversion_temp);
    RUN_TEST(test_conversion_address);

    RUN_TEST(test_conversion_address_ptr);

    UNITY_END();

    return 0;
}

//#endif
//#endif