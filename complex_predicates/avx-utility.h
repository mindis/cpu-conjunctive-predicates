/*******************************************************************************
 * Copyright (c) 2016
 * The National University of Singapore, Xtra Group
 *
 * Author: Zeke Wang (wangzeke638 AT gmail.com)
 * 
 * Description: define the SIMD assemable with AVX2 (m256i). 
 *
 * See file LICENSE.md for details.
 *******************************************************************************/
#ifndef AVX_UTILITY_H
#define AVX_UTILITY_H
 
#include    <cstdint>
#include    <x86intrin.h>

#define NUM_WORD_BITS 64 
#define NUM_AVX_BITS 256


// Load
#ifdef STREAM_LOAD_ENABLE

inline __m256i avx_load(void *mem_info){
    return  _mm256_stream_load_si256( (__m256i*)mem_info );
}
inline void avx_store(void *mem_info, __m256i a){
	_mm256_stream_si256 ( (__m256i *)mem_info, a);
}

#else

inline __m256i avx_load(void *mem_info){
    return _mm256_loadu_si256( (__m256i*)mem_info );
}
inline void avx_store(void *mem_info, __m256i a){
	_mm256_store_si256 ( (__m256i *)mem_info, a);
}

#endif


// Compare less
template <typename T>
inline __m256i avx_cmplt(const __m256i &a, const __m256i &b){
    switch(sizeof(T)){
        case 1:
            return _mm256_cmpgt_epi8(b, a);
        case 2:
            return _mm256_cmpgt_epi16(b, a);
        case 4:
            return _mm256_cmpgt_epi32(b, a);
        case 8:
            return _mm256_cmpgt_epi64(b, a);
    }
}

// Compare greater
template <typename T>
inline __m256i avx_cmpgt(const __m256i &a, const __m256i &b){
    switch(sizeof(T)){
        case 1:
            return _mm256_cmpgt_epi8(a, b);
        case 2:
            return _mm256_cmpgt_epi16(a, b);
        case 4:
            return _mm256_cmpgt_epi32(a, b);
        case 8:
            return _mm256_cmpgt_epi64(a, b);
    }
}

// Compare equal
template <typename T>
inline __m256i avx_cmpeq(const __m256i &a, const __m256i &b){
    switch(sizeof(T)){
        case 1:
            return _mm256_cmpeq_epi8(b, a);
        case 2:
            return _mm256_cmpeq_epi16(b, a);
        case 4:
            return _mm256_cmpeq_epi32(b, a);
        case 8:
            return _mm256_cmpeq_epi64(b, a);
    }
}

// Set1
template <typename T>
inline __m256i avx_set1(T a){
    switch(sizeof(T)){
        case 1:
            return _mm256_set1_epi8(static_cast<int8_t>(a));
        case 2:
            return _mm256_set1_epi16(static_cast<int16_t>(a));
        case 4:
            return _mm256_set1_epi32(static_cast<int32_t>(a));
        case 8:
            return _mm256_set1_epi64x(static_cast<int64_t>(a));
    }
}

// Zero
inline __m256i avx_zero(){
    return _mm256_setzero_si256();
}

// All ones
inline __m256i avx_ones(){
    return _mm256_set1_epi64x(-1ULL);
}

// Bitwise AND
inline __m256i avx_and(const __m256i &a, const __m256i &b){
    return _mm256_and_si256(a, b);
}

// Bitwise OR
inline __m256i avx_or(const __m256i &a, const __m256i &b){
    return _mm256_or_si256(a, b);
}

// Bitwise XOR
inline __m256i avx_xor(const __m256i &a, const __m256i &b){
    return _mm256_xor_si256(a, b);
}

// Bitwise NOT
inline __m256i avx_not(const __m256i &a){
    return _mm256_xor_si256(a, avx_ones());
}

// Bitwise (NOT a) AND b
inline __m256i avx_andnot(const __m256i &a, const __m256i &b){
    return _mm256_andnot_si256(a, b);
}

// Test is zero
inline bool avx_iszero(const __m256i &a){
    return _mm256_testz_si256(a, a);
}
 
