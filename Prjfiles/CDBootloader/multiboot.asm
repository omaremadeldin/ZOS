;==========================================
;	 	ZapperOS Multiboot Structure
;------------------------------------------
;Done By Omar Emad Eldin
;==========================================

BITS 32

struc multiboot_info
	.flags				RESD	1	; required
	.memoryLo			RESD	1	; memory size. Present if flags[0] is set
	.memoryHi			RESD	1
	.bootDevice			RESD	1	; boot device. Present if flags[1] is set
	.cmdLine			RESD	1	; kernel command line. Present if flags[2] is set
	.mods_count			RESD	1	; number of modules loaded along with kernel. present if flags[3] is set
	.mods_addr			RESD	1
	.syms0				RESD	1	; symbol table info. present if flags[4] or flags[5] is set
	.syms1				RESD	1
	.syms2				RESD	1
	.mmap_length		RESD	1	; memory map. Present if flags[6] is set
	.mmap_addr			RESD	1
	.drives_length		RESD	1	; phys address of first drive structure. present if flags[7] is set
	.drives_addr		RESD	1
	.config_table		RESD	1	; ROM configuation table. present if flags[8] is set
	.bootloader_name 	RESD	1	; Bootloader name. present if flags[9] is set
	.apm_table			RESD	1	; advanced power management (apm) table. present if flags[10] is set
	.vbe_control_info 	RESD	1	; video bios extension (vbe). present if flags[11] is set
	.vbe_mode_info		RESD	1
	.vbe_mode			RESW	1
	.vbe_interface_seg 	RESW	1
	.vbe_interface_off 	RESW	1
	.vbe_interface_len 	RESW	1
endstruc
