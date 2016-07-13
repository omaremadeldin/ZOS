//==========================================
//
//		ZapperOS - Multiboot Structures
//
//==========================================
//Reference:
//--------------
//* Multiboot specification
//==========================================
//By Omar Emad Eldin
//==========================================

#pragma once

struct multiboot_info
{
	//Tells the OS what field is available and what isn't
	uint32_t flags;
	
	//Memory Size
	uint32_t mem_lower;
	uint32_t mem_upper;
	
	//Boot Device No.
	uint32_t boot_device;
	
	//Command line string to be sent to kernel
	uint32_t cmdline;
	
	//Kernel modules
	uint32_t mods_count;
	uint32_t mods_addr;
	
	//Symbol Table
	uint32_t syms_0;
	uint32_t syms_1;
	uint32_t syms_2;
	
	//Memory Map
	uint32_t mmap_length;
	uint32_t mmap_addr;
	
	//Drive Structure
	uint32_t drives_length;
	uint32_t drives_addr;
	
	//ROM Configuration Table
	uint32_t config_table;
	
	//Bootloader Name
	uint32_t boot_loader_name;
	
	//Advanced Power Management Table
	uint32_t apm_table;

	//VBE Interface
	uint32_t vbe_control_info;
	uint32_t vbe_mode_info;
	uint16_t vbe_mode;
	uint16_t vbe_interface_seg;
	uint16_t vbe_interface_off;
	uint16_t vbe_interface_len;
	
}__attribute__((packed));