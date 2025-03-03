#ifndef BITSET_H
#define BITSET_H
#include <assert.h>
#include <iostream>
//Reference: https://github.com/yuhuifishash/SysY
class DynamicBitset{
private:
    int bit_width;// 位集的宽度（即位集的大小，表示位的总数）
    long *bits;// 存储位的动态数组，每个long类型存储多个位
    int bit_array_length;// 存储位的数组的长度（每个long类型可以存储多个位）
       
public:
    DynamicBitset();
    ~DynamicBitset();
    DynamicBitset(int temp_bit_width);

    void remake(int bit_width);
    int count_the_number_of_one();//计算1的个数
    void setbit(int pos, bool value);
    bool getbit(int pos);

    DynamicBitset operator&(DynamicBitset);//交集
    DynamicBitset operator|(DynamicBitset);//并集
    DynamicBitset operator^(DynamicBitset);//异或
    DynamicBitset operator-(DynamicBitset);//差集
    DynamicBitset operator=(DynamicBitset);//深拷贝
    bool operator==(DynamicBitset);//深度比较是否相等
    bool operator!=(DynamicBitset);//深度比较是否不相等
    DynamicBitset(const DynamicBitset &);

};

#endif