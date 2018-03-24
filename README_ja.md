# skyeye-lpc2300
**SkyEye ARM Simulator for LPC2300**

## What is this
これは[SkyEye ARM simulator](https://sourceforge.net/projects/skyeye/)のブランチです。ベースのSkyEyeのバージョンは[1.2.6_rc1](https://sourceforge.net/projects/skyeye/files/skyeye/skyeye-1.2.6_rc1/)です。

このプロジェクトはSkyEyeにNXP LPC23xx（特に**CQ-FRK-NXP-ARM**基板）のサポートを追加することです。

現在、以下の機能が追加されています。
* UART(0/1/2/3) および Rx/Tx 割り込み
* TIMER(0/1/2/3) および MR0/1/2/3 割り込み

## How to build
### For Linux
単に **cd src** 、 **./configure** 、 **make** を続けて実行してください。
### For MS-Windows
**bin/** ディレクトリに完成した実行ファイル **skyeye.exe** があります。
もし、自分でビルドする場合はMinGW/MSYS環境が必要です。MSYSシェル上で以下のコマンドを実行してください。

* **cd src*
* **./configure --enable-lcd=no LDFLAGS="-static -static-libstdc++ -static-libgcc -s"**
* **make**

## Known Issue
KEILのサンプル __*Interrupt-Driven UART I/O for Philips LPC23xx*__ は動作することを確認していますが、他のプログラムで動かないものも確認されています。おそらくモード切り替えに絡むCPSR/SPSRの動作がおかしいようです。
