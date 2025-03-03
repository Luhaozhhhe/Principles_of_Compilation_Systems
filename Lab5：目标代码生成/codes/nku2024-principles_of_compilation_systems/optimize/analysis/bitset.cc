#include "bitset.h"
#include <assert.h>
#include <iostream>
//Reference: https://github.com/yuhuifishash/SysY
DynamicBitset::DynamicBitset(){
    bit_width = 0;
    bit_array_length = 0;
    bits = nullptr;
}

DynamicBitset::~DynamicBitset(){
    if (bits != nullptr){
        delete[] bits;  // 释放内存
    }
}

DynamicBitset::DynamicBitset(int temp_bit_width){
    bit_width = temp_bit_width;
    bit_array_length = (bit_width + sizeof(long) - 1) / sizeof(long);
    bits = new long[bit_array_length];

    for(int i = 0; i<bit_array_length; i++){
        bits[i] = 0;
    }
}

void DynamicBitset::remake(int bit_width){
    this->bit_width = bit_width;
    if(bits != nullptr){
        delete[] bits;
    }
    bit_array_length = (bit_width + sizeof(long) - 1) / sizeof(long);

    bits = new long[(bit_width + sizeof(long) - 1) / sizeof(long)];//重新分配bits
    for(int i = 0 ;i < (bit_width + sizeof(long) - 1) / sizeof(long) ;i++){
        bits[i] = 0;
    }
}

int DynamicBitset::count_the_number_of_one(){
    assert(bit_width);
    int num_count = 0;
    for (int i = 0; i < bit_array_length; i++) {
        num_count += __builtin_popcount(bits[i]);
    }
    return num_count;
}

void DynamicBitset::setbit(int pos, bool value){
    assert(pos < bit_width);
    int array_idx = pos / sizeof(long);
    int bit_idx = pos % sizeof(long);
    if (value) {
        bits[array_idx] |= (1 << bit_idx);
    } 
    else {
        bits[array_idx] &= ~(1 << bit_idx);
    }
}

bool DynamicBitset::getbit(int pos){
    assert(pos < bit_width);
    int array_idx = pos / sizeof(long);
    int bit_idx = pos % sizeof(long);
    bool change_bits = (bits[array_idx] >> bit_idx) & 1;
    return change_bits;
}

DynamicBitset DynamicBitset::operator&(DynamicBitset other) {
    assert(bit_width && other.bit_width);
    assert(this->bit_width == other.bit_width);
    DynamicBitset result(bit_width);
    for (int i = 0; i < bit_array_length; i++) {
        result.bits[i] = this->bits[i] & other.bits[i];
    }
    return result;
}

DynamicBitset DynamicBitset::operator|(DynamicBitset other) {
    assert(bit_width && other.bit_width);
    assert(this->bit_width == other.bit_width);
    DynamicBitset result(bit_width);
    for (int i = 0; i < bit_array_length; i++) {
        result.bits[i] = this->bits[i] | other.bits[i];
    }
    return result;
}

DynamicBitset DynamicBitset::operator^(DynamicBitset other) {
    assert(bit_width && other.bit_width);
    assert(this->bit_width == other.bit_width);
    DynamicBitset result(bit_width);
    for (int i = 0; i < bit_array_length; i++) {
        result.bits[i] = this->bits[i] ^ other.bits[i];
    }
    return result;
}

DynamicBitset DynamicBitset::operator-(DynamicBitset other) {
    assert(bit_width && other.bit_width);
    assert(this->bit_width == other.bit_width);
    DynamicBitset result(bit_width);
    for (int i = 0; i < bit_array_length; i++) {
        result.bits[i] = this->bits[i] & (this->bits[i] ^ other.bits[i]);
    }
    return result;
}

DynamicBitset DynamicBitset::operator=(DynamicBitset other) {
    assert(bit_width && other.bit_width);
    assert(this->bit_width == other.bit_width);
    DynamicBitset result(bit_width);
    for (int i = 0; i < bit_array_length; i++) {
        result.bits[i] = this->bits[i] = other.bits[i];
    }
    return result;
}

bool DynamicBitset::operator==(DynamicBitset other) {
    assert(bit_width && other.bit_width);
    assert(this->bit_width == other.bit_width);
    for (int i = 0; i < bit_array_length; i++) {
        if (this->bits[i] != other.bits[i]) {
            return false;
        }
    }
    return true;
}

bool DynamicBitset::operator!=(DynamicBitset other) {
    assert(bit_width && other.bit_width);
    assert(this->bit_width == other.bit_width);
    for (int i = 0; i < bit_array_length; i++) {
        if (this->bits[i] != other.bits[i]) {
            return true;
        }
    }
    return false;
}

DynamicBitset::DynamicBitset(const DynamicBitset &other) {
    this->bit_width = other.bit_width;
    this->bit_array_length = other.bit_array_length;
    this->bits = new long[bit_array_length];
    for (int i = 0; i < bit_array_length; i++) {
        this->bits[i] = other.bits[i];
    }
}