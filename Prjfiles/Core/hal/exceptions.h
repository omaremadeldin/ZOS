//==========================================
//
//		     ZapperOS - Exceptions
//
//==========================================
//Used Acronyms:
//--------------
//* NMI = Non Maskable Interrupt
//* OP Code = Operation Code
//* TSS = Task State Segment
//==========================================
//By Omar Emad Eldin
//==========================================

#pragma once

#include <stdlib.h>

#include "vmm.h"

namespace zos
{
	class Exceptions
	{
	private:
		friend void ihErrDivideByZero();
		friend void ihErrSingleStepDebug();
		friend void ihErrNMIPin();
		friend void ihErrBreakpointDebug();
		friend void ihErrOverflow();
		friend void ihErrBoundsRange();
		friend void ihErrUndefinedOPCode();
		friend void ihErrDoubleFault();
		friend void ihErrInvalidTSS();
		friend void ihErrSegmentNotPresent();
		friend void ihErrStackSegOverun();
		friend void ihErrGeneralProtectionFault();
		friend void ihErrPageFault(VirtualAddress requestedAddress, VirtualAddress currentAddress);

	public:
		static void init();
	};
}