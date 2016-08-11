#include "printk.h"
#include "lib/OS_varargs.h"
#include "lib/OS_ctype.h"
#include "video/VIDEO_textmode.h"
static char buffer[256]={0};

static char * itoa( int value, char * str, int base )
{
    char * rc;
    char * ptr;
    char * low;
    // Check for supported base.
    if ( base < 2 || base > 36 )
    {
        *str = '\0';
        return str;
    }
    rc = ptr = str;
    // Set '-' for negative decimals.
    if ( value < 0 && base == 10 )
    {
        *ptr++ = '-';
    }
    // Remember where the numbers start.
    low = ptr;
    // The actual conversion.
    do
    {
        // Modulo is negative for negative value. This trick makes abs() unnecessary.
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + value % base];
        value /= base;
    } while ( value );
    // Terminating the string.
    *ptr-- = '\0';
    // Invert the numbers.
    while ( low < ptr )
    {
        char tmp = *low;
        *low++ = *ptr;
        *ptr-- = tmp;
    }
    return ptr;
}

// very simple printf, only supports basic stuff
int do_printk(char * buffer, const char * fmt,va_list args)
{
    char * ptr;
    int len = 0;
	char c;
    while (*fmt)
        {
            if (*fmt != '%')
            {
                    *buffer++ = *fmt++;
            }
            else // Formatting in place... :)
                {
					fmt++;
                    switch (*fmt)
                    {
                        case 'd':
                        case 'i':
                        case 'u':
                            buffer = itoa(va_arg(args,int), buffer, 10);
                            break;
                        case 's':
                            ptr = va_arg(args, char *);
                            while (*ptr)
                                (*buffer++ = *ptr++);
                            break;
                        case 'c':
                            c = va_arg(args, char);
                            if (isascii(c))
                                *buffer++ = c;
                            break;
						case 'x':
						case 'X':
                            buffer = itoa(va_arg(args,int), buffer, 16);
							break;
                        default:
                            break;
                    }
                    fmt++;
					len++;
                }
        }
	*buffer='\0';
	return len;
}
int printk (const char *format, ...)
{
	va_list arg;
	int done;
	va_start (arg, format);
	done = do_printk(buffer, format, arg);
	va_end (arg);
	VIDEO_print_string(buffer);
	return done;
}
