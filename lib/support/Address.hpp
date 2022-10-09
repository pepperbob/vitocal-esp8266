#pragma once

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
enum class AddressType { Temp, Int };
struct Address {
    const char* name;
    const uint16_t addr;
    
    const AddressType type = AddressType::Temp;
    const uint8_t length = 2;

    void output(JsonDocument &doc, AddressValue &value) {
        int16_t temp;
        value.toInt(temp);
        doc[name] = (float)temp/10.0f;
    }
};

const Address ADDR_AU = { "temp_au", 0x0101 };
const Address ADDR_WW = { "temp_ww", 0x010D };
const Address ADDR_VL = { "temp_vl", 0x0105 };
const Address ADDR_RL = { "temp_rl", 0x0106 };
const Address ADDR_BS = { "count_bs", 0x5005, AddressType::Int, 4 };