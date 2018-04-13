/*
	skyeye_mach_lpc2300.c - define machine lpc2300 for skyeye
	Copyright (C) 2003 Skyeye Develop Group
	for help please send mail to <skyeye-developer@lists.gro.clinux.org>

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 
*/

/*
 * 03/03/2018 	New file skyeye_mach_lpc2300.c based on skyeye_mach_lpc2210.c
 * 04/08/2018 	Bug fix: PINMODE read/write results in PINSEL read/write
 *            	Add UART 2/3
 *            	Fix wrong access to some registers
 * 04/09/2018 	Bug fix: Some program goes into infinite loop when UART polling.
 * */

#ifdef __WIN32__
#include <windows.h>   /* for GetSystemTimeAsFileTime() */
#endif

#include "armdefs.h"
#include "lpc.h"
//#include "skyeye-ne2k.h"

/* 2007-01-18 added by Anthony Lee : for new uart device frame */
#include "skyeye_uart.h"


//teawater add for arm2x86 2005.03.18-------------------------------------------
//make gcc-3.4 can compile this file
ARMword lpc2300_uart_read(ARMul_State *state, ARMword addr,int i);
void lpc2300_io_write_word(ARMul_State *state, ARMword addr, ARMword data);
//AJ2D--------------------------------------------------------------------------

#define BITMSK(n)	(1<<(n))
#define TC_DIVISOR	(50)	/* Set your BogoMips here :) may move elsewhere*/

#define DEBUG 0
#if DEBUG
#define DBG_PRINT(a...) fprintf(stderr,##a)
#else
#define DBG_PRINT(a...)
#endif

typedef struct timer
{
	ARMword	ir;
	ARMword tcr;
	ARMword tc;
	ARMword pr;
	ARMword pc;
	ARMword mcr;
	ARMword mr[4];
	ARMword ccr;
	ARMword cr0;
	ARMword cr1;
	ARMword cr2;
	ARMword cr3;
	ARMword emr;
	ARMword ctcr;
} lpc2300_timer_t;

typedef struct uart
{
	ARMword	rbr;
	ARMword thr;
	ARMword ier;
	ARMword iir;
	ARMword fcr;
	ARMword lcr;
	ARMword lsr;
	ARMword msr;
	ARMword scr;
	ARMword dll;
	ARMword dlm;
	char t_fifo[16];
	char r_fifo[16];
} lpc2300_uart_t;

typedef struct pll
{
	ARMword	con;
	ARMword cfg;
	ARMword stat;
	ARMword feed;
} lpc2300_pll_t;

typedef struct vic
{
	ARMword IRQStatus;
	ARMword FIQStatus;
	ARMword RawIntr;
	ARMword IntSelect;
	ARMword IntEnable;
	ARMword IntEnClr;
	ARMword SoftInt;
	ARMword SoftIntClear;
	ARMword Protection;
	ARMword Vect_Addr;
	ARMword VICSWPrioMask;
	ARMword VectAddr[32];
	ARMword VectPrio[32];  /* VICVectCntl */
	signed int	last_irq_no;
} lpc2300_vic_t;

typedef struct lpc2300_io {
	ARMword			syscon;			/* System control */
	ARMword			sysflg;			/* System status flags */
	lpc2300_pll_t	pll;
	lpc2300_timer_t	timer[4];
	lpc2300_vic_t	vic;
	ARMword			pconp;			/* Peripheral Power Control */
	ARMword			pinsel[11];		/*Pin Select Register*/
	ARMword			pinmode[10];	/*Pin Mode Select Register*/
	ARMword			FIO[160];
	ARMword			bcfg[4];		/*BCFG Extend Mem control*/
		int			tc_prescale;
	lpc2300_uart_t	uart[4];		/* Receive data register */
	ARMword			mmcr;			/*Memory mapping control register*/
	ARMword			vibdiv;

	ARMword			pclksel0;
	ARMword			pclksel1;

	/*real time regs*/
	ARMword		sec;
	ARMword		min;
	ARMword		hour;
	ARMword		dom;
	ARMword		dow;
	ARMword		doy;
	ARMword		month;
	ARMword		year;
	ARMword		preint;
	ARMword		prefrac;
	ARMword		ccr;

	/*mam accelerator module*/
	ARMword		mamcr;
	ARMword		mamtim;
#ifdef __WIN32__
	__int64		lasttimertick;
#endif

} lpc2300_io_t;

static lpc2300_io_t lpc2300_io;

#define io lpc2300_io
#define IRQTIMER(i)		(1<<(i<2 ? i+4 : i+24))
#define IRQUART(i)		(1<<(i<2 ? i+6 : i+26))

