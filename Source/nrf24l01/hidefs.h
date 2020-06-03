/******************************************************************************
* File        : hidefs.h
* Author      : Lihao
* DESCRIPTION :
*
******************************* Revision History *******************************
* No.   Version   Date          Modified By   Description               
* 1     V1.0      2011-05-01    Lihao         Original Version
*******************************************************************************/
#ifndef HI_DEFS_H
#define HI_DEFS_H

#if defined __cplusplus
extern "C" {
#endif

/******************************************************************************/
/***        Include files                                                   ***/
/******************************************************************************/
//#include <stdarg.h>
//#include <stdio.h>

/******************************************************************************/
/***        Macro Definitions                                               ***/
/******************************************************************************/
#if defined(__GNUC__)
    /* The following 4 macros force the compiler to align the specified object  */
    /* on a given byte boundary.                                                */
    /* Example:                                                                 */
    /*     int  iVar  ALIGN_(16);  Force iVar to be aligned on the next         */
    /*                             16 byte boundary                             */
    #define PACK      __attribute__ ((packed))        /* align to byte boundary  */
    #define ALIGN_2   __attribute__ ((aligned (2)))   /* 16-bit boundary (2 byte)*/
    #define ALIGN_4   __attribute__ ((aligned (4)))   /* 32-bit boundary (4 byte)*/
    #define ALIGN_(x) __attribute__ (((aligned (x)))) /* arbitrary alignment     */
    /* force an unused variable not to be elided */
    #define USED      __attribute__ ((used))
    #define INLINE    extern inline
    #define ALWAYS_INLINE __attribute__((always_inline))
#elif defined(_MSC_VER)
    #define PACK
    #define USED
    #define __attribute__(x)
    #define ALWAYS_INLINE
    #define INLINE __inline
#else
    //#error "Unsupported compiler"
#endif

/* Alignment masks */
#define ALIGNMENT_MASK_4_BYTE   ((uint32)0x00000003)
#define ALIGNMENT_MASK_16_BYTE  ((uint32)0x0000000F)

/* Test for alignment on an arbitrary byte boundary - TRUE if aligned */
#define IS_ALIGNED(addr, mask)  (((((uint32)addr) & mask) ? FALSE : TRUE))

/* Bit opration */
#define BIT_SET(var,bitno)      ((var) |=  (1 << (bitno)))
#define BIT_CLR(var,bitno)      ((var) &= ~(1 << (bitno)))
#define BIT_IS_SET(var,bitno)   (var & (1 << (bitno)))
#define BIT(bitno)              (1 << (bitno))

#define U8_CLR_BITS(P, B)       ((*(uint8 *)P)  &= ~(B))
#define U8_SET_BITS(P, B)       ((*(uint8 *)P)  |=  (B))
#define U16_CLR_BITS(P, B)      ((*(uint16 *)P) &= ~(B))
#define U16_SET_BITS(P, B)      ((*(uint16 *)P) |=  (B))
#define U32_CLR_BITS(P, B)      ((*(uint32 *)P) &= ~(B))
#define U32_SET_BITS(P, B)      ((*(uint32 *)P) |=  (B))

/* Macros for obtaining maximum/minimum values */
//#define MAX(A,B)                (((A) > (B)) ? (A) : (B))
//#define MIN(A,B)                (((A) < (B)) ? (A) : (B))

/* Number of bits in quantities */
#define BITS_PER_U32            (32)
#define BITS_PER_U16            (16)
#define BITS_PER_U8             (8)
#define BITS_PER_NIBBLE         (4)

/* Masking macros */
#define U8_LOW_NIBBLE_MASK      (0x0F)
#define U8_HIGH_NIBBLE_MASK     (0xF0)
#define U16_LOW_U8_MASK         (0x00FF)
#define U16_HIGH_U8_MASK        (0xFF00)
#define U32_LOWEST_U8_MASK      (0x000000FFL)
#define U32_LOW_U8_MASK         (0x0000FF00L)
#define U32_HIGH_U8_MASK        (0x00FF0000L)
#define U32_HIGHEST_U8_MASK     (0xFF000000L)
#define U32_LOWEST_U16_MASK     (0x0000FFFFL)
#define U32_HIGHEST_U16_MASK    (0xFFFF0000L)

/* Macros for extracting uint8s from a uint16 */
/* NOTE: U16_UPPER_U8 is only safe for an unsigned U16 as >> fills with the sign bit for signed variables */
#define U16_UPPER_U8( x )    ((uint8)((x) >> BITS_PER_U8))
#define U16_LOWER_U8( x )    ((uint8)(((x) & U16_LOW_U8_MASK)))

/* Macros for extracting uint8s from a uint32 */
#define U32_HIGHEST_U8(x)    ((uint8)(((x) & U32_HIGHEST_U8_MASK) >> (BITS_PER_U16 + BITS_PER_U8)))
#define U32_HIGH_U8(x)       ((uint8)(((x) & U32_HIGH_U8_MASK) >> BITS_PER_U16))
#define U32_LOW_U8(x)        ((uint8)(((x) & U32_LOW_U8_MASK) >> BITS_PER_U8))
#define U32_LOWEST_U8(x)     ((uint8)(((x) & U32_LOWEST_U8_MASK)))

/* Macros for extracting uint16s from a uint32 */
#define U32_UPPER_U16(x)     ((uint16)(((x) & U32_HIGHEST_U16_MASK) >> (BITS_PER_U16)))
#define U32_LOWER_U16(x)     ((uint16)(((x) & U32_LOWEST_U16_MASK)))

/* B0 - LSB, B3 - MSB */
#ifdef HOST_PROCESSOR_BIG_ENDIAN  /* BIG ENDIAN DEFINITIONS */
  #define BYTE_ORDER_32(B3, B2, B1, B0)   ((B0) + (B1<<8) + (B2<<16) + (B3<<24))
  #define BYTE_ORDER_24(B2, B1, B0)       ((B0) + (B1<<8) + (B2<<16))
  #define BYTE_ORDER_16(B1, B0)           (uint16)(((B0) + (B1<<8)))
  #define BYTE_ORDER_8(B0)                (B0)
  #define BYTE_ORDER_4(B0)                (B0)
#else  /* LITTLE ENDIAN DEFINITIONS */
  #define BYTE_ORDER_32(B3, B2, B1, B0)   ((B3) + (B2<<8) + (B1<<16) + (B0<<24))
  #define BYTE_ORDER_24(B2, B1, B0)       ((B2) + (B1<<8) + (B0<<16))
  #define BYTE_ORDER_16(B1, B0)           (uint16)(((B1) + (B0<<8)))
  #define BYTE_ORDER_8(B0)                (B0)
  #define BYTE_ORDER_4(B0)                (B0)
#endif

/* Storage classes */
#if !defined PUBLIC
  #define PUBLIC
#endif

#if !defined MODULE
  #define MODULE
#endif

#if !defined PRIVATE
  #define PRIVATE static
#endif

#if !defined BANKED
  #define BANKED
#endif

/*--------------------------------------------------------------------------*/
/* Useful macro for variables that are not currently referenced             */
/* Prevents compiler warnings and should not produce any code               */
/*--------------------------------------------------------------------------*/
#define VARIABLE_INTENTIONALLY_NOT_REFERENCED(x) (x=x);
#define CONST_POINTER_INTENTIONALLY_NOT_REFERENCED(p) (*p);
#define CONST_VARIABLE_INTENTIONALLY_NOT_REFERENCED(x) (x);

/* Offset of field m in a struct s */
#if !defined(__GNUC__) && !defined(ECOS) && !defined(offsetof)
 #define offsetof(type, tag)     ((int)&((type *)0)->tag)
#endif

/******************************************************************************/
/***        Type Definitions                                                ***/
/******************************************************************************/
#ifndef __cplusplus /* microsoft specific */
    #ifndef bool    /* Seems to need this in for certain M$ builds*/
        //typedef int                     bool;     /* boolean type */
    #endif
#endif

#if !defined FALSE && !defined TRUE
  #define TRUE              (1)   /* page 207 K+R 2nd Edition */
  #define FALSE             (0)
#endif

#if !defined NULL
  #define NULL              ((void *)0)
#endif

#if !defined RNULL
  #define RNULL             ((void *)0())
#endif

typedef unsigned char       BOOL_T;     /* boolean type nothing to do with C++ */
typedef unsigned char       bool_t;     /* boolean type nothing to do with C++ */

//typedef signed char         int8;
//typedef short               int16;
//typedef long                int32;
typedef long long           int64;
typedef unsigned char       uint8;
typedef unsigned short      uint16;
//typedef unsigned long       uint32;
typedef unsigned long long  uint64;

typedef unsigned char U8;
typedef unsigned int U16;
typedef unsigned long U32;

typedef signed char S8;
typedef signed int S16;
typedef signed long S32;

typedef char*               string;

typedef volatile uint8      u8Register;
typedef volatile uint16     u16Register;
typedef volatile uint32     u32Register;

typedef union UU16
{
    U16 U16;
    S16 S16;
    U8 U8[2];
    S8 S8[2];
} UU16;

typedef union UU32
{
    U32 U32;
    S32 S32;
    UU16 UU16[2];
    U16 U16[2];
    S16 S16[2];
    U8 U8[4];
    S8 S8[4];
} UU32;

// used with UU16
//# define LSB 1
//# define MSB 0

#define SEGMENT_VARIABLE(name, vartype, locsegment) locsegment vartype name
#define VARIABLE_SEGMENT_POINTER(name, vartype, targsegment) targsegment vartype * name
#define SEGMENT_VARIABLE_SEGMENT_POINTER(name, vartype, targsegment, locsegment) targsegment vartype * locsegment name
#define LOCATED_VARIABLE(name, vartype, locsegment, addr, init) locsegment vartype name @ addr

# define SEG_GENERIC     //SEG_GENERIC only applies to pointers in Raisonance, not variables.
# define SEG_FAR   
# define SEG_DATA  
# define SEG_NEAR  
# define SEG_IDATA 
# define SEG_XDATA 
# define SEG_PDATA 
# define SEG_CODE  
# define SEG_BDATA 

#ifdef DEBUG_ASSERT
    #define debug_assert(expr) ((expr) ? (void)0 : debug_assert_failed((uint8 *)__FILE__, __LINE__))
    void debug_assert_failed(uint8* file, uint32 line);
#else
    #define debug_assert(expr) ((void)0)
#endif

/******************************************************************************/
/***        Local Function Prototypes                                       ***/
/******************************************************************************/

/******************************************************************************/
/***        Exported Variables                                              ***/
/******************************************************************************/

/******************************************************************************/
/***        Local Variables                                                 ***/
/******************************************************************************/

/******************************************************************************/
/***        Exported Functions                                              ***/
/******************************************************************************/

/******************************************************************************/
/***        Local Functions                                                 ***/
/******************************************************************************/

#if defined __cplusplus
}
#endif

#endif /* HI_DEFS_H */
/******************************************************************************/
/***        END OF FILE                                                     ***/
/******************************************************************************/
