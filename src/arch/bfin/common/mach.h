#ifndef MACH_H
#define MACH_H
#include "types.h"
typedef struct mach
{
	bu8 (*io_read_byte) (bu32 addr);
	bu16 (*io_read_word) (bu32 addr);
	bu32 (*io_read_long) (bu32 addr);
	void (*io_write_byte) (bu32 addr, bu8 v);
	void (*io_write_word) (bu32 addr, bu16 v);
	void (*io_write_long) (bu32 addr, bu32 v);
	void (*io_do_cycle) (void);
	void (*clear_int) (bu32 irq);
	void (*set_int) (bu32 irq);
	void (*disable_int) ();
	void (*enable_int) ();
	void (*sti) (bu32 dreg);
	void (*cli) (bu32 dreg);
} mach_t;
#endif
