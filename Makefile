VARIANT	?= d88
NAME	:= usbboot_$(VARIANT)

SRC	:= main.c cgu.c gpio.c uart.c sdram.c #lcd.c #wdt.c helper.c keypad.c nand.c
SRC	+= startup.S

OBJ	= $(patsubst %.S,%.o,$(SRC:%.c=%.o))

CROSS	?= mipsel-linux-
AS	:= $(CROSS)gcc
CC	:= $(CROSS)gcc
CXX	:= $(CROSS)g++
LD	:= $(CROSS)g++
NM	:= $(CROSS)nm
SIZE	:= $(CROSS)size
OBJCOPY	:= $(CROSS)objcopy

ARGS	= -mips1 -g -Os -mno-abicalls -fno-pic -fno-pie -nostdlib -flto -ffreestanding
DEFS	= -DVARIANT=VARIANT_$(shell echo '$(VARIANT)' | tr '[:lower:]' '[:upper:]')
CFLAGS	= $(ARGS) $(DEFS)
ASFLAGS	= $(ARGS) $(DEFS)
LDFLAGS	= $(ARGS) -Xlinker --gc-sections

.DELETE_ON_ERROR:

.PHONY: all
all: stage1/$(NAME)_stage1.bin

# Fill the backup section
%.bin: %.elf
	$(OBJCOPY) -O binary --gap-fill 0xff $< $@

stage1/%.elf: $(OBJ:%.o=stage1/%.o) | stage1
	$(LD) -o $@ $^ $(LDFLAGS) -T stage1.ld
	$(SIZE) $@

stage1/%.o: %.c | stage1
	$(CC) -DSTAGE=1 $(CFLAGS) -o $@ -c $<

stage1/%.o: %.S | stage1
	$(AS) -DSTAGE=1 $(ASFLAGS) -o $@ -c $<

stage1 stage2: %:
	mkdir -p $@

.PHONY: clean
clean:
	rm -rf stage1 stage2
