# skyeye-lpc2300
**SkyEye ARM Simulator for LPC2300**


## What is this
これは[SkyEye ARM simulator](https://sourceforge.net/projects/skyeye/)のforkです。ベースのSkyEyeのバージョンは[1.2.6_rc1](https://sourceforge.net/projects/skyeye/files/skyeye/skyeye-1.2.6_rc1/)です。

このプロジェクトの目的はSkyEyeにNXP LPC23xx（特に**CQ-FRK-NXP-ARM**基板）のサポートを追加することです。

現在、以下のデバイスが追加されています。
* **LPC23xx family** (LPC2300, LPC2361, LPC2362, LPC2364, LPC2365, LPC2366, LPC2367,LPC2368, LPC2377, LPC2378, LPC2387, LPC2388)
* **LPC213x family** (LPC2131, LPC2132, LPC2134, LPC2136, LPC2138)

また、これらのデバイスに以下の機能を実装しています。
* **UART**(0/1/2/3) および Rx/Tx 割り込み
* **TIMER**(0/1/2/3) および MR0/1/2/3 割り込み


## How to build
### For Linux
以下を実行してください。
* **cd src**
* **./configure**
* **make**

### For MS-Windows
**bin/** ディレクトリに完成した実行ファイル **skyeye.exe** があります。
もし、自分でビルドする場合はMinGW/MSYS環境が必要です。MSYSシェル上で以下のコマンドを実行してください。

* **cd src**
* **./configure --enable-lcd=no LDFLAGS="-static -static-libstdc++ -static-libgcc -s"**
* **make**


## Licence
GNU GPL v2


## Known Issue
KEILのサンプル __*Interrupt-Driven UART I/O for Philips LPC23xx*__ は動作することを確認していますが、他のプログラムで動かないものも確認されています。おそらくモード切り替え（特にモード USR -> IRQ -> SVC という遷移）に絡むCPSR/SPSRの動作がおかしいようです。
