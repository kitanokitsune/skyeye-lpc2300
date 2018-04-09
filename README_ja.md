# skyeye-lpc2300
**SkyEye ARM Simulator for LPC2300**


## What is this
�����[SkyEye ARM simulator](https://sourceforge.net/projects/skyeye/)��fork�Ǥ����١�����SkyEye�ΥС�������[1.2.6_rc1](https://sourceforge.net/projects/skyeye/files/skyeye/skyeye-1.2.6_rc1/)�Ǥ���

���Υץ������Ȥ���Ū��SkyEye��NXP LPC23xx���ä�**CQ-FRK-NXP-ARM**���ġˤΥ��ݡ��Ȥ��ɲä��뤳�ȤǤ���

���ߡ��ʲ��ΥǥХ������ɲä���Ƥ��ޤ���
* **LPC23xx family** (LPC2300, LPC2361, LPC2362, LPC2364, LPC2365, LPC2366, LPC2367,LPC2368, LPC2377, LPC2378, LPC2387, LPC2388)
* **LPC21xx family** (LPC2131, LPC2132, LPC2134, LPC2136, LPC2138)

�ޤ��������ΥǥХ����˰ʲ��ε�ǽ��������Ƥ��ޤ���
* **UART**(0/1/2/3) ����� Rx/Tx ������
* **TIMER**(0/1/2/3) ����� MR0/1/2/3 ������


## How to build
### For Linux
�ʲ���¹Ԥ��Ƥ���������
* **cd src**
* **./configure**
* **make**

### For MS-Windows
**bin/** �ǥ��쥯�ȥ�˴��������¹ԥե����� **skyeye.exe** ������ޤ���
�⤷����ʬ�ǥӥ�ɤ������MinGW/MSYS�Ķ���ɬ�פǤ���MSYS�������ǰʲ��Υ��ޥ�ɤ�¹Ԥ��Ƥ���������

* **cd src**
* **./configure --enable-lcd=no LDFLAGS="-static -static-libstdc++ -static-libgcc -s"**
* **make**


## Licence
GNU GPL v2


## Known Issue
KEIL�Υ���ץ� __*Interrupt-Driven UART I/O for Philips LPC23xx*__ ��ư��뤳�Ȥ��ǧ���Ƥ��ޤ�����¾�Υץ�����ư���ʤ���Τ��ǧ����Ƥ��ޤ��������餯�⡼���ڤ��ؤ������CPSR/SPSR��ư����������褦�Ǥ���
