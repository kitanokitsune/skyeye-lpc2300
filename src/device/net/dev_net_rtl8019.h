/*
	dev_net_rtl8019.h - skyeye realtek 8019 ethernet controllor simulation
	Copyright (C) 2003 - 2005 Skyeye Develop Group
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
 * 05/25/2005 	modified for rtl8019
 *			walimis <wlm@student.dlut.edu.cn>
 *
 * 02/25/2003 	initial version
 *			yangye <yangye@163.net> 		
 *			chenyu <chenyu@hpclab.cs.tsinghua.edu.cn>
 */

#ifndef _DEV_NET_RTL8019_H_
#define _DEV_NET_RTL8019_H_


#define NE_CR              0x0	//R/W���Բ�ͬ��ҳ��CR����ͬһ��
//page0 registers  
#define NE_PSTART          0x01	//W�����ջ��廷��ʼҳ
#define NE_PSTOP           0x02	//W�����ջ��廷��ֹҳ����������ҳ��
#define NE_BNRY            0x03	//R/W�����ջ��廷��ָ�룬ָ����һ��������ʱ����ʼҳ,Ӧ��ʼ���ɣ�CURR��PSTART
#define NE_TPSR            0x04	//W��Local DMA���ͻ�����ʼҳ�Ĵ���
#define NE_TBCR0           0x05	//W��Local DMA���ͳ��ȵ�λ
#define NE_TBCR1           0x06	//W��Local DMA���ͳ��ȸ�λ
#define NE_ISR             0x07	//R/W���ж�״̬�Ĵ���
#define NE_RSAR0           0x08	//W��Remote DMAĿ����ʼ��ַ��λ
#define NE_RSAR1           0x09	//W��Remote DMAĿ����ʼ��ַ��λ
//��������CPU������д���������ݰ���ʵ�ʳ��ȣ�ִ��Remote DMA����ǰ����
#define NE_RBCR0           0x0a	//W��Remote DMA���ݳ��ȵ�λ
#define NE_RBCR1           0x0b	//W��Remote DMA���ݳ��ȸ�λ
#define NE_RCR             0x0c	//W���������üĴ���,��ʼ��ʱд��0x04,��ʾֻ���շ���������MAC��ַ��,����64�ֽڵ���̫������㲥��
#define NE_TCR             0x0d	//�������üĴ���,��ʼ����ʼʱд��0x02��������ΪLoop Backģʽ��ֹͣ�������ݰ�,��ʼ������д��0x00�������������ݰ�������CRC
#define NE_DCR             0x0e	//W���������üĴ���,��ʼ��ʱд��0x48��8λģʽ��FIFO���8�ֽڣ�DMA��ʽ
#define NE_IMR             0x0f	//W���ж����μĴ������ĸ�λ��ISR�еĸ�λ���Ӧ����IMRд��ֵ��Ϊ����Ӧ�ж�

//page1 registers
#define NE_PAR0            0x01	//R/W������MAC��ַ���λ
#define NE_PAR1            0x02	//R/W������MAC��ַ
#define NE_PAR2            0x03	//R/W������MAC��ַ
#define NE_PAR3            0x04	//R/W������MAC��ַ
#define NE_PAR4            0x05	//R/W������MAC��ַ
#define NE_PAR5            0x06	//R/W������MAC��ַ���λ
#define NE_CURR            0x07	//R/W�����ջ��廷дָ��
#define NE_MAR0            0x08	//R/W���鲥�Ĵ���
#define NE_MAR1            0x09	//R/W���鲥�Ĵ���
#define NE_MAR2            0x0a	//R/W���鲥�Ĵ���
#define NE_MAR3            0x0b	//R/W���鲥�Ĵ���
#define NE_MAR4            0x0c	//R/W���鲥�Ĵ���
#define NE_MAR5            0x0d	//R/W���鲥�Ĵ���
#define NE_MAR6            0x0e	//R/W���鲥�Ĵ���
#define NE_MAR7            0x0f	//R/W���鲥�Ĵ���

