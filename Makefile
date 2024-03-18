NAME	:= bootrom

SRC	:= helper.c startup.S
SRC	+= gpio.c cgu.c uart.c nand.c

OBJ	= $(patsubst %.S,%.o,$(SRC:%.c=%.o))

CROSS	?= mipsel-none-elf-
AS	:= $(CROSS)gcc
CC	:= $(CROSS)gcc
CXX	:= $(CROSS)g++
LD	:= $(CROSS)gcc
NM	:= $(CROSS)nm
SIZE	:= $(CROSS)size
OBJCOPY	:= $(CROSS)objcopy

ARGS	= -mips32 -g -Os -fno-pic -fno-pie -flto -ffreestanding
ARGS	+= -nostdlib -nostartfiles -specs=nosys.specs
ARGS	+= -Wall -Wextra -Wno-unused-variable -Wno-unused-const-variable -Wno-unused-function
DEFS	= -DJZ4755=1
CFLAGS	= -std=gnu17 $(ARGS) $(DEFS)
ASFLAGS	= $(ARGS) $(DEFS)
LDFLAGS	= $(ARGS) -Xlinker --gc-sections -G0

.DELETE_ON_ERROR:
.SECONDARY:

.PHONY: all
all: build/$(NAME).bin

# Fill the backup section
%.bin: %.elf
	$(OBJCOPY) -O binary --gap-fill 0xff $< $@

build/%.elf: build/main.o $(OBJ:%.o=build/%.o) | build
	$(LD) -o $@ $^ $(LDFLAGS) -T bootrom.ld
	$(SIZE) $@

build/%.o: %.c | build
	$(CC) -DSTAGE=1 $(CFLAGS) -o $@ -c $<

build/%.o: %.S | build
	$(AS) -DSTAGE=1 $(ASFLAGS) -o $@ -c $<

build: %:
	mkdir -p $@

.PHONY: clean
clean:
	rm -rf build
