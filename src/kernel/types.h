/* 32 bit executable */

typedef unsigned char           uint8_t;
typedef unsigned short int      uint16_t;
typedef unsigned int            uint32_t;
typedef unsigned long long int  uint64_t;

typedef signed char             int8_t;
typedef signed short int        int16_t;
typedef signed int              int32_t;
typedef signed long long int    int64_t;

typedef uint32_t size_t;

#define TRUE                    1
#define FALSE                   0
#define NULL                    0L

/* memory */
#define MEM_BLOCK_SIZE          4096
#define MEM_BLOCKS              1048576     /* enough for 4 GB */
#define PAGE_BITS               32

#define RESERVED                2
#define FREE                    0
#define ALLOCATED               1

#define ALLOCATE                1

#define PAGES                   MEM_BLOCKS / PAGE_BITS

#define MEM_ERR_OK              0
#define MEM_ERR_BAD_MEM         1
#define MEM_ERR_RANGE           2
#define MEM_ERR_NOMEM           3
#define MEM_ERR_DOUBLE_FREE     4

/* PIT timer Hz */
#define TIMER_FREQ              100

//gdt descriptor offsets
#define GDT_KERNEL_CODE             0x08
#define GDT_KERNEL_DATA             0x10
