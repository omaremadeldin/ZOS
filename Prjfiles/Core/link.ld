ENTRY(_start)

SECTIONS
{
	KERNEL_VBASE = 0xC0000000;
    . = KERNEL_VBASE;
	
	/* (entry text) Assembly stub to transfer control to c++ */
	.etext : AT(ADDR(.etext) - KERNEL_VBASE)
	{
		*(.etext);
	}
    
    /* (entry data) Assembly stub to transfer control to c++ */
	.edata BLOCK (4K): AT(ADDR(.edata) - KERNEL_VBASE)
	{
		*(.edata);
	}
	
	.ctors :
	{
		start_ctors = .;
		*(SORT(.ctors*))
		end_ctors = .;
	}
	
	/* Read-write data (initialized) */
	.data :
	{
		*(.data);
		*(.rodata);
	}

	/* Read-write data (uninitialized) and stack */
	.bss :
	{
		*(.bss);
	}
	
	.init :
	{
		*(.init);
	}
	
	/*
	.fini :
	{
		*(.fini);
	}
	*/
	
	/* Executable Code*/
	.text :
    {
        *(.text)
        . = ALIGN(4096);
    }
    
	/DISCARD/ :
	{
		crtend.o(.eh_frame)
		crtend.o(.eh_frame_hdr)
		*(.comment)
		*(.note)
	}
}