static void lpc2300_update_int(ARMul_State *state)
{
	u32 irq_no, irq = 0;
	int irq_prio, i;

	irq = io.vic.RawIntr & io.vic.IntEnable ;
	io.vic.IRQStatus = irq & ~io.vic.IntSelect;
	io.vic.FIQStatus = irq & io.vic.IntSelect;

	irq_no = -1;
	irq_prio = 16;
/*	for (i = 0 ; i < 32 ; i++) { // Full interrupt source */
	for (i = 4 ; i <= 29 ; i++) { // implemented source only
		if (i==8) i=26;		/* skip IRQ #8-#25 (not implemented) */
		if ((io.vic.IRQStatus & BITMSK(i)) && (irq_prio > io.vic.VectPrio[i])) {
			irq_no = i;
		}
	}
	io.vic.last_irq_no = irq_no;
	if (irq_no >= 0) {
		io.vic.Vect_Addr = io.vic.VectAddr[irq_no];
	}

	state->NirqSig = io.vic.IRQStatus ? LOW:HIGH; 
	state->NfiqSig = io.vic.FIQStatus ? LOW:HIGH;

#if 0
	fprintf(stderr, "\n** lpc2300_update_int **\t#%ld\n",state->NumInstrs);
	fprintf(stderr, "CPSR  =%08x  RawIntr=%08x IntEnable=%08x PC=%08x\n",
			state->Cpsr, io.vic.RawIntr, io.vic.IntEnable, state->Reg[15]);
	fprintf(stderr, "IRQStat=%08x FIQStat=%08x VectAddr =%08x\n\n",
			io.vic.IRQStatus, io.vic.FIQStatus, io.vic.Vect_Addr);
	fflush(stderr);
#endif
}
static void lpc2300_io_reset(ARMul_State *state)
{
	int i,j;
#ifdef __WIN32__
	__int64 wintime;
#endif
	for (i=0 ; i<4 ; i++) {
		io.timer[i].pr = 0;
		io.timer[i].ir = 0;
		io.timer[i].mr[0] = 0;
		io.timer[i].mr[1] = 0;
		io.timer[i].mr[2] = 0;
		io.timer[i].mr[3] = 0;
		io.timer[i].mcr = 0;
		io.timer[i].tc = 0;
		io.timer[i].pc = 0;
		io.timer[i].tcr = 0;
		io.timer[i].ctcr = 0;
	}
	io.pll.stat |= 1<<10;   /*PLL state register should be 1<<10 when hardware ready*/

	io.vic.IRQStatus = 0;
	io.vic.FIQStatus = 0;
	io.vic.RawIntr = 0;
	io.vic.IntSelect = 0;
	io.vic.IntEnable = 0;
	io.vic.SoftInt = 0;
	io.vic.Protection = 0;
	io.vic.Vect_Addr = 0;
	io.vic.VICSWPrioMask = 0xFFFF;
	for (i=0; i<32; i++) io.vic.VectAddr[i] = 0;
	for (i=0; i<32; i++) io.vic.VectPrio[i] = 15; /* Vect Priority=lowest*/

	for (i=0 ; i<4 ; i++) {
		io.uart[i].fcr = 0;
		io.uart[i].lcr = 0;
		io.uart[i].scr = 0;
		io.uart[i].ier = 0;
		io.uart[i].dll = 0x01;
		io.uart[i].dlm = 0;
		io.uart[i].lsr = 0x60;
		io.uart[i].iir = 0x01;
	}

	/* PINSELn : 0xE002C000 + 4*n */
	for (i=0; i<10; i++) io.pinsel[i] = 0;
	io.pinsel[10] = (0 << 3);  /* ETM << 3 */
	/* PINMODEn : 0xE002C040 + 4*n */
	for (i=0; i<10; i++) io.pinmode[i] = 0;

	io.bcfg[0] = 0x0000fbef;
	io.bcfg[1] = 0x2000fbef;
	io.bcfg[2] = 0x1000fbef;
	io.bcfg[3] = 0x0000fbef;

	io.pclksel0 = 0;
	io.pclksel1 = 0;
	io.vibdiv  = 0;
	io.pconp = 0x850ffe;
	io.vic.last_irq_no = -1;

	/* FIOnDIR:0,FIOnMASK:10,FIOnPIN:14,FIOnSET:18,FIOnCLR:1c */
    for (i=0; i<160; i++) io.FIO[i] = 0x0;

	state->NirqSig = HIGH;  /* disable IRQ */
	state->NfiqSig = HIGH;  /* disable FIQ */
//	state->Cpsr = 0x93;
#ifdef __WIN32__
	GetSystemTimeAsFileTime((FILETIME*)&wintime);
    io.lasttimertick = wintime;
#endif
}

#define UART_INT_CYCLE (128)

