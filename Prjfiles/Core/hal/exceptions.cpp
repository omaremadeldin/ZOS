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

#include "exceptions.h"

#include "hal.h"
#include "idt.h"
#include "video.h"

using namespace zos;

assembly_stub void aihErrDivideByZero(void);
interrupt_handler void ihErrDivideByZero(void)
{
	zos::ihErrDivideByZero();
}

assembly_stub void aihErrSingleStepDebug(void);
interrupt_handler void ihErrSingleStepDebug(void)
{
	zos::ihErrSingleStepDebug();
}

assembly_stub void aihErrNMIPin(void);
interrupt_handler void ihErrNMIPin(void)
{
	zos::ihErrNMIPin();
}

assembly_stub void aihErrBreakpointDebug(void);
interrupt_handler void ihErrBreakpointDebug(void)
{
	zos::ihErrBreakpointDebug();
}

assembly_stub void aihErrOverflow(void);
interrupt_handler void ihErrOverflow(void)
{
	zos::ihErrOverflow();
}

assembly_stub void aihErrBoundsRange(void);
interrupt_handler void ihErrBoundsRange(void)
{
	zos::ihErrBoundsRange();
}

assembly_stub void aihErrUndefinedOPCode(void);
interrupt_handler void ihErrUndefinedOPCode(void)
{
	zos::ihErrUndefinedOPCode();
}

assembly_stub void aihErrDoubleFault(void);
interrupt_handler void ihErrDoubleFault(void)
{
	zos::ihErrDoubleFault();
}

assembly_stub void aihErrInvalidTSS(void);
interrupt_handler void ihErrInvalidTSS(void)
{
	zos::ihErrInvalidTSS();
}

assembly_stub void aihErrSegmentNotPresent(void);
interrupt_handler void ihErrSegmentNotPresent(void)
{
	zos::ihErrSegmentNotPresent();
}

assembly_stub void aihErrStackSegOverun(void);
interrupt_handler void ihErrStackSegOverun(void)
{
	zos::ihErrStackSegOverun();
}

assembly_stub void aihErrGeneralProtectionFault(void);
interrupt_handler void ihErrGeneralProtectionFault(void)
{
	zos::ihErrGeneralProtectionFault();
}

assembly_stub void aihErrPageFault(VirtualAddress requestedAddress, VirtualAddress currentAddress);
interrupt_handler void ihErrPageFault(VirtualAddress requestedAddress, VirtualAddress currentAddress)
{
	zos::ihErrPageFault(requestedAddress, currentAddress);
}


void zos::ihErrDivideByZero()
{
	Video::clearScreen();
	Video::print("**UNHANDLED EXCEPTION**Divide By Zero Error*");
}

void zos::ihErrSingleStepDebug()
{
	Video::clearScreen();
	Video::print("**UNHANDLED EXCEPTION**Single Step (Debug)*");
}

void zos::ihErrNMIPin()
{
	Video::clearScreen();
	Video::print("**UNHANDLED EXCEPTION**NMI Pin Error*");
}

void zos::ihErrBreakpointDebug()
{
	Video::clearScreen();
	Video::print("**UNHANDLED EXCEPTION**Breakpoint (Debug)*");
}

void zos::ihErrOverflow()
{
	Video::clearScreen();
	Video::print("**UNHANDLED EXCEPTION**Overflow Error*");
}

void zos::ihErrBoundsRange()
{
	Video::clearScreen();
	Video::print("**UNHANDLED EXCEPTION**Bounds Range Exceeded*");
}

void zos::ihErrUndefinedOPCode()
{
	Video::clearScreen();
	Video::print("**UNHANDLED EXCEPTION**Undefined OPCode*");
}

void zos::ihErrDoubleFault()
{
	Video::clearScreen();
	Video::print("**UNHANDLED EXCEPTION**Double Fault*");
}

void zos::ihErrInvalidTSS()
{
	Video::clearScreen();
	Video::print("**UNHANDLED EXCEPTION**Invalid TSS*");
}

void zos::ihErrSegmentNotPresent()
{
	Video::clearScreen();
	Video::print("**UNHANDLED EXCEPTION**Segment Not Present*");
}

void zos::ihErrStackSegOverun()
{
	Video::clearScreen();
	Video::print("**UNHANDLED EXCEPTION**Stack Segment Overrun*");
}

void zos::ihErrGeneralProtectionFault()
{
	Video::clearScreen();
	Video::print("**UNHANDLED EXCEPTION**General Protection Fault*");
}

void zos::ihErrPageFault(VirtualAddress requestedAddress, VirtualAddress currentAddress)
{
	Video::clearScreen();
	Video::print("**UNHANDLED EXCEPTION**Page Fault*\n");
	Video::printf("  -Requested Virtual Address: 0x%x\n", requestedAddress);
	Video::printf("  -At Address: 0x%x", currentAddress);
}

void Exceptions::init()
{
	IDT::installHandler((void*)&aihErrDivideByZero, 0x00);
	IDT::installHandler((void*)&aihErrSingleStepDebug, 0x01);
	IDT::installHandler((void*)&aihErrNMIPin, 0x02);
	IDT::installHandler((void*)&aihErrBreakpointDebug, 0x03);
	IDT::installHandler((void*)&aihErrOverflow, 0x04);
	IDT::installHandler((void*)&aihErrBoundsRange, 0x05);
	IDT::installHandler((void*)&aihErrUndefinedOPCode, 0x06);
	IDT::installHandler((void*)&aihErrDoubleFault, 0x08);
	IDT::installHandler((void*)&aihErrInvalidTSS, 0x0A);
	IDT::installHandler((void*)&aihErrSegmentNotPresent, 0x0B);
	IDT::installHandler((void*)&aihErrStackSegOverun, 0x0C);
	IDT::installHandler((void*)&aihErrGeneralProtectionFault, 0x0D);
	IDT::installHandler((void*)&aihErrPageFault, 0x0E);
}

void Exceptions::throwException(const char* title, const char* details)
{
	Video::clearScreen();
	
	Video::printf("**EXCEPTION**%s*\n", title);
	Video::print(details);

	HAL::halt();
}