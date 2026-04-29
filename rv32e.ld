ENTRY(task)

PHDRS {
    text PT_LOAD;
}

SECTIONS {
    . = 0x00000000;
    .text : { *(.prog) *(.text*) } :text
    .data : { *(.data*) }
    .bss  : { *(.bss*)  }
    /DISCARD/ : { *(.eh_frame) *(.comment) }
}
