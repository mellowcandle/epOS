#include "printk.h"
#include "lib/OS_varargs.h"
#include "lib/OS_ctype.h"
#include "video/VIDEO_textmode.h"
#include "OS_types.h"
static char buffer[256]={0};

static char * itoa( int value, char * str, int base )
{
    char * rc;
    char * ptr;
    char * low;
	char * end;
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
	end = ptr+1;
    // Invert the numbers.
    while ( low < ptr )
    {
        char tmp = *low;
        *low++ = *ptr;
        *ptr-- = tmp;
    }
    return end;
}

static char * uitoa( unsigned int value, char * str, int base )
{
    char * rc;
    char * ptr;
    char * low;
	char * end;
    // Check for supported base.
    if ( base < 2 || base > 36 )
    {
        *str = '\0';
        return str;
    }
    rc = ptr = str;
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
	end = ptr+1;
    // Invert the numbers.
    while ( low < ptr )
    {
        char tmp = *low;
        *low++ = *ptr;
        *ptr-- = tmp;
    }
    return end;
}

static char * lluitoa( unsigned long long value, char * str, int base )
{
    char * rc;
    char * ptr;
    char * low;
	char * end;
    // Check for supported base.
    if ( base < 2 || base > 36 )
    {
        *str = '\0';
        return str;
    }
    rc = ptr = str;
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
	end = ptr+1;
    // Invert the numbers.
    while ( low < ptr )
    {
        char tmp = *low;
        *low++ = *ptr;
        *ptr-- = tmp;
    }
    return end;
}
// very simple printf, only supports basic stuff
static int do_printk(char * buffer, const char * fmt,va_list args)
{
    char * ptr;
    int len = 0;

	bool ll = false;
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
					if ((*fmt == 'l') && (*(fmt+1) == 'l'))
					{
						ll = true;
						fmt+=2;
					}

					switch (*fmt)
                    {
                        case 'd':
					    case 'i':
								buffer = itoa(va_arg(args,int), buffer, 10);
								break;
                        case 'u':
								if (!ll)
									buffer = uitoa(va_arg(args,int), buffer, 10);
								else
									buffer = lluitoa(va_arg(args,long long), buffer, 10);
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
							if (!ll)
								buffer = itoa(va_arg(args,int), buffer, 16);
							else
								buffer = lluitoa(va_arg(args,long long), buffer, 16);
							break;
                        default:
                            break;
                    }
                    fmt++;
					len++;
					ll = false;
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
	serial_write_string(buffer);
//	VIDEO_print_string(buffer);
	return done;
}
