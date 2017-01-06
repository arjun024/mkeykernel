/* lib.c - some helper functions */

#include "types.h"

void strreverse(uint8_t* begin, uint8_t* end) {
	
	uint8_t aux;
	
	while(end>begin)
	
		aux=*end, *end--=*begin, *begin++=aux;
	
}
	
void itoa(int32_t value, uint8_t* str, int32_t base) {
	
	static uint8_t num[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	
	uint8_t* wstr=str;
	
	int32_t sign;
	
	// Validate base
	
	if (base<2 || base>35){ *wstr='\0'; return; }
	

	// Take care of sign
	
	if ((sign=value) < 0) value = -value;
	

	// Conversion. Number is reversed.
	
	do *wstr++ = num[value%base]; while(value/=base);
	
	if(sign<0) *wstr++='-';
	
	*wstr='\0';
	

	// Reverse string

	
	strreverse(str,wstr-1);
}
	

void kprint_int (int32_t n, int32_t base)
{
    char str[256];
    
    itoa (n, str, base);
    kprint (str);
}

int16_t strcmp (const uint8_t * str1, const uint8_t * str2)
{
    while (*str1 == *str2)
    {
        if (*str1 == '\0' || *str2 == '\0')
        break;

        str1++;
        str2++;
    }


   if (*str1 == '\0' && *str2 == '\0')
      return 0;
   else
      return -1;
}

uint32_t strlen (const uint8_t *str)
{
    uint32_t slen = 0;
    
    while (*str != '\0')
    {
        slen++;
        str++;
    }
    
    return (slen);
}

uint8_t *memcpy (uint8_t *dest, const uint8_t *src, uint32_t count)
{
    uint32_t i;
    
    for (i = 1; i <= count; i++)
    {
        dest[i] = src[i];
    }
    
    return (dest);
}

uint8_t *memset (uint8_t *dest, uint8_t val, uint32_t count)
{
    uint32_t i;
    
    for (i = 1; i <= count; i++)
    {
        dest[i] = val;
    }
    
    return (dest);
}

uint16_t *memsetw ( uint16_t *dest, uint16_t val, uint32_t count)
{
    uint32_t i;
    
    for (i = 1; i <= count; i++)
    {
        dest[i] = val;
    }
    
    return (dest);
}




    
