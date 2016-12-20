/*******************************************************************************
 * Copyright (c) 2016
 * The National University of Singapore, Xtra Group
 *
 * Author: Zeke Wang (wangzeke638 AT gmail.com)
 * 
 * Description: define the types for the test. 
 *
 * See file LICENSE.md for details.
 *******************************************************************************/
 
#ifndef TYPES_SIMD_H
#define TYPES_SIMD_H

#include    <x86intrin.h>
#include    <cstdint>
#include    <iostream>

//#define AVX2_DEBUG_ENABLE 

#define MAX_BYTES_PER_CODE 4

//#define STREAM_LOAD_ENABLE 

#define PREFETCHING_DISTANCE (1024)
#define HINT_LEVEL 0



typedef uint64_t WordUnit;
typedef uint8_t  ByteUnit;
typedef uint32_t UintUnit;

typedef __m256i AvxUnit;
typedef __m128i AvxUnit128;

#define K_NUM_WORD_BITS 64
#define K_NUM_AVX_BITS 256


	
constexpr size_t kNumWordBits = 8*sizeof(WordUnit);
constexpr size_t kNumAvxBits  = 8*sizeof(AvxUnit);

enum class ColumnType{
    kNaive,
    kByteSlicePadRight,
    kByteSlicePadLeft
};

enum class Bitwise{
    kSet,
    kAnd,
    kOr, 
	kNeg  
};

enum class Comparator{
    kEqual,
    kInequal,
    kLess,
    kGreater,
    kLessEqual,
    kGreaterEqual
};

enum class Direction{
    kLeft,
    kRight
};


//for debug use
std::ostream& operator<< (std::ostream &out, ColumnType type);
std::ostream& operator<< (std::ostream &out, Comparator comp);

/* T should be uint8/16/32/64_t */

template <typename T>
inline T FLIP(T value){
    constexpr T offset = (static_cast<T>(1) << (sizeof(T)*8 - 1));
    return static_cast<T>(value ^ offset);
}

#endif //TYPES_H
