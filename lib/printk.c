/*
	This is free and unencumbered software released into the public domain.

	Anyone is free to copy, modify, publish, use, compile, sell, or
	distribute this software, either in source code form or as a compiled
	binary, for any purpose, commercial or non-commercial, and by any
	means.

	In jurisdictions that recognize copyright laws, the author or authors
	of this software dedicate any and all copyright interest in the
	software to the public domain. We make this dedication for the benefit
	of the public at large and to the detriment of our heirs and
	successors. We intend this dedication to be an overt act of
	relinquishment in perpetuity of all present and future rights to this
	software under copyright law.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
	MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
	IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
	OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
	ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
	OTHER DEALINGS IN THE SOFTWARE.

	For more information, please refer to <http://unlicense.org>
*/

#include <printk.h>
#include <lib/varargs.h>
#include <lib/ctype.h>
#include <lib/list.h>
#include <lib/kmalloc.h>
#include <types.h>
#include <bits.h>
#include <lib/string.h>

#define LEFT_JUSTIFY BIT(0)
#define PLUS_MANDATORY BIT(1)
#define SPACE_SIGN BIT(2)
#define PRECEEDED_WITH BIT(3)
#define LEFT_PAD_ZEROS BIT(4)
#define SIGNED	BIT(5)
#define CAPS BIT(9)
#define LENGTH_HH 0 //char
#define LENGTH_H 1 //short int
#define LENGTH_L 2 // long int
#define LENGTH_LL 3 // long long int

#define GET_LENGTH(a) BF_GET(a,6,2)
#define SET_LENGTH(a,b) BF_SET(a,b,6,2)

static char buffer[256] = {0};

typedef struct
{
	bool enabled;
	log_func func;
} logger_t;


#define MAX_LOGGERS 5

static logger_t loggers[MAX_LOGGERS] = {{0, NULL}};
static char *itoa(unsigned long long value, char *str, int base, int width, int precision, int flags)
{

	char digits[] = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz";
	char caps_digits[] = "ZYXWVUTSRQPONMLKJIHGFEDCBA9876543210123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	char *digits_ptr;
	char *ptr;
	char *low;
	char *end;
	char sign = 0;
	char fill_char;
	unsigned long long original = value;

	// Check for supported base.
	if (base < 2 || base > 36)
	{
		*str = '\0';
		return str;
	}

	ptr = str;

	digits_ptr = (flags & CAPS) ? caps_digits : digits;
	fill_char = ((flags & LEFT_PAD_ZEROS) && !(flags & LEFT_JUSTIFY)) ?  '0' : ' ';

	// Set '-', '+' or + ' ' according to flags and sign
	if (flags & SIGNED)
	{

		switch (GET_LENGTH(flags))
		{
		case LENGTH_HH:
			if ((signed char)value < 0)
			{
				sign = '-';
				value = - (signed char) value;
				goto negative;
			}

			break;

		case LENGTH_H:
			if ((signed short)value < 0)
			{
				sign = '-';
				value = - (signed short) value;
				goto negative;
			}

			break;

		case LENGTH_L:
			if ((signed long)value < 0)
			{
				sign = '-';
				value = - (signed long) value;
				goto negative;
			}

			break;

		case LENGTH_LL:
			if ((signed long long)value < 0)
			{
				sign = '-';
				value = - (signed long long) value;
				goto negative;
			}

			break;

		default:
			break;
		}

		if (flags & PLUS_MANDATORY)
		{
			sign = '+';
		}
		else if (flags & SPACE_SIGN)
		{
			sign = ' ';
		}
	}

negative:
	// Remember where the numbers start.
	low = ptr;

	// The actual conversion.
	do
	{
		// Modulo is negative for negative value. This trick makes abs() unnecessary.
		*ptr++ = digits_ptr[35 + value % base];
		value /= base;
	}
	while (value);

	if (precision > 0)
	{
		int len = ptr - low;

		if (precision > len)
		{
			precision -= len;
		}

		while (precision--)
		{
			*ptr++ = '0';
		}
	}

	if (original && (flags & PRECEEDED_WITH) && (fill_char != '0'))
	{
		if (base == 16)
		{
			if (flags & CAPS)
			{
				*ptr++ = 'X';
			}
			else
			{
				*ptr++ = 'x';
			}
		}

		*ptr++ = '0';
	}

	if ((sign) && (fill_char == ' '))
	{
		*ptr++ = sign;
		sign = 0;
	}

	if (width)
	{
		if (width <= (ptr - low))
		{
			width = 0;
		}
		else
		{
			width -= ptr - low;

			if (sign)
			{
				width--;
			}

			if ((fill_char == '0') && (original) && (flags & PRECEEDED_WITH))
			{
				width--;

				if (base == 16)
				{
					width --;
				}
			}

			if (!(flags & LEFT_JUSTIFY))
				for (int i = 0; i < width; i++)
				{
					*ptr++ = fill_char;
				}
		}
	}

	if (sign)
	{
		*ptr++ = sign;
	}

	if (original && (flags & PRECEEDED_WITH) && (fill_char == '0'))
	{
		if (base == 16)
		{
			if (flags & CAPS)
			{
				*ptr++ = 'X';
			}
			else
			{
				*ptr++ = 'x';
			}
		}

		*ptr++ = '0';
	}

	// Terminating the string.
	*ptr-- = '\0';
	end = ptr + 1;

	// Invert the numbers.
	while (low < ptr)
	{
		char tmp = *low;
		*low++ = *ptr;
		*ptr-- = tmp;
	}

	if (width && (flags & LEFT_JUSTIFY))
	{
		ptr = end;

		for (int i = 0; i < width; i++)
		{
			*ptr++ = fill_char;
		}

		*ptr-- = '\0';
		end = ptr + 1;
	}

	return end;
}