/*lpc2300 io_do_cycle*/
void lpc2300_io_do_cycle(ARMul_State *state)
{
	static int uartcnt = UART_INT_CYCLE;
	ARMword t,v,ntc;
	int i,n;
	struct timeval tv;
	unsigned char buf;

#if defined(__WIN32__) && 1		/* accelerate timer */
	__int64 tns;
	GetSystemTimeAsFileTime((FILETIME*)&tns);
    t = 1 + (15*(ARMword)(tns - io.lasttimertick))/10;	/* speed up TC step */
	io.lasttimertick = tns;
	for (i=0 ; i<4 && t>1 ; i++) {	/* adjust TC step in order to match MRn */
		if( (i==0) && !(io.pconp & (1<<1)) ) continue;
		if( (i==1) && !(io.pconp & (1<<2)) ) continue;
		if( (i==2) && !(io.pconp & (1<<22)) ) continue;
		if( (i==3) && !(io.pconp & (1<<23)) ) continue;
		v = io.timer[i].tc;			/* current TC */
		for (n=0 ; n<4 && t>1 ; n++) {
			if (((io.timer[i].mcr >> 3*n) & 0x7) &&	/* MRn is enabled */
				 (t > (ARMword)(io.timer[i].mr[n]-v))) { /* step > MRn - TC */
				t = (ARMword)(io.timer[i].mr[n]-v);	/* step = MRn - TC */
			}
		}
	}
#else
	t = 1;
#endif
	/* TIMER 0/1/2/3 */
	for (i=0 ; i<4 ; i++) {
		if( (i==0) && !(io.pconp & (1<<1)) ) continue;
		if( (i==1) && !(io.pconp & (1<<2)) ) continue;
		if( (i==2) && !(io.pconp & (1<<22)) ) continue;
		if( (i==3) && !(io.pconp & (1<<23)) ) continue;
		if (io.timer[i].tcr & 0x2) {
			io.timer[i].pc = 0;
			io.timer[i].pr = 0;
		} else if (io.timer[i].tcr & 0x1) {
			if (!(io.timer[i].ctcr & 0x3)) io.timer[i].pc++;
			if (!(io.vic.RawIntr & IRQTIMER(i)) && (io.timer[i].pc >= io.timer[i].pr+1)) {
				io.timer[i].tc += t;
				io.timer[i].pc = 0;
				ntc = io.timer[i].tc;
				for (n=0; n<4; n++) {
					if(ntc == io.timer[i].mr[n]) {	// MRn match
						v = io.timer[i].mcr >> 3*n;
						if (v & 0x4) io.timer[i].tcr &= ~1;	// stop timer
						if (v & 0x2) io.timer[i].tc = 0;	// reset TC
						if (v & 0x1) {						// gen interrupt
						/* T0MR2-3 and T1MR3 don't generate VIC interrupt */
						/* See manual UM10211 Section 6.6 */
							if (io.vic.IntEnable & IRQTIMER(i)) {
								io.timer[i].ir |= 1<<n;	// MRn interrupt flag
								io.vic.RawIntr |= IRQTIMER(i);
							}
							lpc2300_update_int(state);
						}
					}
				}
			}
		} /*End of else if (io.timer[i].tcr & 0x1) */
	}


	/* UART 0/1/2/3 */
	for (i = 0; i < 4; i++) {
		if( (i==0) && !(io.pconp & (1<<3)) ) continue;
		if( (i==1) && !(io.pconp & (1<<4)) ) continue;
		if( (i==2) && !(io.pconp & (1<<24)) ) continue;
		if( (i==3) && !(io.pconp & (1<<25)) ) continue;
		if((io.vic.IntEnable & IRQUART(i)) && !(io.vic.RawIntr & IRQUART(i)) && (io.uart[i].iir & 0x1)) {		/* UART Interrupt */
			if ((uartcnt == i) && (io.uart[i].ier & 0x1)) {	/* Rx Interrupt */
				tv.tv_sec = 0;
				tv.tv_usec = 0;
				if(skyeye_uart_read(i, &buf, 1, &tv, NULL) > 0)
				{
//					fprintf(stderr, "SKYEYE:uart%d(int) get input is %c IntEnable=%08x\n",i,buf,io.vic.IntEnable);fflush(stderr);
					io.uart[i].rbr = buf;
					io.uart[i].lsr |= 0x1;
					io.uart[i].iir = (io.uart[i].iir & ~0xf) | 0x4;
					io.vic.RawIntr |= IRQUART(i);
				}
				lpc2300_update_int(state);
			} else if ((uartcnt == i+UART_INT_CYCLE/2) && (io.uart[i].ier & 0x2)) {		/* Tx Interrupt */
				io.uart[i].lsr |= 0x60;
				io.uart[i].iir = (io.uart[i].iir & ~0xf) | 0x2;
				io.vic.RawIntr |= IRQUART(i);
				lpc2300_update_int(state);
			}
		} //else if(!(io.vic.IntEnable & IRQUART(i)) && !(io.uart[i].lsr & 0x1)) {		/* UART Polling */
//			tv.tv_sec = 0;
//			tv.tv_usec = 0;
//			if(skyeye_uart_read(i, &buf, 1, &tv, NULL) > 0) {
//				fprintf(stderr, "SKYEYE:uart%d(pol) get input is %c IntEnable=%08x\n",i,buf,io.vic.IntEnable);fflush(stderr);
//				io.uart[i].rbr = buf;
//				io.uart[i].lsr |= 0x1;
//			}
//		}
	}

	if (uartcnt == 0) {
		uartcnt = UART_INT_CYCLE;
	} else {
		uartcnt--;
	}
}


