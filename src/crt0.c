// crt0.c - C runtime initialization code
extern unsigned int  _bss;
extern unsigned int  _bss_end;
extern unsigned int  _data;
extern unsigned int  _data_end;
extern unsigned int  _data_src;

// Linker will look for _start symbol as the entry point of our program
void _start()
{
	// Start by turning off interupts, and setting the workspace pointer
	__asm__
	(
		"limi	0	\n\t"
		"lwpi	0x8300\n\t"
	);

	// Set stack
	__asm__
	(
		"li	sp, 0x8400\n\t"
	);

	// Make uninitialized/unused memory in scratchpad visible in debugger
	{
		unsigned int dst = 0x8320;
		while (dst < 0x83ff)
			*((int*)dst++) = 0xAAAA;
	}

	// Zero BSS
	unsigned int *dst = 0;
	for (dst = &_bss; dst < &_bss_end; dst++)
		*dst = 0;

	// Copy .data from ROM to RAM
	unsigned int *src = &_data_src;
	for (dst = &_data; dst < &_data_end; dst++)
		*dst = *src++;

	// Start executing C program from main function
	__asm__
	(
		"b	@main\n\t"
	);
}
