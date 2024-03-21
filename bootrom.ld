/* Specify the memory areas */
MEMORY
{
  /* 8KiB of BootROM space */
  ROM(xr):    ORIGIN = 0x9fc00000, LENGTH = 0x2000
  /* 16KiB of SRAM is available initially, but use only the first 8KiB for BootROM */
  RAM(xrw):   ORIGIN = 0x80002000, LENGTH = 0x2000
}

/* Stack starts from the end of available RAMs */
__stack_end__ = ORIGIN(RAM) + LENGTH(RAM);

/* Entry Point */
ENTRY(_header)

/* Define output sections */
SECTIONS
{
  /* Header for bootrom to identify NAND type and entry */
  .header :
  {
    . = ALIGN(4);
    KEEP(*(.header))
  } >ROM

  /* The program code and other data goes into TEXT */
  .text :
  {
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.eh_frame)

    *(.init)
    *(.fini)
    _etext = .;        /* define a global symbols at end of code */
  } >ROM

  /* Constant data goes into TEXT */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
  } >ROM

  . = ALIGN(4);
  .preinit_array     :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP(*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >ROM
  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP(*(SORT(.init_array.*)))
    KEEP(*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >ROM
  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP(*(SORT(.fini_array.*)))
    KEEP(*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >ROM

  /* Initialized data sections goes into RAM, load LMA copy after code */
  . = ALIGN(4);
  /* used by the startup to initialize data */
  __data_load__ = LOADADDR(.data);
  .data :
  {
    __data_start__ = .;
    . = ALIGN(4);
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */
    /* Special program code inside RAM */
    *(.ram)
    *(.ram*)

    . = ALIGN(4);
    __data_end__ = .;
  } >RAM AT >ROM

  /* Uninitialized data section */
  . = ALIGN(4);
  .bss (NOLOAD) :
  {
    /* This is used by the startup in order to initialize the .bss secion */
    __bss_start__ = .;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    __bss_end__ = .;
  } >RAM

  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }
}