ARMword
lpc2300_uart_read(ARMul_State *state, ARMword addr,int i)
{
	ARMword data;
	struct timeval tv;
	unsigned char buf;

	DBG_PRINT("lpc2300_uart_read,addr=%x\n",addr);
	switch ((addr & 0xfff) >> 2) {
	case 0x0: // DLL or RbR
		if (io.uart[i].lcr & 0x80) {	/* DLL if DLAB=1*/
			data = io.uart[i].dll;
		} else {						/* RbR if DLAB=0 */
			io.uart[i].lsr &= ~0x1;
			io.vic.RawIntr &= ~IRQUART(i); /* clear interrupt */
			if ((io.uart[i].iir & 0x6) == 0x4) io.uart[i].iir = (io.uart[i].iir & ~0xe) | 0x1; /* reset interrupt if IIR[3:0]==*100 */
			lpc2300_update_int(state);
			data = io.uart[i].rbr;
		}
		break;

	case 0x1: // DLM or IER
		if (io.uart[i].lcr & 0x80) {	/* DLM if DLAB=1*/
			data = io.uart[i].dlm;
		} else {						/* IER if DLAB=0 */
			data = io.uart[i].ier;
		}
		break;
	case 0x2: // iir
		data = io.uart[i].iir;
		io.vic.RawIntr &= ~IRQUART(i);
		if ((io.uart[i].iir & 0xf) == 0x2) io.uart[i].iir = (io.uart[i].iir & ~0xe) | 0x1; /* reset interrupt if IIR[3:0]=0010 */
		lpc2300_update_int(state);
		break;
	case 0x3: // LCR
		data = io.uart[i].lcr;
		break;
	case 0x5: // LSR
		if(!(io.uart[i].ier & 0x1) && !(io.uart[i].lsr & 0x1)) {
			tv.tv_sec = 0;
			tv.tv_usec = 0;
			if(skyeye_uart_read(i, &buf, 1, &tv, NULL) > 0) {
				io.uart[i].rbr = buf;
				io.uart[i].lsr |= 0x1;
			}
		}
		io.vic.RawIntr &= ~IRQUART(i);
		if ((io.uart[i].iir & 0xf) == 0x6) io.uart[i].iir = (io.uart[i].iir & ~0xe) | 0x1; /* reset interrupt if IIR[3:0]=0110 */
		lpc2300_update_int(state);
		data = io.uart[i].lsr;
		break;
	case 0x6: // MSR
	    data = io.uart[i].msr;
		io.vic.RawIntr &= ~IRQUART(i);
		if ((io.uart[i].iir & 0xf) == 0) io.uart[i].iir = (io.uart[i].iir & ~0xe) | 0x1; /* reset interrupt if IIR[3:0]=0000 */
		lpc2300_update_int(state);
		break;
	case 0x7: // SCR
		data = io.uart[i].scr;
		break;

	default:
		data = 0;
		fprintf(stderr,"WARNING: Reading UART%d address %08x is not implemented.\n",i,addr);
		break;
	}

	return(data);
}


void
lpc2300_uart_write(ARMul_State *state, ARMword addr, ARMword data,int i)
{
	static ARMword tx_buf = 0;

	DBG_PRINT("uart_write(0x%x, 0x%x)\n", (addr & 0xfff) >> 2, data);
	switch ((addr & 0xfff) >> 2) {
		case 0x0: // DLL or THR
			if (io.uart[i].lcr & 0x80) {	/* DLL if DLAB=1*/
				io.uart[i].dll = data;
			} else {						/* THR if DLAB=0 */
				unsigned char c = data & 0xff;

				skyeye_uart_write(i, &c, 1, NULL);

				io.uart[i].lsr |= 0x60; /* indicate U1THR and U1TSR are empty */
				io.vic.RawIntr &= ~IRQUART(i);
				if ((io.uart[i].iir & 0xf) == 0x2) io.uart[i].iir = (io.uart[i].iir & ~0xe) | 0x1; /* reset interrupt if IIR[3:0]=0010 */
				lpc2300_update_int(state);
			}
			break;
		case 0x1: // DLM or IER
			if (io.uart[i].lcr & 0x80) {	/* DLL if DLAB=1*/
				io.uart[i].dlm = data;
			} else {						/* IER if DLAB=0 */
				io.uart[i].ier = data;
			}
			break;
		case 0x2: //FCR
			io.uart[i].fcr = data;
			break;
		case 0x3: // LCR
			io.uart[i].lcr = data;
			break;
		case 0x7: // SCR
			io.uart[i].scr = data;
			break;
		default:
			fprintf(stderr,"WARNING: Writing UART%d address %08x (data=0x%x) is not implemented.\n",i,addr,data);
			break;
	}
}


ARMword
lpc2300_timer_read(ARMul_State *state, ARMword addr,int i)
{
	ARMword data;
	switch (addr & 0xfe) {
		case 0x00:
			data = io.timer[i].ir;
			break;
		case 0x04:
			data = io.timer[i].tcr;
			break;
		case 0x08:
			data = io.timer[i].tc;
			break;
		case 0x0c:
			data = io.timer[i].pr;
			break;
		case 0x10:
			data = io.timer[i].pc;
			break;
		case 0x14:
			data = io.timer[i].mcr;
			break;
		case 0x18:
			data = io.timer[i].mr[0];
			break;
		case 0x1c:
			data = io.timer[i].mr[1];
			break;
		case 0x20:
			data = io.timer[i].mr[2];
			break;
		case 0x24:
			data = io.timer[i].mr[3];
			break;
		case 0x70:
			data = io.timer[i].ctcr;
			break;
		default:
			data = 0;
			fprintf(stderr,"WARNING: Reading TIMER%d address %08x is not implemented.\n",i,addr);
			break;
	}

    return data;
}


