# SDCardOperate
这是一个SD卡读写测试的裸机程序，可以不用修改直接在韦东山的JZ2440开发板上运行。

##使用方法
*程序菜单*
##### SD Card Read #####
[R] Read SD card
[W] Write SD card
Enter your selection:

*读SD卡*
1.输入R或r
2.见到“Enter Address and Size：”后，依次输入读取地址和读取长度。
  etc:
  0x0 512 (从0x0地址读取512字节）

*写SD卡*
1.输入W或w
2.见到“Enter Address and Data：”后，依次输入修改起始地址和修改数据。
  etc：
  0x0 0x21 0x34 （从0x0地址开始，依次修改0x0地址和0x1地址数据）
