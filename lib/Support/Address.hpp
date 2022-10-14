#pragma once

#include <ArduinoJson.h>
#include <stdint.h>

/**
 * @brief Wrapper for a value read from a address.
 */
struct AddressValue {
    const std::vector<uint8_t> val;

    template <typename IntegerType>
    IntegerType toInt(IntegerType &result) const {
        result = 0;
        for (int n = val.size()-1; n >= 0; n--) {
            result = (result << 8) + val[n];
        }
        return result;
    }
};

/**
 * @brief Defines a specific address.
 * 
 * Address has a name, a data point, a type of what is represents and the length of 
 * the expected data at this address.
 * 
 */
struct Address {

    Address(const std::string name, uint16_t addr, uint8_t length): name(name), addr(addr), length(length) { }
    virtual ~Address() { }

    const std::string name;
    const uint16_t addr;
    const uint8_t length;

    virtual void output(JsonDocument &doc, const AddressValue &value) const = 0;
};

/**
 * @brief Address variable input length; values will be transformed to uint32_t. 
 */
struct CO_4: Address {
    CO_4(const std::string name, uint16_t addr, uint8_t length): Address(name, addr, length) {}
    ~CO_4() { }

    void output(JsonDocument &doc, const AddressValue &value) const override {
        uint32_t no;
        value.toInt(no);
        doc[name] = no;
    }
};

struct Temperature: Address {
    Temperature(const std::string name, uint16_t addr): Address(name, addr, 2) { }
    ~Temperature() { }

    void output(JsonDocument &doc, const AddressValue &value) const override {
        int16_t temp;
        value.toInt(temp);
        doc[name] = (float)temp/10.0f;
    }
};

const Temperature ADDR_AU = { "temp_au", 0x0101 };
const Temperature ADDR_WW = { "temp_ww", 0x010D };
const Temperature ADDR_VL = { "temp_vl", 0x0105 };
const Temperature ADDR_RL = { "temp_rl", 0x0106 };

const CO_4 ADDR_BS = { "count_bs", 0x5005, 4 };