//page2 registers
//#define NE_PSTART          0x01            //R�����ջ��廷��ʼҳ
//#define NE_PSTOP           0x02            //R�����ջ��廷��ֹҳ����������ҳ��
//#define NE_TPSR            0x04            //R��Local DMA���ͻ�����ʼҳ�Ĵ���
//#define NE_RCR             0x0c            //R���������üĴ���
//#define NE_TCR             0x0d            //R,�������üĴ���
//#define NE_DCR             0x0e            //R���������üĴ���
//#define NE_IMR             0x0f            //R���������ж����μĴ���IMR״̬


//CR����Ĵ��������� 
#define	CMD_STOP	0x01	//����ֹͣ�շ�����
#define	CMD_RUN	    0x02	//����ִ�������ʼ�շ����ݰ�������Ϊ�������֣�
#define	CMD_XMIT	0x04	//Local DMA SEND�������D�D>��̫�� ��
#define	CMD_READ	0x08	//Remote DMA READ�������ֶ��������ݣ������D�D>CPU��
#define	CMD_WRITE	0x10	//Remote DMA WRITE ������<�D�DCPU��
#define	CMD_SEND	0x18	//SEND COMMAND��������Զ��������ݰ�                                      �������D�D>CPU��
#define	CMD_NODMA	0x20	//ֹͣDMA����
#define	CMD_PAGE0	0x00	//   ѡ���0ҳ��Ҫ��ѡҳ���ٶ�д��ҳ�Ĵ�����
#define	CMD_PAGE1	0x40	//   ѡ���1ҳ
#define	CMD_PAGE2	0x80	//   ѡ���2ҳ

//д��TPSR��ֵ 
#define	XMIT_START	 0x4000	//���ͻ�����ʼ��ַ��д��ʱҪ����8λ�õ�ҳ�ţ�
//д��PSTART��ֵ 
#define	RECV_START	 0x4600	//���ջ�����ʼ��ַ��д��ʱҪ����8λ�õ�ҳ�ţ�
//д��PSTOP��ֵ 
#define	RECV_STOP	 0x6000	//���ջ��������ַ��д��ʱҪ����8λ�õ�ҳ�ţ�

//�ж�״̬�Ĵ�����ֵ 
#define	ISR_PRX	    0x01	//��ȷ�������ݰ��жϡ������մ���
#define	ISR_PTX		0x02	//��ȷ�������ݰ��жϡ�����������Ҫ���ϲ�����ˡ�
#define	ISR_RXE	    0x04	//�������ݰ���������������BNRY��CURR����
#define	ISR_TXE	    0x08	//���ڳ�ͻ�������࣬���ͳ������ط�����
#define	ISR_OVW	    0x10	//�����ڴ��������������������������ֲᡣ
#define	ISR_CNT	    0x20	//����������жϣ����ε���������IMR�Ĵ�������
#define	ISR_RDC	    0x40	//Remote DMA���� �����ε�����ѯ�ȴ�DMA������
#define	ISR_RST		0x80	//����Reset�����ε���
//�ж����μĴ�����ֵ 
#define	ISR_PRX	    0x01
#define	ISR_PTX		0x02
#define	ISR_RXE	    0x04
#define	ISR_TXE	    0x08
#define	ISR_OVW	    0x10
#define	ISR_CNT 	0x20
#define	ISR_RDC	    0x40
#define	ISR_RST		0x80


//���ݿ��ƼĴ���
//��ʼ��ʱд��0x48��8λģʽ��FIFO���8�ֽڣ�DMA��ʽ��
#define DCR_WTS 	0x01
#define DCR_BOS 	0x02
#define DCR_LAS 	0x04
#define DCR_LS		0x08
#define DCR_ARM 	0x10
#define DCR_FIFO2	0x00
#define DCR_FIFO4	0x20
#define DCR_FIFO8	0x40
#define DCR_FIFO12	0x60

//TCR�������üĴ���
//��ʼ����ʼʱд��0x02��������ΪLoop Backģʽ��ֹͣ�������ݰ���
//��ʼ������д��0x00�������������ݰ�������CRC��
#define TCR_CRC 	0x01
#define TCR_LOOP_NONE	0x00
#define TCR_LOOP_INT	0x02
#define TCR_LOOP_EXT	0x06
#define TCR_ATD 	0x08
#define TCR_OFST	0x10

//RCR�������üĴ���
//��ʼ��ʱд��0x04��ֻ���շ���������MAC��ַ����64�ֽڵ���̫������㲥��
#define RCR_SEP 	0x01
#define RCR_AR		0x02
#define RCR_AB		0x04
#define RCR_AM		0x08
#define RCR_PRO 	0x10
#define RCR_MON 	0x20