void
lpc2300_timer_write(ARMul_State *state, ARMword addr, ARMword data,int i)
{
	switch (addr & 0xfe) {
		case 0x00:
			if(data & io.timer[i].ir & 0x3f) {
				io.vic.RawIntr &= ~IRQTIMER(i);	/* clear Timer interrupt */
				io.timer[i].ir = 0;				/* clear int bit */
			}
			lpc2300_update_int(state);
			break;
		case 0x04:
			io.timer[i].tcr = data;
			if (data & 0x02) {
				io.timer[i].tc = 0;
				io.timer[i].pc = 0;
			}
			break;
		case 0x08:
			io.timer[i].tc = data;
			break;
		case 0x0c:
			io.timer[i].pr = data;
			break;
		case 0x10:
			io.timer[i].pc = data;
			break;
		case 0x14:
			io.timer[i].mcr = data;
			break;
		case 0x18:
			io.timer[i].mr[0] = data;
			break;
		case 0x1c:
			io.timer[i].mr[1] = data;
			break;
		case 0x20:
			io.timer[i].mr[2] = data;
			break;
		case 0x24:
			io.timer[i].mr[3] = data;
			break;
		case 0x70:
			io.timer[i].ctcr = data;
			break;
		default:
			fprintf(stderr,"WARNING: Writing TIMER%d address %08x (data=0x%x) is not implemented.\n",i,addr,data);
			break;
	}
}