static int skip_atoi(const char **string)
{
	int i = 0;

	while (isdigit(**string))
	{
		i = i * 10 + *((*string)++) - '0';
	}

	return i;
}

int do_printk(char *buffer, const char *fmt, va_list args)
{
	char *ptr;
	char *begin = buffer;
	uint32_t flags = 0;
	int field_width;
	int field_precision;
	int base;
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

			while (1)
			{
				/* Check for flags */
				switch (*fmt)
				{

				case '-':
					flags |= LEFT_JUSTIFY;
					break;

				case '+':
					flags |= PLUS_MANDATORY;
					break;

				case ' ':
					flags |= SPACE_SIGN;
					break;

				case '#':
					flags |= PRECEEDED_WITH;
					break;

				case '0':
					flags |= LEFT_PAD_ZEROS;
					break;

				default:
					goto width;
				}

				fmt++;
			}

			/* Width flags */
width:

			field_width = 0;

			if (*fmt == '*')
			{
				field_width = va_arg(args, int);
				fmt++;
			}
			else if (isdigit(*fmt))
			{
				field_width = skip_atoi(&fmt);
			}

			/* Precision flags */
			field_precision = -1;

			if (*fmt == '.')
			{
				fmt++;

				if (*fmt == '*')
				{
					field_precision = va_arg(args, int);
					fmt++;
				}
				else if (isdigit(*fmt))
				{
					field_precision = skip_atoi(&fmt);
				}
				else
				{
					field_precision = 0;
				}
			}


			/* Length specs */

			switch (*fmt)
			{
			case 'h':
				fmt++;

				if (*fmt == 'h')
				{
					SET_LENGTH(flags, LENGTH_HH);
				}
				else
				{
					SET_LENGTH(flags, LENGTH_H);
				}

				break;

			case 'l':
				fmt++;

				if (*fmt == 'l')
				{
					SET_LENGTH(flags, LENGTH_LL);
				}
				else
				{
					SET_LENGTH(flags, LENGTH_L);
				}

				break;

			default:
				SET_LENGTH(flags, LENGTH_L);
				break;
			}

			/* Get the specifier base and set caps */
			switch (*fmt)
			{
			case 'd':
			case 'i':
			case 'u':
				base = 10;
				break;

			case 'X':
				flags |= CAPS; //fallthrough

			case 'x':
			case 'p':
				base = 16;
				break;

			case 'o':
				base = 8;
				break;

			case 'b':
				base = 2;
				break;

			default:
				break;
			}

			switch (*fmt)
			{
			case 'd':
			case 'i':
				if (flags & LENGTH_LL)
				{
					buffer = itoa(va_arg(args, long long), buffer, base, field_width,
					              field_precision, flags | SIGNED);
				}
				else
				{
					buffer = itoa(va_arg(args, int), buffer, base, field_width,
					              field_precision, flags | SIGNED);
				}

				break;

			case 'u':
			case 'o':
			case 'x':
			case 'X':
			case 'p':
			case 'b':
				if (flags & LENGTH_LL)
				{
					buffer = itoa(va_arg(args, unsigned long long), buffer, base, field_width,
					              field_precision, flags);
				}
				else
				{
					buffer = itoa(va_arg(args, unsigned int), buffer, base, field_width,
					              field_precision, flags);
				}

				break;

			case 's':
				ptr = va_arg(args, char *);

				if (!ptr)
				{
					ptr = "<NULL>";
				}

				if (field_precision == -1)
				{
					field_precision = strlen(ptr);
				}
				else if (field_precision > (int) strlen(ptr))
				{
					field_precision = strlen(ptr);    // To avoid printing more than the actual string length
				}


				if ((field_width) && (field_width - field_precision > 0) && !(flags & LEFT_JUSTIFY))
					for (int i = 0; i < field_width - field_precision; i++)
					{
						*buffer++ = ' ';
					}

				for (int i = 0; i < field_precision; i++)
				{
					(*buffer++ = *ptr++);
				}

				if ((field_width) && (field_width - field_precision > 0) && (flags & LEFT_JUSTIFY))
					for (int i = 0; i < field_width - field_precision; i++)
					{
						*buffer++ = ' ';
					}

				break;

			case 'c':
				c = (char) va_arg(args, int);

				if (!(flags & LEFT_JUSTIFY))
				{
					while (--field_width > 0)
					{
						*buffer++ = ' ';
					}
				}

				if (isascii(c))
				{
					*buffer++ = c;
				}

				while (--field_width > 0) // Left justify
				{
					*buffer++ = ' ';
				}

				break;

			case 'n': // For completness, I don't see a reason for using this
			{
				int *n_ptr = va_arg(args, int *);
				*n_ptr = buffer - begin;
			}
			break;

			case '%':
				*buffer++ = '%';
				break;

			default:
				break;
			}

			fmt++;
			flags = 0;
		}
	}

	*buffer = '\0';
	return (buffer - begin);
}