//Scan Kernel2 --- Optimized on Scan Kernel
//to remove unnecessary equal comparison for last byte slice
//add mask_from_column: 1 means that one code is right for early stop.  
template <Comparator CMP, bool LAST_BYTE>
inline void computeKernel(const AvxUnit &byteslice1,
                          const AvxUnit &byteslice2,
                                AvxUnit &mask_less,
                                AvxUnit &mask_greater,
                                AvxUnit &mask_equal) 
{
    //internal ByteSlice --- not last BS                                                        
    if(!LAST_BYTE){ 
        switch(CMP){
            case Comparator::kEqual:
                mask_equal       = avx_and(mask_equal, avx_cmpeq<ByteUnit>(byteslice1, byteslice2));
                break;	
            case Comparator::kInequal:
                mask_equal       = avx_and(mask_equal, avx_cmpeq<ByteUnit>(byteslice1, byteslice2));
                break;
            case Comparator::kLess:
            case Comparator::kLessEqual:
                mask_less        = avx_or(mask_less, avx_and(mask_equal, avx_cmplt<ByteUnit>(byteslice1, byteslice2)));
                mask_greater     = avx_or(mask_greater, avx_and(mask_equal, avx_cmpgt<ByteUnit>(byteslice1, byteslice2)));				
                mask_equal       = avx_and(mask_equal, avx_cmpeq<ByteUnit>(byteslice1, byteslice2));
                break;
            case Comparator::kGreater:
            case Comparator::kGreaterEqual:
                mask_less        = avx_or(mask_less, avx_and(mask_equal, avx_cmplt<ByteUnit>(byteslice1, byteslice2)));
                mask_greater     = avx_or(mask_greater, avx_and(mask_equal, avx_cmpgt<ByteUnit>(byteslice1, byteslice2)));				
                mask_equal       = avx_and(mask_equal, avx_cmpeq<ByteUnit>(byteslice1, byteslice2));
                break;
        }
    }
    //it is the last byte..... last BS: no need to compute mask_equal for some comparisons
    else //if(BYTE_ID == kNumBytesPerCode - 1)   
    {
		switch(CMP){
            case Comparator::kEqual:
            case Comparator::kInequal:
                mask_equal   = avx_and(mask_equal, avx_cmpeq<ByteUnit>(byteslice1, byteslice2));
                break;
            case Comparator::kLessEqual:
                mask_less    = avx_or(mask_less, avx_and(mask_equal, avx_cmplt<ByteUnit>(byteslice1, byteslice2)));
                mask_equal   = avx_and(mask_equal, avx_cmpeq<ByteUnit>(byteslice1, byteslice2));
                break;
            case Comparator::kLess:
                mask_less    = avx_or(mask_less, avx_and(mask_equal, avx_cmplt<ByteUnit>(byteslice1, byteslice2)));
                break;
            case Comparator::kGreaterEqual:
                mask_greater = avx_or(mask_greater, avx_and(mask_equal, avx_cmpgt<ByteUnit>(byteslice1, byteslice2)));
                mask_equal   = avx_and(mask_equal, avx_cmpeq<ByteUnit>(byteslice1, byteslice2));
                break;
            case Comparator::kGreater:
                mask_greater = avx_or(mask_greater, avx_and(mask_equal, avx_cmpgt<ByteUnit>(byteslice1, byteslice2)));
                break;
        }
    }
}

 
template <Comparator CMP>
inline void computeKernelWithMask_FIRST(const AvxUnit &byteslice1,
                                  const AvxUnit &byteslice2,
                                        AvxUnit &mask_less,
                                        AvxUnit &mask_greater,
                                        AvxUnit &mask_equal,
                                        AvxUnit &mask_success,
						                AvxUnit &mask_fail) 
{
        switch(CMP){
            case Comparator::kEqual:
                //mask_less        = avx_cmplt<ByteUnit>(byteslice1, byteslice2);
                //mask_greater     = avx_cmpgt<ByteUnit>(byteslice1, byteslice2);				
                mask_equal       = avx_cmpeq<ByteUnit>(byteslice1, byteslice2);
				mask_fail        = avx_not(mask_equal); //avx_or(mask_less, mask_greater); //
				mask_success     = avx_zero();
                break;	
            case Comparator::kInequal:
                //mask_less        = avx_cmplt<ByteUnit>(byteslice1, byteslice2);
                //mask_greater     = avx_cmpgt<ByteUnit>(byteslice1, byteslice2);				
                mask_equal       = avx_cmpeq<ByteUnit>(byteslice1, byteslice2);
				mask_success     = avx_not(mask_equal); //avx_or(mask_less, mask_greater);	//			
				mask_fail        = avx_zero();
                break;
            case Comparator::kLess:
            case Comparator::kLessEqual:
                mask_less        = avx_cmplt<ByteUnit>(byteslice1, byteslice2);
                mask_greater     = avx_cmpgt<ByteUnit>(byteslice1, byteslice2);				
                mask_equal       = avx_cmpeq<ByteUnit>(byteslice1, byteslice2);
                mask_fail        = mask_greater;
				mask_success     = mask_less;	
                break;
            case Comparator::kGreater:
            case Comparator::kGreaterEqual:
                mask_less        = avx_cmplt<ByteUnit>(byteslice1, byteslice2);
                mask_greater     = avx_cmpgt<ByteUnit>(byteslice1, byteslice2);				
                mask_equal       = avx_cmpeq<ByteUnit>(byteslice1, byteslice2);
                mask_fail        = mask_less;
				mask_success     = mask_greater;	
                break;
        }
} 




