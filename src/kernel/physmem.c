// #include <stdio.h>

#include "types.h"

uint32_t physmem_pages[PAGES];

void set_bit (uint32_t *ptr, uint16_t bit)
{
    uint32_t ret = *ptr;
    
    ret |= 1 << bit;
    
    *ptr = ret;
}

void clear_bit (uint32_t *ptr, uint16_t bit)
{
    uint32_t ret = *ptr;
    
    ret &= ~(1 << bit);
    
    *ptr = ret;
}

uint16_t get_bit (uint32_t n, uint16_t bit)
{
    uint16_t bitset;
    
    bitset = (n >> bit) & 1;
    return (bitset);
}

void pmem_init_bitmap ()
{
    uint32_t page;
    
    for (page = 0; page < PAGES; page++)
    {
        physmem_pages[page] = 0xFFFFFFFF;   /* mark as reserved */
    }
}

void pmem_set_first_page ()
{
    set_bit (&physmem_pages[0], 0);
}

uint32_t pmem_get_page_from_address (uint32_t address)
{
    uint32_t page;
    page = address / MEM_BLOCK_SIZE;

    return (page);
}
    
int16_t pmem_set_bitmap (uint32_t start, uint32_t end, uint8_t state)
{
    /* set bits in bitmap  */
    uint32_t start_p, end_p;
    int16_t start_b, end_b;
    
    uint32_t p; int16_t b;
    
    uint32_t page_start = pmem_get_page_from_address (start);
    uint32_t page_end = pmem_get_page_from_address (end);
    
    /*
    kprint ("start address: "); kprint_int (start, 10); kprint_newline ();
    kprint ("start page: "); kprint_int (page_start, 10); kprint_newline ();
    
    kprint ("end address: "); kprint_int (end, 10); kprint_newline ();
    kprint ("end page: "); kprint_int (page_end, 10); kprint_newline ();
    */
    
    
    if (page_end > MEM_BLOCKS)
    {
        return (MEM_ERR_RANGE);
    }
    
    if (page_start >= PAGE_BITS)
    {
        start_p = page_start / PAGE_BITS;
        start_b = page_start - (start_p * PAGE_BITS);
    }
    else
    {
        start_p = 0;
        start_b = page_start;
    }
    
    if (page_end >= PAGE_BITS)
    {
        end_p = page_end / PAGE_BITS;
        end_b = page_end - (end_p * PAGE_BITS);
    }
    else
    {
        end_p = 0;
        end_b = page_end;
    }
    

    
    if (end_p > start_p)
    {
        for (p = start_p; p <= end_p; p++)
        {
            if (p == end_p)
            {
                for (b = 0; b <= end_b; b++)
                {
                    if (state == ALLOCATE)
                    {
                        set_bit (&physmem_pages[p], b);
                    }
                    else
                    {
                        clear_bit (&physmem_pages[p], b);
                    }
                }
            }
            else
            {
                if (p == start_p)
                {
                    for (b = start_b; b <= PAGE_BITS; b++)
                    {
                        if (state == ALLOCATE)
                        {
                            set_bit (&physmem_pages[p], b);
                        }
                        else
                        {
                            clear_bit (&physmem_pages[p], b);
                        }
                    }
                }
                else
                {
                    for (b = 0; b < PAGE_BITS; b++)
                    {
                        if (state == ALLOCATE)
                        {
                            set_bit (&physmem_pages[p], b);
                        }
                        else
                        {
                            clear_bit (&physmem_pages[p], b);
                        }
                    }
                }
            }
        }
    }
    else
    {
        for (b = start_b; b <= end_b; b++)
        {
            if (state == ALLOCATE)
            {
                set_bit (&physmem_pages[p], b);
            }
            else
            {
                clear_bit (&physmem_pages[p], b);
            }
            
        }
    }
    return (MEM_ERR_OK);
}
                
            