ARMword lpc2300_io_read_word(ARMul_State *state, ARMword addr)
{
	ARMword data = -1;
	static ARMword current_ivr = 0; /* mega hack,  2.0 needs this */
	int i;
	ARMword dataimr = 0;
	ARMword ofs;

	DBG_PRINT("io_read(0x%08x)\n", addr);
	switch (addr) {
	case 0xe01fc0c4: /* PCONP */
		data = io.pconp;
		break;
	case 0xfffff000: /* ISR */
		data = io.vic.IRQStatus;
		DBG_PRINT("read ISR=%d\n", data);
		break;
	case 0xfffff004: /* interrupt status register */
		data = io.vic.FIQStatus;
		DBG_PRINT("SKYEYE:read ISR=%x,%x\n", data, io.vic.FIQStatus);
		break;
	case 0xfffff008: /* IMR */
		data = io.vic.RawIntr;
		break;
	case 0xfffff00c: /* CORE interrupt status register */
		data = io.vic.IntSelect;
		break;
	case 0xfffff010: /* IER */
		data = io.vic.IntEnable;
		DBG_PRINT("read IER=%x,after update IntEnable=%x\n", data,io.vic.IntEnable);
		break;
	case 0xfffff014: /* Int Enable Clr Reg */
		data = io.vic.IntEnClr;
		lpc2300_update_int(state);
		break;
	case 0xfffff024: /* Software Priority Mask Reg */
		data = io.vic.VICSWPrioMask ;
		DBG_PRINT("read VICSWPrioMask\n");
		break;
	case 0xffffff00: /* VICVectAddr */
		data = io.vic.Vect_Addr ;
		break;

	/*pll*/
	case 0xe01fc080:
		data = io.pll.con;
		break;
	case 0xe01fc084:
		data = io.pll.cfg;
		break;
	case 0xe01fc088:
		data = io.pll.stat|(0x7<<24); /*skyeye should aways return a pll ready*/;
        DBG_PRINT("read PLLSTAT=0x%08x\n", data);
		break;
	case 0xe01fc08c:
		data = io.pll.feed;

	/*Extend Mem control*/
	case 0xFFE00000:
		data = io.bcfg[0];
		break;
	case 0xFFE00004:
		data = io.bcfg[1];
		break;
	case 0xFFE00008:
		data = io.bcfg[2];
		break;
	case 0xFFE0000c:
		data = io.bcfg[3];
		break;

	/*Real timer*/
	case 0xe0024080:
		data = io.preint;
		break;
	case 0xe0024084:
		data = io.prefrac;
		break;
	case 0xe0024008:
		data = io.ccr;
		break;
	case 0xe0024020:
		data = io.sec;
		break;
	case 0xe0024024:
		data = io.min;
		break;
	case 0xe0024028:
		data = io.hour;
		break;
	case 0xe002402c:
		data = io.dom;
		break;
	case 0xe0024030:
		data = io.dow;
		break;
	case 0xe0024034:
		data = io.doy;
		break;
	case 0xe0024038:
		data = io.month;
		break;
	case 0xe002403c:
		data = io.year;
		break;

	/*Mem accelerator regs*/
	case 0xe01fc000:
		data = io.mamcr;
		break;
	case 0xe01fc004:
		data = io.mamtim;
		break;

	/*System Control and Status regs (SCS)*/
	case 0xe01fc1a0:
		data = io.syscon;
		break;
	/*PCLKSEL0*/
	case 0xe01fc1a8:
		data = io.pclksel0;
		break;
	/*PCLKSEL1*/
	case 0xe01fc1ac:
		data = io.pclksel1;
		break;

	default:
		/*UART*/
		if (addr >=0xe000c000 && addr <= 0xe000c030) {
			data = lpc2300_uart_read(state, addr,0);
			break;
		}
		if (addr >=0xe0010000 && addr <= 0xe0010030) {
			data = lpc2300_uart_read(state, addr,1);
			break;
		}
		if (addr >=0xe0078000 && addr <= 0xe0078030) {
			data = lpc2300_uart_read(state, addr,2);
			break;
		}
		if (addr >=0xe007c000 && addr <= 0xe007c030) {
			data = lpc2300_uart_read(state, addr,3);
			break;
		}
		/*TIMER*/
		if (addr >=0xe0004000 && addr <= 0xe0004070) {
			data = lpc2300_timer_read(state, addr,0);
			break;
		}
		if (addr >=0xe0008000 && addr <= 0xe0008070) {
			data = lpc2300_timer_read(state, addr,1);
			break;
		}
		if (addr >=0xe0070000 && addr <= 0xe0070070) {
			data = lpc2300_timer_read(state, addr,2);
			break;
		}
		if (addr >=0xe0074000 && addr <= 0xe0074070) {
			data = lpc2300_timer_read(state, addr,3);
			break;
		}
		/*VICVectAddr*/
		if(addr-0xfffff100 <=0x7c && addr-0xfffff100 >=0){
			data = io.vic.VectAddr[(addr-0xfffff100)/4] ;
			break;
		}
		if(addr-0xfffff200 <=0x7c && addr-0xfffff200>=0){
			data = io.vic.VectPrio[(addr-0xfffff200)/4] ;
			break;
		}
		/* GPIO/FIO */
		if( (addr-0x3fffc000 <0xa0 && addr-0x3fffc000>=0) ||
			(addr-0xe0028000 <0x20 && addr-0xe0028000>=0)) {
			if (addr >= 0xe002801c) {
				ofs = addr - 0xe002801c + 0x3c;
			} else if (addr >= 0xe0028018) {
				ofs = addr - 0xe0028018 + 0x20;
			} else if (addr >= 0xe0028010) {
				ofs = addr - 0xe0028010 + 0x34;
			} else if (addr >= 0xe002800c) {
				ofs = addr - 0xe002800c + 0x1c;
			} else if (addr >= 0xe0028008) {
				ofs = addr - 0xe0028008 + 0x00;
			} else if (addr >= 0xe0028000) {
				ofs = addr - 0xe0028000 + 0x14;
			} else {
				ofs = addr - 0x3fffc000 ;
			}
			data = io.FIO[ofs] ;
			/* GPIO input pin always returns HIGH */
//			if ((ofs & 0x1f)>=0x14 && (ofs & 0x1f)<0x18) {
//				data |= ~io.FIO[ofs-0x14] ;
//			}
			break;
		}
		/*Pin Select Control*/
		if(addr-0xe002c000 <=0x28 && addr-0xe002c000>=0){
			data = io.pinsel[(addr-0xe002c000)/4] ;
			break;
		}
		/*Pin Mode Select Control*/
		if(addr-0xe002c040 <=0x24 && addr-0xe002c040>=0){
			data = io.pinmode[(addr-0xe002c040)/4] ;
			break;
		}
#if 1
		fprintf(stderr,"WARNING:Reading address 0x%08x is not implemented.\n", addr);
		/*fprintf(stderr,"NumInstr %llu, io_read_word unknown addr(0x%08x) = 0x%08x\n", state->NumInstrs, addr, data);*/
		SKYEYE_OUTREGS(stderr);
		//ARMul_Debug(state, 0, 0);
#else
		data = 0;
#endif
		break;
	}
	return data; 
}

ARMword lpc2300_io_read_byte(ARMul_State *state, ARMword addr)
{
			return lpc2300_io_read_word(state,addr);
//			SKYEYE_OUTREGS(stderr);
			//exit(-1);

}

ARMword lpc2300_io_read_halfword(ARMul_State *state, ARMword addr)
{
		return lpc2300_io_read_word(state,addr);
		//SKYEYE_OUTREGS(stderr);
		//exit(-1);
}




void lpc2300_io_write_byte(ARMul_State *state, ARMword addr, ARMword data)
{

     lpc2300_io_write_word(state,addr,data);
	   //SKYEYE_OUTREGS(stderr);
	   //exit(-1);
         
}

void lpc2300_io_write_halfword(ARMul_State *state, ARMword addr, ARMword data)
{
	lpc2300_io_write_word(state,addr,data);
	//SKYEYE_OUTREGS(stderr);
	//exit(-1);
}

