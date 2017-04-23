/* lib.c - some helper functions */

#include "types.h"

extern uint8_t keyboard_ch;

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
	
// A utility function to check whether x is numeric
uint8_t isNumericChar(uint8_t x)
{
    return (x >= '0' && x <= '9')? TRUE: FALSE;
}
  
// A simple atoi() function. If the given string contains
// any invalid character, then this function returns 0
int32_t atoi(uint8_t *str)
{
    if (*str == NULL)
       return 0;
  
    int32_t res = 0;  // Initialize result
    int32_t sign = 1;  // Initialize sign as positive
    int32_t i = 0;  // Initialize index of first digit
  
    // If number is negative, then update sign
    if (str[0] == '-')
    {
        sign = -1;
        i++;  // Also update index of first digit
    }
  
    // Iterate through all digits of input string and update result
    for (; str[i] != '\0'; ++i)
    {
        if (isNumericChar(str[i]) == FALSE)
            return 0; // You may add some lines to write error message
                      // to error stream
        res = res*10 + str[i] - '0';
    }
  
    // Return result with sign
    return sign*res;
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

uint8_t *strcpy (uint8_t *dest, uint8_t *src)
{
    uint32_t i = 0;
    
    while (src[i] != '\0')
    {
        dest[i] = src[i];
        i++;
    }
    
    dest[i] = '\0';
    
    return (dest);
}

uint8_t *strncpy (uint8_t *dest, uint8_t *src, uint32_t len)
{
    uint32_t i = 0;
    while (src[i] != '\0')
    {
        dest[i] = src[i];
        i++;
        if (i == len) break;
    }
    
    dest[i] = '\0';
    
    return (dest);
}

uint8_t *memcpy (uint8_t *dest, const uint8_t *src, uint32_t count)
{
    uint32_t i;
    
    uint8_t *src_ptr = src;
    uint8_t *dest_ptr = dest;
    

    while (count-- > 0)
    {
        *dest_ptr++ = *src_ptr++;
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

uint8_t getch (void)
{
    uint8_t ch;
    
    while (keyboard_ch == NULL)
    {
        kdelay (10);
    }
    
    ch = keyboard_ch;
    keyboard_ch = NULL;     // reset buffer, kind of hack I know ;)
    return (ch);
}


    
