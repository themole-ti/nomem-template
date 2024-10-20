/* Linker script to create TI99/4A cartridges */

/* Output straight to a flat binary format (i.e. not ELF) */
OUTPUT_FORMAT(binary)
OUTPUT(cartridge.bin)

/* TI memory layout */
MEMORY
{
	cart_rom (rx) : ORIGIN = 0x6000, LENGTH = 0x2000 /* cartridge ROM, read-only */
	scratchpad (wx) : ORIGIN = 0x8320, LENGTH = 0x00df /* 32b is for gcc workspace */
}

/* Where we put sections */
SECTIONS
{
	. = 0x6000;

	/* Sections can define VMA and LMA addresses.
	   		VMA: Defines where the section ends up in the system's RAM address space, the link addresses
	   		LMA: Defines where the section resides in the ROM image, starting from the offset above (i.e. 0x6000)

	   In case the VMA address is absent, the linker will refer to the >region in MEMORY and use the next free
	   address there.

	   crt0.c will copy .data and .bss sections from their LMA to their VMA addresses

       .section [VMA] : [AT(LMA)]
       {
           output-section-command;
           output-section-command;
           ...
       } [>region]

	*/
	.header : AT(0x6000)
	{ 
		src/cart_header.o(.text);
	} >cart_rom

	.src : AT (ADDR (.header) + SIZEOF (.header))
	{ 
		src\\*.o(.text); 
		src/*.o(.text);  /**/
	} >cart_rom
	
	.data : AT (ADDR (.src) + SIZEOF (.src))
	{
		_data_src = LOADADDR(.data);
		_data = . ; 
		src\\*.o( .data ) 
		src/*.o( .data ) /**/
		_data_end = . ;
	} >scratchpad

	.bss : AT (ADDR (.data) + SIZEOF (.data))
	{ 
		_bss = . ; 
		src\\*.o( .bss ) 
		src/*.o( .bss ) /**/
		_bss_end = . ;
	} >scratchpad

	/* pad to 8k */
	.fill 0x7fff : AT (0x7fff)
	{
		BYTE(0);
	}
}