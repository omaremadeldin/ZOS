//==========================================
//
//		      ZapperOS - KERNEL
//
//==========================================
//By Omar Emad Eldin
//==========================================

//TODO:Usermode Protections
//Paging
//Interrupts
//Segmentation

#include <stdlib.h>
#include <gccfuncs.h>

#include "hal/hal.h"
#include "hal/kybrd.h"

using namespace zos;

extern "C" void kmain()
{
	HAL::init();
}