template <Comparator CMP>
inline void computeKernelWithMask(const AvxUnit &byteslice1,
                                  const AvxUnit &byteslice2,
                                        AvxUnit &mask_less,
                                        AvxUnit &mask_greater,
                                        AvxUnit &mask_equal,
                                        AvxUnit &mask_success,
						                AvxUnit &mask_fail) 
{
//	if (!avx_iszero(mask_equal))
        switch(CMP){
            case Comparator::kEqual:
                //mask_less        = avx_or(mask_less, avx_and(mask_equal, avx_cmplt<ByteUnit>(byteslice1, byteslice2)));
                //mask_greater     = avx_or(mask_greater, avx_and(mask_equal, avx_cmpgt<ByteUnit>(byteslice1, byteslice2)));				
                mask_equal       = avx_and(mask_equal, avx_cmpeq<ByteUnit>(byteslice1, byteslice2));
				mask_fail        = avx_not(mask_equal); //avx_or(mask_less, mask_greater); //
				mask_success     = avx_zero();
                break;	
            case Comparator::kInequal:
                //mask_less        = avx_or(mask_less, avx_and(mask_equal, avx_cmplt<ByteUnit>(byteslice1, byteslice2)));
                //mask_greater     = avx_or(mask_greater, avx_and(mask_equal, avx_cmpgt<ByteUnit>(byteslice1, byteslice2)));				
                mask_equal       = avx_and(mask_equal, avx_cmpeq<ByteUnit>(byteslice1, byteslice2));
				mask_success     = avx_not(mask_equal); //	avx_or(mask_less, mask_greater);	//		
				mask_fail        = avx_zero();		
                break;
            case Comparator::kLess:
            case Comparator::kLessEqual:
                mask_less        = avx_or(mask_less, avx_and(mask_equal, avx_cmplt<ByteUnit>(byteslice1, byteslice2)));
                mask_greater     = avx_or(mask_greater, avx_and(mask_equal, avx_cmpgt<ByteUnit>(byteslice1, byteslice2)));				
                mask_equal       = avx_and(mask_equal, avx_cmpeq<ByteUnit>(byteslice1, byteslice2));
                mask_fail        = mask_greater;
				mask_success     = mask_less;
                break;
            case Comparator::kGreater:
            case Comparator::kGreaterEqual:
                mask_less        = avx_or(mask_less, avx_and(mask_equal, avx_cmplt<ByteUnit>(byteslice1, byteslice2)));
                mask_greater     = avx_or(mask_greater, avx_and(mask_equal, avx_cmpgt<ByteUnit>(byteslice1, byteslice2)));				
                mask_equal       = avx_and(mask_equal, avx_cmpeq<ByteUnit>(byteslice1, byteslice2));
                mask_fail        = mask_less;
				mask_success     = mask_greater;	
                break;
        }
} 

