#include "types.h"

uint32_t page_directory[1024] __attribute__((aligned(4096)));
uint32_t first_page_table[1024] __attribute__((aligned(4096)));

// This should go outside any function..
extern void loadPageDirectory(unsigned int*);
extern void enablePaging();

void vmem_init_page_directory (void)
{
    //set each entry to not present
    uint16_t i;
    
    for (i = 0; i < 1024; i++)
    {
        // This sets the following flags to the pages:
        //   Supervisor: Only kernel-mode can access them
        //   Write Enabled: It can be both read from and written to
        //   Not Present: The page table is not present
        page_directory[i] = 0x00000002;
    }
}
    
void vmem_init_page_table (void)
{
    // holds the physical address where we want to start mapping these pages to.
    // in this case, we want to map these pages to the very beginning of memory.
    uint16_t i;
 
    //we will fill all 1024 entries in the table, mapping 4 megabytes
    for (i = 0; i < 1024; i++)
    {
        // As the address is page aligned, it will always leave 12 bits zeroed.
        // Those bits are used by the attributes ;)
        first_page_table[i] = (i * 0x1000) | 3; // attributes: supervisor level, read/write, present.
    }
    
    // attributes: supervisor level, read/write, present
    page_directory[0] = ((unsigned int) first_page_table) | 3;
}

void vmem_paging (void)
{
    vmem_init_page_directory ();
    vmem_init_page_table ();
    
    loadPageDirectory(page_directory);
    enablePaging();
}