void register_logger(log_func func)
{
	int i;

	for (i = 0; i < MAX_LOGGERS; i++)
		if (!loggers[i].enabled)
		{
			break;
		}

	if (i == MAX_LOGGERS)
	{
		pr_error("No place for additional logger\r\n");
		return;
	}

	loggers[i].func = func;
	loggers[i].enabled = true;
}

int vprintk(const char *format, va_list arg)
{
	int done;
	done = do_printk(buffer, format, arg);

	for (int i = 0; i < MAX_LOGGERS; i++)
		if (loggers[i].enabled)
		{
			loggers[i].func(buffer);
		}

	return done;

}

int printk(const char *format, ...)
{
	va_list arg;
	int done;
	va_start(arg, format);
	done = do_printk(buffer, format, arg);
	va_end(arg);

	for (int i = 0; i < MAX_LOGGERS; i++)
		if (loggers[i].enabled)
		{
			loggers[i].func(buffer);
		}

	return done;
}

int sprintk(char *buf, const char *format, ...)
{
	va_list arg;
	int done;
	va_start(arg, format);
	done = do_printk(buf, format, arg);
	va_end(arg);
	return done;
}
void hex_dump(void *ptr, uint32_t len)
{
	for (uint32_t i = 0; i < len; i++)
	{
		printk("%x ", ((char *)ptr)[i]);

		if (i % 16 == 0)
		{
			printk("\r\n");
		}
	}

	printk("\r\n");
}
