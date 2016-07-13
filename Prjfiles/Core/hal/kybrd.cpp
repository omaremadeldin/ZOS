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

#include "kybrd.h"

#include "hal.h"
#include "idt.h"
#include "pic.h"

using namespace zos;

bool KYBRD::disabled = false;

bool KYBRD::numLock = false;
bool KYBRD::capsLock = false;
bool KYBRD::scrollLock = false;

bool KYBRD::modifierCtrl = false;
bool KYBRD::modifierAlt = false;
bool KYBRD::modifierShift = false;

KEYCODE KYBRD::lastKeyCode = KEY_UNKNOWN;

const KEYCODE KYBRD::scanCodes[] =
{
	KEY_UNKNOWN,			//0x00
	KEY_ESCAPE,				//0x01
	KEY_1,					//0x02
	KEY_2,					//0x03
	KEY_3,					//0x04
	KEY_4,					//0x05
	KEY_5,					//0x06
	KEY_6,					//0x07
	KEY_7,					//0x08
	KEY_8,					//0x09
	KEY_9,					//0x0A
	KEY_0,					//0x0B
	KEY_MINUS,				//0x0C
	KEY_PLUS,				//0x0D
	KEY_BACKSPACE,			//0x0E
	KEY_TAB,				//0x0F
	KEY_Q,					//0x10
	KEY_W,					//0x11
	KEY_E,					//0x12
	KEY_R,					//0x13
	KEY_T,					//0x14
	KEY_Y,					//0x15
	KEY_U,					//0x16
	KEY_I,					//0x17
	KEY_O,					//0x18
	KEY_P,					//0x19
	KEY_LEFTBRACKET,		//0x1A
	KEY_RIGHTBRACKET,		//0x1B
	KEY_RETURN,				//0x1C
	KEY_LCTRL,				//0x1D
	KEY_A,					//0x1E
	KEY_S,					//0x1F
	KEY_D,					//0x20
	KEY_F,					//0x21
	KEY_G,					//0x22
	KEY_H,					//0x23
	KEY_J,					//0x24
	KEY_K,					//0x25
	KEY_L,					//0x26
	KEY_SEMICOLON,			//0x27
	KEY_QUOTE,				//0x28
	KEY_GRAVE,				//0x29
	KEY_LSHIFT,				//0x2A
	KEY_BACKSLASH,			//0x2B
	KEY_Z,					//0x2C
	KEY_X,					//0x2D
	KEY_C,					//0x2E
	KEY_V,					//0x2F
	KEY_B,					//0x30
	KEY_N,					//0x31
	KEY_M,					//0x32
	KEY_COMMA,				//0x33
	KEY_DOT,				//0x34
	KEY_SLASH,				//0x35
	KEY_RSHIFT,				//0x36
	KEY_KP_ASTERISK,		//0x37
	KEY_RALT,				//0x38
	KEY_SPACE,				//0x39
	KEY_CAPSLOCK,			//0x3A
	KEY_F1,					//0x3B
	KEY_F2,					//0x3C
	KEY_F3,					//0x3D
	KEY_F4,					//0x3E
	KEY_F5,					//0x3F
	KEY_F6,					//0x40
	KEY_F7,					//0x41
	KEY_F8,					//0x42
	KEY_F9,					//0x43
	KEY_F10,				//0x44
	KEY_KP_NUMLOCK,			//0x45
	KEY_SCROLLLOCK,			//0x46
	KEY_HOME,				//0x47
	KEY_KP_8,				//0x48
	KEY_PAGEUP,				//0x49
	KEY_KP_2,				//0x50
	KEY_KP_3,				//0x51
	KEY_KP_0,				//0x52
	KEY_KP_DECIMAL,			//0x53
	KEY_UNKNOWN,			//0x54
	KEY_UNKNOWN,			//0x55
	KEY_UNKNOWN,			//0x56
	KEY_F11,				//0x57
	KEY_F12					//0x58
};

assembly_stub void aihKYBRD(void);
interrupt_handler void ihKYBRD(void)
{
	zos::ihKYBRD();
}

uint8_t KYBRD::readEncoderBuffer()
{
	return HAL::inportb((uint16_t)KYBRD_ENC_INPUT_BUF);
}

