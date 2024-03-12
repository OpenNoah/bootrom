/* Specify the memory areas */
MEMORY
{
  /* 2MiB at start of SDRAM space + 2MiB offset */
  RAM(xrw):   ORIGIN = 0x80200000, LENGTH = 0x00400000 - 0x00200000
  /* Next 4MiB space for dynamic allocation */
  ALLOC(xrw): ORIGIN = ORIGIN(RAM) + LENGTH(RAM), LENGTH = 0x00400000
}

/* Stack starts from the end of available RAMs */
__stack_end__ = ORIGIN(RAM) + LENGTH(RAM);

/* SDRAM space for dynamic allocation */
__alloc_start__ = ORIGIN(ALLOC);

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
  } >RAM

  /* The program code and other data goes into TEXT */
  .text :
  {
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.eh_frame)

    KEEP(*(.init))
    KEEP(*(.fini))
    _etext = .;        /* define a global symbols at end of code */
  } >RAM

  /* Constant data goes into TEXT */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
  } >RAM

  . = ALIGN(4);
  .preinit_array     :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP(*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >RAM
  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP(*(SORT(.init_array.*)))
    KEEP(*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >RAM
  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP(*(SORT(.fini_array.*)))
    KEEP(*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >RAM

  /* Initialized data sections goes into RAM, load LMA copy after code */
  . = ALIGN(4);
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
  } >RAM

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
    *(.olist.*)
  }
}