/* Bits in received packet status byte and EN0_RSR*/
#define RSR_RXOK      0x01	/* Received a good packet */
#define RSR_CRC       0x02	/* CRC error */
#define RSR_FAE       0x04	/* frame alignment error */
#define RSR_FO        0x08	/* FIFO overrun */
#define RSR_MPA       0x10	/* missed pkt */
#define RSR_PHY       0x20	/* physical/multicast address */
#define RSR_DIS       0x40	/* receiver disable. set in monitor mode */
#define RSR_DEF       0x80	/* deferring */

/* Transmitted packet status, EN0_TSR. */
#define TSR_PTX 0x01		/* Packet transmitted without error */
#define TSR_ND  0x02		/* The transmit wasn't deferred. */
#define TSR_COL 0x04		/* The transmit collided at least once. */
#define TSR_ABT 0x08		/* The transmit collided 16 times, and was deferred. */
#define TSR_CRS 0x10		/* The carrier sense was lost. */
#define TSR_FU  0x20		/* A "FIFO underrun" occurred during transmit. */
#define TSR_CDH 0x40		/* The collision detect "heartbeat" signal was lost. */
#define TSR_OWC 0x80		/* There was an out-of-window collision. */



/* walimis */
#define START_PAGE	0x40
#define END_PAGE	0x80
#define PAGE_SIZE	0x100
#define PAGE_NUM	(END_PAGE - START_PAGE)	/* 16Kbytes */

#define INT_RTL8019 0



//the structure 

typedef struct net_rtl8019_io
{
	// Page 0
	//  Command Register - 00h read/write

	u8 CR;
	//01h write ; page start register
	u8 PSTART;
	//02h write ; page stop register
	u8 PSTOP;
	//03h read/write ; boundary pointer
	u8 BNRY;
	//04h write ; transmit page start register
	u8 TSR;
	u8 TPSR;
	//05,06h write ; transmit byte-count register
	u8 TBCR0;
	u8 TBCR1;
	// Interrupt Status Register - 07h read/write
	u8 ISR;
	//08,09h write ; remote start address register
	u8 RSAR0;
	u8 RSAR1;
	//0a,0bh write ; remote byte-count register
	u8 RBCR0;
	u8 RBCR1;
	// Receive Configuration Register - 0ch write
	u8 RSR;
	u8 RCR;
	// Transmit Configuration Register - 0dh write
	u8 CNTR0;
	u8 TCR;
	// Data Configuration Register - 0eh write
	u8 CNTR1;
	u8 DCR;
	// Interrupt Mask Register - 0fh write
	u8 CNTR2;
	u8 IMR;

	// Page 1
	//
	// Command Register 00h (repeated)
	//01-06h read/write ; MAC address
	u8 PAR0;
	u8 PAR1;
	u8 PAR2;
	u8 PAR3;
	u8 PAR4;
	u8 PAR5;
	// 07h read/write ; current page register
	u8 CURR;
	// 08-0fh read/write ; multicast hash array
	//Bit8u  MAR[8];    


	//
	// Page 2  - diagnostic use only
	// 
	//   Command Register 00h (repeated)
	//
	//   Page Start Register 01h read  (repeated)   PSTART
	//   Page Stop Register  02h read  (repeated)   PSTOP
	//   Transmit Page start address 04h read (repeated)  TPSR
	//   Receive Configuration Register 0ch read (repeated) RCR
	//   Transmit Configuration Register 0dh read (repeated)TCR
	//   Data Configuration Register 0eh read (repeated) DCR
	//   Interrupt Mask Register 0fh read (repeated) IMR
	//

	u8 PROM[12];		// 12 bytes in Prom for MAC addr.

	u8 *sram;
	u32 remote_read_offset;
	u32 remote_write_offset;

	u32 remote_read_count;
	u32 remote_write_count;

	int need_update;
	int index;
	int op_16;
} net_rtl8019_io_t;

static u8 rtl8019_output (struct device_desc *dev, u8 * buf, u16 packet_len);
static void rtl8019_input (struct device_desc *dev);
#endif //_DEV_NET_RTL8019_H_