int16_t pmem_get_free (size_t size, uint32_t *start, uint32_t *end)
{
    /* return pointers to free memory range */
    int8_t found_start = 0;
    
    uint32_t p, i;
    uint16_t b;
    uint32_t start_free, end_free;
    size_t mempages, mempages_free = 0;
    
    
    if (size > MEM_BLOCK_SIZE)
    {
        mempages = size / MEM_BLOCK_SIZE;
    }
    else
    {
        mempages = 1;
    }
    
    for (p = 0; p < PAGES; p++)
    {
        for (b = 0; b < PAGE_BITS; b++)
        {
            if (get_bit (physmem_pages[p], b) == FREE)
            {
                if (found_start == 0)
                {
                    found_start = 1;
                    start_free = PAGE_BITS * MEM_BLOCK_SIZE * p + b * MEM_BLOCK_SIZE;
                    mempages_free = 1;
                    
                    if (mempages == 1)
                    {
                        *start = start_free;
                        *end = start_free;
                        return (MEM_ERR_OK);
                    }
                }
                else
                {
                    mempages_free++;
                }
            }
            else
            {
                if (mempages_free >= mempages)
                {
                    end_free = (PAGE_BITS * MEM_BLOCK_SIZE * p + b * MEM_BLOCK_SIZE) - 1;
                    
                    *start = start_free;
                    *end = end_free;
                    return (MEM_ERR_OK);
                }
                else
                {
                    found_start = 0; mempages_free = 0;
                }
            }
        }
        if (mempages_free >= mempages)
        {
            end_free = (PAGE_BITS * MEM_BLOCK_SIZE * p + b * MEM_BLOCK_SIZE) - 1;
                    
            *start = start_free;
            *end = end_free;
            return (MEM_ERR_OK);
        }
                
    }
    return (MEM_ERR_NOMEM);
}

uint32_t pmem_count_free_pages (void)
{
    /* for informantion only: count the free pages */
    
    uint32_t pages = 0;
    uint32_t p, i;
    uint16_t b;
    
    for (p = 0; p < PAGES; p++)
    {
        for (b = 0; b < PAGE_BITS; b++)
        {
            if (get_bit (physmem_pages[p], b) == FREE)
            {
                pages++;
            }
        }
    }
    return (pages);
}
    
    
uint32_t kmalloc (size_t size)
{
    uint32_t start_free, end_free;
    uint32_t *ptr;
    size_t new_size;
    new_size = size + sizeof (size_t);
    
    if (pmem_get_free (new_size, &start_free, &end_free) == MEM_ERR_OK)
    {
        // kprint ("free mem found"); kprint_newline ();
        
        if (pmem_set_bitmap (start_free, end_free, ALLOCATE) == MEM_ERR_OK)
        {
            ptr = start_free;
            *ptr = new_size;    /* store size of memory */
            return (start_free + sizeof (size_t));
        }
        else
        {
            return (NULL);
        }
    }
    else
    {
        // kprint ("kmalloc: get free failed"); kprint_newline ();
        return (NULL);
    }
}

uint32_t kfree (uint32_t address)
{
    uint32_t *ptr;
    uint32_t end_mem;
    size_t size;
    
    ptr = address - sizeof (size_t);
    size = *ptr;
    
    // kprint ("kfree: deallocate "); kprint_int ((uint32_t) size, 10); kprint_newline ();
    
    end_mem = address + size;
    
    if (pmem_set_bitmap (address, end_mem, FREE) == MEM_ERR_OK)
    {
        return (NULL);
    }
    else
    {
        return (MEM_ERR_BAD_MEM);
    }
}

void pmem_show_page (uint32_t page)
{
    uint16_t b;
    
    for (b = 0; b < PAGE_BITS; b++)
    {
        if (get_bit (physmem_pages[page], b) == FREE)
        {
            kprint ("0 ");
        }
        else
        {
            kprint ("1 ");
        }
    }
    kprint_newline ();
}

    
    
/*
void main ()
{
    pmem_set_bitmap (1024 * 1024, 0x300000, ALLOCATE);
}
*/
