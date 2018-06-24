#ifndef C_EXT_H_
#define C_EXT_H_

#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif

#define STATIC_ASSERT(COND,MSG) typedef char static_assertion_##MSG[(COND)?1:-1]

//typedef unsigned char uint8_t;
//typedef unsigned short uint16_t;
//typedef unsigned int uint32_t;

STATIC_ASSERT(sizeof(uint8_t) == 1, size_of_uint8_t_must_be_1);
STATIC_ASSERT(sizeof(uint16_t) == 2, size_of_uint16_t_must_be_2);
STATIC_ASSERT(sizeof(uint32_t) == 4, size_of_uint32_t_must_be_4);

#ifdef __cplusplus
}
#endif
#endif // C_EXT_H_