template <Bitwise OP>
inline void computeForEarlyStop(const AvxUnit &input_equal_1,
                                const AvxUnit &input_success_1,
                                const AvxUnit &input_fail_1,
							    const AvxUnit &input_equal_2,
                                const AvxUnit &input_success_2,
                                const AvxUnit &input_fail_2,
								      AvxUnit &output_equal,
                                      AvxUnit &output_success,
                                      AvxUnit &output_fail) 
{
        switch(OP){
            case Bitwise::kAnd:            //not (a) & b
                output_equal       = avx_or( avx_andnot(input_fail_1, input_equal_2), avx_andnot(input_fail_2, input_equal_1) ); 
				                     //avx_and( avx_not(avx_or(input_fail_1, input_fail_2)), avx_or(input_equal_1, input_equal_2)); 
				                     //avx_or( avx_and(input_success_1, input_equal_2), avx_and(input_success_2, input_equal_1) ); 
				output_success     = avx_and( input_success_1, input_success_2);
				output_fail        = avx_or ( input_fail_1, input_fail_2);
                break;	
            case Bitwise::kOr:
                output_equal       =  avx_or( avx_andnot(input_success_1, input_equal_2), avx_andnot(input_success_2, input_equal_1) ); 
				                    //avx_or( avx_and(input_fail_1, input_equal_2), avx_and(input_fail_2, input_equal_1) );
				                    //avx_and( avx_not(avx_or(input_success_1, input_success_2)), avx_or(input_equal_1, input_equal_2));
				output_success     = avx_or ( input_success_1, input_success_2);
				output_fail        = avx_and( input_fail_1, input_fail_2);
                break;
        }
}

template <Comparator CMP1, Comparator CMP2, Bitwise OP>
inline void computeFinalMask(const AvxUnit &input_equal_1,
                             const AvxUnit &input_greater_1,
                             const AvxUnit &input_less_1,
							 const AvxUnit &input_equal_2,
                             const AvxUnit &input_greater_2,
                             const AvxUnit &input_less_2,
							       AvxUnit &output_mmask) 
{
	AvxUnit output_mmask_1, output_mmask_2;
    switch(CMP1){
        case Comparator::kEqual:
            output_mmask_1       = input_equal_1;
            break;	
        case Comparator::kInequal:
            output_mmask_1       = avx_not(input_equal_1);
            break;
        case Comparator::kLess:
            output_mmask_1       = input_less_1;
            break;
        case Comparator::kLessEqual:
            output_mmask_1       = avx_or(input_less_1, input_equal_1);
            break;
        case Comparator::kGreater:
            output_mmask_1       = input_greater_1;
            break;
		case Comparator::kGreaterEqual:
             output_mmask_1       = avx_or(input_greater_1, input_equal_1);
            break;
    }	
    switch(CMP2){
        case Comparator::kEqual:
            output_mmask_2       = input_equal_2;
            break;	
        case Comparator::kInequal:
            output_mmask_2       = avx_not(input_equal_2);
            break;
        case Comparator::kLess:
            output_mmask_2       = input_less_2;
            break;
        case Comparator::kLessEqual:
            output_mmask_2       = avx_or(input_less_2, input_equal_2);
            break;
        case Comparator::kGreater:
            output_mmask_2       = input_greater_2;
            break;
		case Comparator::kGreaterEqual:
             output_mmask_2       = avx_or(input_greater_2, input_equal_2);
            break;
    }		
    switch(OP){
        case Bitwise::kAnd:
            output_mmask       = avx_and( output_mmask_1, output_mmask_2);
            break;	
        case Bitwise::kOr:
            output_mmask       = avx_or( output_mmask_1, output_mmask_2);
            break;
    }
}


template <Comparator CMP>
inline void computeMask_one_predicate(const AvxUnit &input_equal,
                                      const AvxUnit &input_greater,
                                      const AvxUnit &input_less,
							                AvxUnit &output_mmask) 
{
    switch(CMP){
        case Comparator::kEqual:
            output_mmask       = input_equal;
            break;	
        case Comparator::kInequal:
            output_mmask       = avx_not(input_equal);
            break;
        case Comparator::kLess:
            output_mmask       = input_less;
            break;
        case Comparator::kLessEqual:
            output_mmask       = avx_or(input_less, input_equal);
            break;
        case Comparator::kGreater:
            output_mmask       = input_greater;
            break;
		case Comparator::kGreaterEqual:
             output_mmask       = avx_or(input_greater, input_equal);
            break;
    }	
}

template <Bitwise OP>
inline void computeConjunctivePredicates(const AvxUnit &input_mmask_1,
                                         const AvxUnit &input_mmask_2,
							                   AvxUnit &output_mmask) 
{
    switch(OP){
        case Bitwise::kAnd:
            output_mmask       = avx_and( input_mmask_1, input_mmask_2);
            break;	
        case Bitwise::kOr:
            output_mmask       = avx_or( input_mmask_1, input_mmask_2);
            break;
/*        case Bitwise::kNeg:
            output_mmask       = avx_not( input_mmask_1 );
            break;	*/		
    }
}
//after aggregating informatio from all the predicates, 
//then scatter the updated equation to all 

#endif  //AVX_UTILITY_H