void lpc2300_io_write_word(ARMul_State *state, ARMword addr, ARMword data)
{
  	int i, mask, nIRQNum, nHighestIRQ;
	ARMword ofs;

	/*
  	 * The lpc2300 system registers
  	 */


	DBG_PRINT("io_write(0x%08x, 0x%08x)\n", addr,data);
	switch (addr) {
	case 0xe01fc0c4: /* PCONP */
		io.pconp = data;
		break;
	case 0xfffff000: /* ISR */
		DBG_PRINT("SKYEYE:can not write  ISR,it is RO,=%d\n", data);
		break;
	case 0xfffff004: /* interrupt status register */
		//io.vic.FIQStatus = data ;
//		DBG_PRINT("read ISR=%x,%x\n", data, io.intsr);
		DBG_PRINT("can not write  FIQStatus,it is RO,=%d\n", data);
		break;
	case 0xfffff008: /* IMR */
		//io.vic.RawIntr = data;
		DBG_PRINT("can not write  RawIntr,it is RO,=%d\n", data);
		break;
	case 0xfffff00c: /* CORE interrupt status register */
		io.vic.IntSelect = data;
		break;
	case 0xfffff010: /* IER */
		io.vic.IntEnable |= data;
		io.vic.IntEnClr &= ~data;
		lpc2300_update_int(state);
//		data = unfix_int(io.intmr);
		DBG_PRINT("write IER=%x,after update IntEnable=%x\n", data,io.vic.IntEnable);
		break;
	case 0xfffff014: /* IECR */
		io.vic.IntEnClr = data;
		io.vic.IntEnable &= ~data;
		break;

	case 0xfffff018: /* SIR */
		io.vic.SoftInt |= data;
		io.vic.SoftIntClear &= ~data;
/*		lpc2100_update_int(state); */
		break;
	case 0xfffff01c: /* SICR */
		io.vic.SoftIntClear = data;
		io.vic.SoftInt &= ~data;
		break;
	case 0xfffff020: /* PER */
		io.vic.Protection = data;
		break;
	case 0xffffff00: /* VAR */
//		io.vic.Vect_Addr = data;
		//rmk by linxz, write VAR with any data will clear current int states
		//FIQ interrupt
		//FIXME:clear all bits of FIQStatus?
		if ( io.vic.FIQStatus )
		{
			io.vic.FIQStatus = 0;
			break;
		}
		if (io.vic.last_irq_no >= 0) {
			io.vic.IRQStatus &= ~BITMSK(io.vic.last_irq_no);
			io.vic.RawIntr &= ~BITMSK(io.vic.last_irq_no);
			switch (io.vic.last_irq_no) {
			  case 6: /* Reset UART0 IIR Bit */
				io.uart[0].iir = (io.uart[0].iir & ~0xf) | 0x1;
				break;
			  case 7: /* Reset UART1 IIR Bit */
				io.uart[1].iir = (io.uart[1].iir & ~0xf) | 0x1;
				break;
			  default:
				break;
			}
			io.vic.last_irq_no = -1;
			lpc2300_update_int(state);
		}
		break;
	case 0xfffff024: /* DVAR */
		io.vic.VICSWPrioMask = data;
		break;

	/*pll*/
	case 0xe01fc080:
		io.pll.con = data;
		break;
	case 0xe01fc084:
		io.pll.cfg = data;
		io.pll.stat= (data & 0x00ffffffL) | 0x07000000L;
		break;
	case 0xe01fc088:
		/* io.pll.stat = data; // read only */
		break;
	case 0xe01fc08c:
		io.pll.feed = data;
		break;

	/*memory map control*/
	case 0xe01fc040:
		io.mmcr = data;
		break;

	/*Extend Mem control*/
	case 0xFFE00000:
		io.bcfg[0] = data;
		break;
	case 0xFFE00004:
		io.bcfg[1] = data;
		break;
	case 0xFFE00008:
		io.bcfg[2] = data;
		break;
	case 0xFFE0000c:
		io.bcfg[3] = data;
		break;

	/*Real timer*/
	case 0xe0024008:
		io.ccr = data;
		break;
	case 0xe0024080:
		io.preint = data;
		break;
	case 0xe0024084:
		io.prefrac = data;
		break;
	case 0xe0024020:
		io.sec = data;
		break;
	case 0xe0024024:
		io.min = data;
		break;
	case 0xe0024028:
		io.hour = data;
		break;
	case 0xe002402c:
		io.dom = data;
		break;
	case 0xe0024030:
		io.dow = data;
		break;
	case 0xe0024034:
		io.doy = data;
		break;
	case 0xe0024038:
		io.month = data;
		break;
	case 0xe002403c:
		io.year = data;
		break;

	/*Mem accelerator regs*/
	case 0xe01fc000:
		io.mamcr = data;
		break;
	case 0xe01fc004:
		io.mamtim = data;
		break;

	/*System Control and Status regs (SCS)*/
	case 0xe01fc1a0:
		io.syscon = data | ((data & 0x20) << 1);
		break;
	/*PCLKSEL0*/
	case 0xe01fc1a8:
		io.pclksel0 = data;
		break;
	/*PCLKSEL1*/
	case 0xe01fc1ac:
		io.pclksel1 = data;
		break;

	default:
		/*UART*/
		if (addr >=0xe000c000 && addr <= 0xe000c030) {
			lpc2300_uart_write(state, addr, data,0);
			break;
		}
		if (addr >=0xe0010000 && addr <= 0xe0010030) {
			lpc2300_uart_write(state, addr, data,1);
			break;
		}
		if (addr >=0xe0078000 && addr <= 0xe0078030) {
			lpc2300_uart_write(state, addr, data,2);
			break;
		}
		if (addr >=0xe007c000 && addr <= 0xe007c030) {
			lpc2300_uart_write(state, addr, data,3);
			break;
		}
		/*TIMER*/
		if (addr >=0xe0004000 && addr <= 0xe0004070) {
			lpc2300_timer_write(state, addr, data,0);
			break;
		}
		if (addr >=0xe0008000 && addr <= 0xe0008070) {
			lpc2300_timer_write(state, addr, data,1);
			break;
		}
		if (addr >=0xe0070000 && addr <= 0xe0070070) {
			lpc2300_timer_write(state, addr, data,2);
			break;
		}
		if (addr >=0xe0074000 && addr <= 0xe0074070) {
			lpc2300_timer_write(state, addr, data,3);
			break;
		}
		/*VICVectAddr*/
		if(addr-0xfffff100 <=0x7c && addr-0xfffff100 >=0){
			io.vic.VectAddr[(addr-0xfffff100)/4] = data;
			break;
		}
		if(addr-0xfffff200 <=0x7c && addr-0xfffff200>=0){
			io.vic.VectPrio[(addr-0xfffff200)/4] = data;
			break;
		}
		/* GPIO/FIO */
		if( (addr-0x3fffc000 <0xa0 && addr-0x3fffc000>=0) ||
			(addr-0xe0028000 <0x20 && addr-0xe0028000>=0)) {
			if (addr >= 0xe002801c) {
				ofs = addr - 0xe002801c + 0x3c;
			} else if (addr >= 0xe0028018) {
				ofs = addr - 0xe0028018 + 0x20;
			} else if (addr >= 0xe0028010) {
				ofs = addr - 0xe0028010 + 0x34;
			} else if (addr >= 0xe002800c) {
				ofs = addr - 0xe002800c + 0x1c;
			} else if (addr >= 0xe0028008) {
				ofs = addr - 0xe0028008 + 0x00;
			} else if (addr >= 0xe0028000) {
				ofs = addr - 0xe0028000 + 0x14;
			} else {
				ofs = addr - 0x3fffc000 ;
			}
			if ((ofs & 0x1f)>=0x14 && (ofs & 0x1f)<0x18) {
				io.FIO[ofs] = (data & ~io.FIO[ofs-4]) | (io.FIO[ofs] & io.FIO[ofs-4]);
			} else {
				io.FIO[ofs] = data;
			}
#if 1		/* Show LED state on CQ-NXP-ARM */
			if (ofs == 0x34) {
				if ((data & 0x00040000) && (io.FIO[0x20] & 0x00040000)) {
					fprintf(stderr, "\nCQ-FRK-NXP-ARM: P1[18] LED Off @ %lld", state->NumInstrs);
				} else {
					fprintf(stderr, "\nCQ-FRK-NXP-ARM: P1[18] LED On  @ %lld", state->NumInstrs);
				}
			}
#endif
			break;
		}
		/*Pin Select Control*/
		if(addr-0xe002c000 <=0x28 && addr-0xe002c000>=0){
			io.pinsel[(addr-0xe002c000)/4] = data;
			break;
		}
		/*Pin Mode Select Control*/
		if(addr-0xe002c040 <=0x24 && addr-0xe002c040>=0){
			io.pinmode[(addr-0xe002c040)/4] = data;
			break;
		}
#if 1
		fprintf(stderr,"WARNING:Writing address 0x%08x (data=0x%x) is not implemented.\n", addr, data);
		/*
		fprintf(stderr,"NumInstr %llu,io_write_word unknown addr(1x%08x) = 0x%08x\n", state->NumInstrs, addr, data);*/
		//SKYEYE_OUTREGS(stderr);
		//ARMul_Debug(state, 0, 0);
#endif
		break;
	}
}