KYBRD::Status KYBRD::readCtrlStatus()
{
	uint8_t val = HAL::inportb(KYBRD_CTRL_STS_REG);
	return *((Status*)(&val));
}

void KYBRD::waitForInput()
{
	//Wait for input buffer to be empty
	while (true)
		if (readCtrlStatus().inputFull == false)
			break;
}

void KYBRD::waitForOutput()
{
	//Wait for output buffer to be full
	while (true)
		if (readCtrlStatus().outputFull == true)
			break;
}

void KYBRD::sendCommandToCtrl(uint8_t cmd)
{
	//Wait for input buffer to be empty
	waitForInput();
	
	HAL::outportb(KYBRD_CTRL_CMD_REG, cmd);
}

void KYBRD::sendCommandToEncoder(uint8_t cmd)
{
	//Wait for input buffer to be empty
	waitForInput();
	
	HAL::outportb(KYBRD_ENC_CMD_REG, cmd);
}

KEYCODE KYBRD::scanCodeToKeyCode(uint8_t scancode)
{
	return scanCodes[scancode];
}

KEYCODE KYBRD::extendedScanCodeToKeyCode(uint8_t exscancode)
{
	switch (exscancode)
	{
		case 0x47:
			return KEY_HOME;
		case 0x48:
			return KEY_UP;
		case 0x49:
			return KEY_PAGEUP;
		case 0x4B:
			return KEY_LEFT;
		case 0x4D:
			return KEY_RIGHT;
		case 0x4F:
			return KEY_END;
		case 0x50:
			return KEY_DOWN;
		case 0x51:
			return KEY_PAGEDOWN;
		case 0x52:
			return KEY_INSERT;
		case 0x53:
			return KEY_DELETE;
		default:
			return KEY_UNKNOWN;
	}
}

KEYCODE KYBRD::getLastKey()
{
	return lastKeyCode;
}

void KYBRD::discardLastKey()
{
	lastKeyCode = KEY_UNKNOWN;
}

char KYBRD::keyToASCII(KEYCODE code)
{
	uint8_t key = code;
	
	if (isascii(code))
	{
		if (modifierShift || capsLock)
			key = toupper(key);
				
		if (modifierShift && !capsLock)
		{
			if (isdigit(key))
			{
				switch (key)
				{
					case '1':
						key = KEY_EXCLAMATION;
						break;
					case '2':
						key = KEY_AT;
						break;
					case '3':
						key = KEY_HASH;
						break;
					case '4':
						key = KEY_DOLLAR;
						break;
					case '5':
						key = KEY_PERCENT;
						break;
					case '6':
						key = KEY_CARRET;
						break;
					case '7':
						key = KEY_AMPERSAND;
						break;
					case '8':
						key = KEY_ASTERISK;
						break;
					case '9':
						key = KEY_LEFTPARENTHESIS;
						break;
					case '0':
						key = KEY_RIGHTPARENTHESIS;
						break;
				}
			}
			else
			{
				switch (key)
				{
					case KEY_COMMA:
						key = KEY_LESS;
						break;
					case KEY_DOT:
						key = KEY_GREATER;
						break;
					case KEY_SLASH:
						key = KEY_QUESTION;
						break;
					case KEY_SEMICOLON:
						key = KEY_COLON;
						break;
					case KEY_QUOTE:
						key = KEY_QUOTEDOUBLE;
						break;
					case KEY_LEFTBRACKET :
						key = KEY_LEFTCURL;
						break;
					case KEY_RIGHTBRACKET :
						key = KEY_RIGHTCURL;
						break;
					case KEY_GRAVE:
						key = KEY_TILDE;
						break;
					case KEY_MINUS:
						key = KEY_UNDERSCORE;
						break;
					case KEY_PLUS:
						key = KEY_EQUAL;
						break;
					case KEY_BACKSLASH:
						key = KEY_BAR;
						break;
				}
			}
		}
		
		return key;
	}
	
	return 0;
}

