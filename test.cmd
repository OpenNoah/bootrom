connect 0x601a 0x4750 0
cpu

echo CGU dump
mdw 0xb0000000
mdw 0xb0000010
mdw 0xb0000014
mdw 0xb0000060
mdw 0xb0000064
mdw 0xb0000068
mdw 0xb0000074
mdw 0xb000007c

# *0xb0000000 = 0x49a33330
# *0xb0000010 = 0x09000520
# *0xb0000014 = 0x34000000
# *0xb0000060 = 0x00000004
# *0xb0000064 = 0x00000004
# *0xb0000068 = 0x00000000
# *0xb0000074 = 0x00000000
# *0xb000007c = 0x00000004

echo SDRAM dump
mdw 0xb3010080

echo NAND dump
mdw 0xb3010000
mdw 0xb3010014
mdw 0xb3010018

echo Stage 1 firmware
sleep 1
config /home/zhiyb/ingenic/usbboot/configs/dicple_d88.cfg
#stage1 /home/zhiyb/ingenic/usbboot_stage1.bin
stage1 /tmp/usbboot_d88_stage1.bin
sleep 1
cpu

echo Firmware outputs
mdw 0xf4000000
mdw 0xf4000004
mdw 0xf4000008
mdw 0xf400000c

#echo SDRAM test
#write 0x20000000 /home/zhiyb/tmp/test.bin
#verify 0x20000000 /home/zhiyb/tmp/test.bin
#read 0x20000000 0x04000000 /run/d88_check.bin
#cpu

echo LCD state
mdw 0xb3050000
mdw 0xb3050030
mdw 0xb3050034
mdw 0xb3050100
mdw 0xb3050104
mdw 0xb3050108
