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

#include "video/VIDEO_textmode.h"
#include "kernel/ports.h"
#define TAB_SIZE    8

//prototypes
static void move_cursor(void);

// Globals

// The VGA framebuffer starts at 0xB8000.
uint16_t *video_memory = (uint16_t *)0xB8000;
// Stores the cursor position.
uint8_t cursor_x = 0;
uint8_t cursor_y = 0;

// Updates the hardware cursor.
static void move_cursor()
{
	// The screen is 80 characters wide...
	uint16_t cursorLocation = (uint16_t)(cursor_y * 80 + cursor_x);
	PORT_outb(0x3D4, 14);                  // Tell the VGA board we are setting the high cursor byte.
	PORT_outb(0x3D5, cursorLocation >> 8); // Send the high cursor byte.
	PORT_outb(0x3D4, 15);                  // Tell the VGA board we are setting the low cursor byte.
	PORT_outb(0x3D5, cursorLocation);      // Send the low cursor byte.
}

// Scrolls the text on the screen up by one line.
static void scroll()
{

	// Get a space character with the default colour attributes.
	uint8_t attributeByte = (0 /*black*/ << 4) | (15 /*white*/ & 0x0F);
	uint16_t blank = 0x20 /* space */ | (attributeByte << 8);

	// Row 25 is the end, this means we need to scroll up
	if (cursor_y >= 25)
	{
		// Move the current text chunk that makes up the screen
		// back in the buffer by a line
		int i;

		for (i = 0 * 80; i < 24 * 80; i++)
		{
			video_memory[i] = video_memory[i + 80];
		}

		// The last line should now be blank. Do this by writing
		// 80 spaces to it.
		for (i = 24 * 80; i < 25 * 80; i++)
		{
			video_memory[i] = blank;
		}

		// The cursor should now be on the last line.
		cursor_y = 24;
	}
}


// Writes a single character out to the screen.
void VIDEO_print_char(uint8_t c)
{
	// The background colour is black (0), the foreground is white (15).
	uint8_t backColour = 0;
	uint8_t foreColour = 15;

	// The attribute byte is made up of two nibbles - the lower being the
	// foreground colour, and the upper the background colour.
	uint8_t  attributeByte = (backColour << 4) | (foreColour & 0x0F);
	// The attribute byte is the top 8 bits of the word we have to send to the
	// VGA board.
	uint16_t attribute = attributeByte << 8;
	uint16_t *location;

	// Handle a backspace, by moving the cursor back one space
	if (c == 0x08 && cursor_x)
	{
		cursor_x--;
	}

	// Handle a tab by increasing the cursor's X, but only to a point
	// where it is divisible by 8.
	else if (c == 0x09)
	{
		cursor_x = (cursor_x + TAB_SIZE) & ~(TAB_SIZE - 1);
	}

	// Handle carriage return
	else if (c == '\r')
	{
		cursor_x = 0;
	}

	// Handle newline by moving cursor back to left and increasing the row
	else if (c == '\n')
	{
		cursor_x = 0;
		cursor_y++;
	}


	// Handle any other printable character.
	else if (c >= ' ')
	{
		location = video_memory + (cursor_y * 80 + cursor_x);
		*location = c | attribute;
		cursor_x++;
	}

	// Check if we need to insert a new line because we have reached the end
	// of the screen.
	if (cursor_x >= 80)
	{
		cursor_x = 0;
		cursor_y ++;
	}

	// Scroll the screen if needed.
	scroll();
	// Move the hardware cursor.
	move_cursor();

}

void VIDEO_print_string(char *string)
{
	while (*string)
	{
		VIDEO_print_char(*string);
		string++;
	}
}

void VIDEO_init()
{

}

void VIDEO_clear_screen(void)
{
	// Make an attribute byte for the default colours
	uint8_t attributeByte = (0 /*black*/ << 4) | (15 /*white*/ & 0x0F);
	uint16_t blank = 0x20 /* space */ | (attributeByte << 8);

	for (int i = 0; i < 80 * 25; i++)
	{
		video_memory[i] = blank;
	}

	// Move the hardware cursor back to the start.
	cursor_x = 0;
	cursor_y = 0;
	move_cursor();
}
