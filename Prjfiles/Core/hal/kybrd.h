//==========================================
//
//		  ZapperOS - Keyboard Driver
//
//==========================================
//Used Acronyms:
//--------------
//* IRQ = Interrupt Request
//* BAT = Basic Assurance Test
//* LED = Light Emitting Diode
//* ENC = Encoder
//==========================================
//By Omar Emad Eldin
//==========================================

#pragma once

#include <stdlib.h>

#include "misc/keycode.h"

#include "utils/bitfield.h"

#define KYBRD_IRQ_NO		0x01

#define KYBRD_ENC_INPUT_BUF	UINT16_C(0x60)
#define KYBRD_ENC_CMD_REG	UINT16_C(0x60)

#define KYBRD_CTRL_STS_REG	UINT16_C(0x64)
#define KYBRD_CTRL_CMD_REG	UINT16_C(0x64)

//KYBRD Encoder Commands
#define KYBRD_ENC_CMD_LED					0xED		//set LEDs
#define KYBRD_ENC_CMD_ECHO					0xEE		//Returns 0xEE to port 0x60 as a diagnostic test
#define KYBRD_ENC_CMD_ALTSCAN				0xF0		//Set alternate scan code set
#define KYBRD_ENC_CMD_SETAUTORPT			0xF3		//Set autorepeat delay and repeat rate
#define KYBRD_ENC_CMD_ENABLEKYBRD			0xF4		//Enable keyboard
#define KYBRD_ENC_CMD_PWR_WAITENABLE		0xF5		//Reset to power on condition and wait for enable command
#define KYBRD_ENC_CMD_PWR_BEGINSCAN			0xF6		//Reset to power on condition and begin scanning keyboard
#define KYBRD_ENC_CMD_ALL_ONLYMAKE			0xF9		//Set all keys to generate only make codes
#define KYBRD_ENC_CMD_ALL_AUTORPTMAKEBREAK	0xFA		//Set all keys to autorepeat and generate make and break codes
#define KYBRD_ENC_CMD_SNGL_AUTORPT			0xFB		//Set a single key to autorepeat
#define KYBRD_ENC_CMD_SNGL_MAKEBREAK		0xFC		//Set a single key to generate make and break codes
#define KYBRD_ENC_CMD_SNGL_ONLYBREAK		0xFD		//Set a single key to generate only break codes
#define KYBRD_ENC_CMD_RESEND				0xFE		//Resend last result
#define KYBRD_ENC_CMD_PWR_TEST				0xFF		//Reset keyboard to power on state and start self test

//KYBRD Controller Commands
#define KYBRD_CTRL_CMD_READCMD			0x20		//Read command byte
#define KYBRD_CTRL_CMD_WRITECMD			0x60		//Write command byte
#define KYBRD_CTRL_CMD_SELFTEST			0xAA		//Self test
#define KYBRD_CTRL_CMD_INTERFACETEST	0xAB		//Interface test
#define KYBRD_CTRL_CMD_DISABLEKYBRD		0xAD		//Disable keyboard
#define KYBRD_CTRL_CMD_ENABLEKYBRD		0xAE		//Enable keyboard
#define KYBRD_CTRL_CMD_READINPUT		0xC0		//Read input port
#define KYBRD_CTRL_CMD_READOUTPUT		0xD0		//Read ouput port
#define KYBRD_CTRL_CMD_WRITEOUTPUT		0xD1		//Write output port
#define KYBRD_CTRL_CMD_READTEST			0xE0		//Read test inputs
#define KYBRD_CTRL_CMD_SYSTEMRESET		0xFE		//System reset
#define KYBRD_CTRL_CMD_DISABLEMOUSE		0xA7		//Disable mouse port
#define KYBRD_CTRL_CMD_ENABLEMOUSE		0xA8		//Enable mouse port
#define KYBRD_CTRL_CMD_TESTMOUSE		0xA9		//Test mouse port
#define KYBRD_CTRL_CMD_WRITEMOUSE		0xD4		//Write to mouse

namespace zos
{
	class KYBRD
	{
	private:
		struct CommandByte : BitField<uint8_t>
		{
			bool 	keyboardInterrupt 	:1;		//Enable Keyboard Interrupts ?
			bool 	mouseInterrupt		:1;		//Enable Mouse Interrupts ?
			uint8_t systemFlag			:1;		//0 -> Cold Reboot, 1 -> Warm Reboot
			bool	ignoreLock			:1;		//Ignore Keyboard Lock ?
			bool	keyboardEnable		:1;		//Enable Keyboard ?
			bool	mouseEnable			:1;		//Enable Mouse ?
			bool	Translation			:1;		//Use Translation ?
		}__attribute__((packed));
		
		struct Status : BitField<uint8_t>
		{
			enum Action
			{
				ActionCommand = 0,
				ActionData = 1
			};
			
			bool 		outputFull 	:1;		//Output Full ?
			bool 		inputFull 	:1;		//Input Full ?
			bool 		BATSuccess 	:1;		//BAT Successful ?
			Action 		lastAction 	:1;		//Last Action
			bool 		Locked	 	:1;		//Keyboard Locked ?
			uint8_t					:3;		//Reserved
		}__attribute__((packed));
		
	private:
		//Keyboard disabled flag
		static bool disabled;
		
		//Keyboard lock flags
		static bool numLock;
		static bool capsLock;
		static bool scrollLock;
		
		//Keyboard modifier flags
		static bool modifierCtrl;
		static bool modifierAlt;
		static bool modifierShift;
		
		//Keyboard last keycode
		static KEYCODE lastKeyCode;
		
		static const KEYCODE scanCodes[];
		
	private:
		static Status readCtrlStatus();
		static uint8_t readEncoderBuffer();
		static void waitForInput();
		static void waitForOutput();
		static void sendCommandToCtrl(uint8_t cmd);
		static void sendCommandToEncoder(uint8_t cmd);
		static KEYCODE scanCodeToKeyCode(uint8_t scancode);
		static KEYCODE extendedScanCodeToKeyCode(uint8_t exscancode);
		static KEYCODE getLastKey();
		static void discardLastKey();
		static char keyToASCII(KEYCODE code);
		static void setLEDs(bool scroll, bool num, bool caps);
		static void updateLEDs();
		static bool selfTest();
		
	private:
		friend void ihKYBRD();
		
	public:
		static void disable();
		static void enable();
		static KEYCODE getKey();
		static char getCh();				
		static void systemReset();
		static void init();
	};
}