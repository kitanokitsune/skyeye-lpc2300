/*
	skyeye_uart_dummy.c - skyeye uart device from standard input/output
	Copyright (C) 2003 - 2007 Skyeye Develop Group
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
 * 2018.03.24	added dummy uart
 */

#include "skyeye_uart.h"

int uart_dummy_open(struct uart_device *uart_dev)
{
	return 0;
}


int uart_dummy_close(struct uart_device *uart_dev)
{
	return 0;
}


int uart_dummy_read(struct uart_device *uart_dev, void *buf, size_t count, struct timeval *timeout)
{
	return -1;
}


int uart_dummy_write(struct uart_device *uart_dev, void *buf, size_t count)
{
	return -1;
}

