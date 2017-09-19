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

#ifndef PROCESS_H_98R63AWS
#define PROCESS_H_98R63AWS

#include <types.h>
#include <mem/memory.h>
#include <lib/list.h>

typedef struct
{
	void *v_addr;
	addr_t p_addr;
	int count;
	list_t list;
} memblock_t;

typedef struct
{
	uint32_t eax;
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;
	uint32_t ebp;
	uint32_t esi;
	uint32_t edi;
	uint32_t ss;
	uint32_t esp;
	uint32_t eflags;
	uint32_t cs;
	uint32_t eip;
} __attribute__((packed)) task_registers_t;

typedef enum
{
	TASK_RUNNING = 0,
	TASK_STOPPED
} task_state_t;

typedef enum
{
	TASK_KERNEL = 0,
	TASK_USER
} task_type_t;

typedef struct __task_t
{
	uint32_t pid;
	uint32_t parent_pid;
	int32_t exit_value;
	task_type_t type;
	task_state_t state;

	addr_t pdt_phy_addr;
	void *pdt_virt_addr;

	addr_t stack_phy_addr;
	void *stack_virt_addr;

	addr_t kernel_stack_phy_addr;
	void *kernel_stack_virt_addr;
	void *kernel_stack_pointer;

	void * heap_top;
	task_registers_t regs;
	list_t mapped_memory_list;
	list_t list;
} task_t;

task_t *clone(task_t *parent);
int process_cleanup(task_t *task);
void prepare_init_task(void *physical, uint32_t count);
void switch_to_task(task_t *task);
void dump_task_state(task_t *task);

#endif /* end of include guard: PROCESS_H_98R63AWS */
