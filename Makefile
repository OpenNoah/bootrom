VARIANT	?= d88
NAME	:= usbboot_$(VARIANT)

SRC	:= helper.c cgu.c gpio.c uart.c sdram.c lcd.c i2c.c nand.c #wdt.c keypad.c
SRC	+= startup.S

OBJ	= $(patsubst %.S,%.o,$(SRC:%.c=%.o))

CROSS	?= mipsel-linux-
AS	:= $(CROSS)gcc
CC	:= $(CROSS)gcc
CXX	:= $(CROSS)g++
LD	:= $(CROSS)gcc
NM	:= $(CROSS)nm
SIZE	:= $(CROSS)size
OBJCOPY	:= $(CROSS)objcopy

ARGS	= -mips32 -g -Os -fno-pic -fno-pie -mno-abicalls -nostdlib -flto -ffreestanding
ARGS	+= -Wall -Wextra -Wno-unused-variable -Wno-unused-const-variable -Wno-unused-function
DEFS	= -DVARIANT=VARIANT_$(shell echo '$(VARIANT)' | tr '[:lower:]' '[:upper:]')
CFLAGS	= -std=gnu17 $(ARGS) $(DEFS)
ASFLAGS	= $(ARGS) $(DEFS)
LDFLAGS	= $(ARGS) -Xlinker --gc-sections

.DELETE_ON_ERROR:

.PHONY: all
all: stage1/$(NAME)_stage1.bin stage2/$(NAME)_stage2.bin

# Fill the backup section
%.bin: %.elf
	$(OBJCOPY) -O binary --gap-fill 0xff $< $@

stage1/%.elf: stage1/stage1_main.o $(OBJ:%.o=stage1/%.o) | stage1
	$(LD) -o $@ $^ $(LDFLAGS) -T stage1.ld
	$(SIZE) $@

stage2/%.elf: stage2/stage2_main.o $(OBJ:%.o=stage2/%.o) | stage2
	$(LD) -o $@ $^ $(LDFLAGS) -T stage2.ld
	$(SIZE) $@

stage1/%.o: %.c | stage1
	$(CC) -DSTAGE=1 $(CFLAGS) -o $@ -c $<

stage2/%.o: %.c | stage2
	$(CC) -DSTAGE=2 $(CFLAGS) -o $@ -c $<

stage1/%.o: %.S | stage1
	$(AS) -DSTAGE=1 $(ASFLAGS) -o $@ -c $<

stage2/%.o: %.S | stage2
	$(AS) -DSTAGE=2 $(ASFLAGS) -o $@ -c $<

stage1 stage2: %:
	mkdir -p $@

.PHONY: clean
clean:
	rm -rf stage1 stage2