void lpc2300_mach_init(ARMul_State *state, machine_config_t *this_mach)
{
	//chy 2003-08-19, setprocessor
	ARMul_SelectProcessor(state, ARM_v4_Prop);
        //chy 2004-05-09, set lateabtSig
        state->lateabtSig = HIGH;

	this_mach->mach_io_do_cycle = 		lpc2300_io_do_cycle;
	this_mach->mach_io_reset = 		lpc2300_io_reset;
	this_mach->mach_io_read_byte = 		lpc2300_io_read_byte;
	this_mach->mach_io_write_byte = 	lpc2300_io_write_byte;
	this_mach->mach_io_read_halfword = 	lpc2300_io_read_halfword;
	this_mach->mach_io_write_halfword = 	lpc2300_io_write_halfword;
	this_mach->mach_io_read_word = 		lpc2300_io_read_word;
	this_mach->mach_io_write_word = 	lpc2300_io_write_word;

	this_mach->mach_update_int = 		lpc2300_update_int;

	//ksh 2004-2-7


	state->mach_io.instr = (ARMword *)&io.vic.IRQStatus;
	//*state->io.instr = (ARMword *)&io.intsr;
	//state->io->net_flags = (ARMword *)&io.net_flags;
	//state->mach_io.net_int = (ARMword *)&io.net_int;
}