void KYBRD::setLEDs(bool num, bool caps, bool scroll)
{
	//1 = on, 0 = off
	//Scroll lock (bit 0)
	//Num lock (bit 1)
	//Caps lock (bit 2)
	uint8_t data = 0;
	
	data = (scroll) ? (data | 1):(data & 0xFE);
	data = (num) ? (data | 2):(data & 0xFD);
	data = (caps) ? (data | 4):(data & 0xFB);
	
	sendCommandToEncoder(KYBRD_ENC_CMD_LED);
	sendCommandToEncoder(data);
}

void KYBRD::updateLEDs()
{
	setLEDs(numLock, capsLock, scrollLock);
}

bool KYBRD::selfTest()
{
	sendCommandToCtrl(KYBRD_CTRL_CMD_SELFTEST);
	
	//Wait for output buffer to be full
	waitForOutput();
	
	return ((readEncoderBuffer() == 0x55) ? true : false);
}

void zos::ihKYBRD()
{
	static bool extendedCode = false;
	uint8_t code = 0;
	
	//only process code if output buffer is full (has scan code in it)
	if (KYBRD::readCtrlStatus().outputFull == true)
	{
		//Read scan code from encoder buffer
		code = KYBRD::readEncoderBuffer();

		if (code == 0xE0 || code == 0xE1)
			extendedCode = true;
		else
		{	
			//Test if this is a break code
			if (code & 0x80)
			{
				//Convert break code to its make code equivalent
				code &= 0x7F;
				
				KEYCODE key = (extendedCode) ? KYBRD::extendedScanCodeToKeyCode(code) : KYBRD::scanCodeToKeyCode(code);
				
				switch (key)
				{
					case KEY_LCTRL:
					case KEY_RCTRL:
						KYBRD::modifierCtrl = false;
						break;
					case KEY_LALT:
					case KEY_RALT:
						KYBRD::modifierAlt = false;
						break;
					case KEY_LSHIFT:
					case KEY_RSHIFT:
						KYBRD::modifierShift = false;
						break;
					default:
						break;
				}
			}
			else		//Not a break code (make code)
			{
				KEYCODE key = (extendedCode) ? KYBRD::extendedScanCodeToKeyCode(code) : KYBRD::scanCodeToKeyCode(code);
				
				KYBRD::lastKeyCode = key;

				switch (key)
				{
					case KEY_LCTRL:
					case KEY_RCTRL:
						KYBRD::modifierCtrl = true;
						break;
					case KEY_LALT:
					case KEY_RALT:
						KYBRD::modifierAlt = true;
						break;
					case KEY_LSHIFT:
					case KEY_RSHIFT:
						KYBRD::modifierShift = true;
						break;
					case KEY_KP_NUMLOCK:
						KYBRD::numLock = !KYBRD::numLock;
						KYBRD::updateLEDs();
						break;
					case KEY_CAPSLOCK:
						KYBRD::capsLock = !KYBRD::capsLock;
						KYBRD::updateLEDs();
						break;
					case KEY_SCROLLLOCK:
						KYBRD::scrollLock = !KYBRD::scrollLock;
						KYBRD::updateLEDs();
						break;
					default:
						break;
				}
			}
			
			extendedCode = false;
		}
	}
	
	PIC::sendEOI(KYBRD_IRQ_NO);
}

void KYBRD::disable()
{
	sendCommandToCtrl(KYBRD_CTRL_CMD_DISABLEKYBRD);
	disabled = true;
}

void KYBRD::enable()
{
	sendCommandToCtrl(KYBRD_CTRL_CMD_ENABLEKYBRD);
	disabled = false;
}

KEYCODE KYBRD::getKey()
{
	KEYCODE key = KEY_UNKNOWN;
	
	while (key == KEY_UNKNOWN)
		key = getLastKey();
	
	discardLastKey();
	
	return key;
}

//TODO: only returns when user enters valid character
char KYBRD::getCh()
{
	KEYCODE key = KEY_UNKNOWN;
	
	while (key == KEY_UNKNOWN)
		key = getLastKey();
	
	discardLastKey();
	
	return keyToASCII(key);
}

void KYBRD::systemReset()
{
	sendCommandToCtrl(KYBRD_CTRL_CMD_SYSTEMRESET);
}

void KYBRD::init()
{	
	IDT::installHandler((void*)&aihKYBRD, PIC::IRQ0 + KYBRD_IRQ_NO);
	
	updateLEDs();
}