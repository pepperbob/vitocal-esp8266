//#ifndef ARDUINO
//#ifdef UNIT_TEST

#include <unity.h>
#include <ArduinoJson.h>
#include <vector>

#include <Address.hpp>


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

    int16_t temperature;
    v.toInt(temperature);
    TEST_ASSERT_EQUAL(temperature, -10);

    uint16_t someInt;
    v.toInt(someInt);
    TEST_ASSERT_EQUAL(someInt, 65526);
}

void test_conversion_json() {
    std::vector<uint8_t> reading;
    reading.push_back(0xF1);
    reading.push_back(0xFF);

    AddressValue v = { reading };

    StaticJsonDocument<50> json;
    
    Address y = { "temp_xx", 0x0123 };
    y.output(json, v);

    TEST_ASSERT_EQUAL_FLOAT(-1.5f, json["temp_xx"]);
}

int main(int argc, char *argv[]) {
    UNITY_BEGIN();
    
    RUN_TEST(test_address_defintions);
    RUN_TEST(test_conversion_int);
    RUN_TEST(test_conversion_json);

    UNITY_END();

    return 0;
}

//#endif
//#endif