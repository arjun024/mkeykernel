/* lib.c - some helper functions */

void strreverse(char* begin, char* end) {
	
	char aux;
	
	while(end>begin)
	
		aux=*end, *end--=*begin, *begin++=aux;
	
}
	
void itoa(unsigned long int value, char* str, int base) {
	
	static char num[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	
	char* wstr=str;
	
	int sign;
	
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
	

void kprint_int (long n, int base)
{
    char str[256];
    
    itoa (n, str, base);
    kprint (str);
}

int strcmp (const char * str1, const char * str2)
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

    
