//==========================================
//
//	 	  		ZapperOS - IDE
//      (Integrated Drive Electronics)
//==========================================
//Used Acronyms:
//--------------
//* BAR = Base Address Register
//* CHS = Cylinder - Head - Sector
//* LBA = Linear Block Addressing
//* IRQ = Interrupt Request
//* PIO = Programmed Input/Output
//* DMA = Direct Memory Access
//==========================================
//TODO:
//* Advanced Error Handling with error codes
//	or types of errors
//* Handle null errors
//==========================================
//By Omar Emad Eldin
//==========================================

#pragma once

#include <stdlib.h>

#include "pci.h"

#include "utils/bitfield.h"
#include "utils/linkedlist.h"

#define IDE_DEFAULT_BAR0	0x1F0
#define IDE_DEFAULT_BAR1	0x3F4
#define IDE_DEFAULT_BAR2	0x170
#define IDE_DEFAULT_BAR3	0x374

#define IDE_DEFAULT_INTLINE1	0x0E
#define IDE_DEFAULT_INTLINE2	0x0F

#define IDE_PATA_PROGIF1	0x80
#define IDE_PATA_PROGIF2	0x8A

//ATA Registers
#define IDE_REG_DATA		0x00
#define IDE_REG_ERROR		0x01	//Read Only
#define IDE_REG_FEATURES	0x01	//Write Only
#define IDE_REG_SECCOUNT0	0x02
#define IDE_REG_LBA0		0x03
#define IDE_REG_LBA1		0x04
#define IDE_REG_LBA2		0x05
#define IDE_REG_HDDEVSEL	0x06
#define IDE_REG_STATUS		0x07	//Read Only
#define IDE_REG_COMMAND		0x07	//Write Only
#define IDE_REG_SECCOUNT1	0x08
#define IDE_REG_LBA3		0x09
#define IDE_REG_LBA4		0x0A
#define IDE_REG_LBA5		0x0B
#define IDE_REG_ALTSTATUS	0x0C	//Read Only
#define IDE_REG_CONTROL		0x0C	//Write Only

#define IDE_CTRL_NIEN_OFF	0x00
#define IDE_CTRL_NIEN_ON	0x02

//IDE Commands
#define IDE_CMD_READ_PIO		0x20
#define IDE_CMD_READ_PIO_EXT	0x24
#define IDE_CMD_READ_DMA		0xC8
#define IDE_CMD_READ_DMA_EXT	0x25
#define IDE_CMD_WRITE_PIO		0x30
#define IDE_CMD_WRITE_PIO_EXT	0x34
#define IDE_CMD_WRITE_DMA		0xCA
#define IDE_CMD_WRITE_DMA_EXT	0x35
#define IDE_CMD_CACHE_FLUSH		0xE7
#define IDE_CMD_CACHE_FLUSH_EXT	0xEA
#define IDE_CMD_PACKET			0xA0
#define IDE_CMD_IDENTIFY_PACKET	0xA1
#define IDE_CMD_IDENTIFY 		0xEC

#define IDE_ATAPI_CMD_READ	0xA8
#define IDE_ATAPI_CMD_EJECT	0x1B

//HDDEVSEL Values
#define IDE_HDDEVSEL_CHS_MASTER	0xA0
#define IDE_HDDEVSEL_LBA_MASTER	0xE0
#define IDE_HDDEVSEL_CHS_SLAVE	0xB0
#define IDE_HDDEVSEL_LBA_SLAVE	0xF0

//IDENTIFY offsets (in WORDs)
#define IDE_IDENT_GENERALCONF  	0		//16-Bit value (last Bit = IDE/IDEPI (0/1))
#define IDE_IDENT_SERIAL       	10		//20 ASCII Characters
#define IDE_IDENT_FIRMWARE		23		//8 ASCII Characters
#define IDE_IDENT_MODEL        	27		//40 ASCII Characters
#define IDE_IDENT_CAPABILITIES 	49		//32-Bit value
#define IDE_IDENT_MAX_LBA      	60		//32-Bit value
#define IDE_IDENT_COMMANDSETS  	82		//12-Byte (6 WORDs)
#define IDE_IDENT_MAX_LBA_EXT  	100		//8-Byte (4 WORDs)

#define IDE_ATA_BYTESPERSECTOR		512
#define IDE_ATAPI_BYTESPERSECTOR	2048

namespace zos
{
	class IDE
	{
	public:
		class Controller : public PCI::Device
		{
			public:
				class Channel
				{
					public:
						class Device
						{
							public:
								enum Type : uint8_t
								{
									ATA,
									ATAPI
								};
							
							private:
								Channel* parentChannel;
								bool Master;
								bool IRQ;
								
							public:
								bool Available;
								Type deviceType;
								bool nIEN;
								char Model[41];
								uint16_t Capabilities[2];
								uint16_t CommandSets[6];
								uint64_t maxLBA;
							
							private:
								void setNIEN(bool val);
								bool isLBA48Supported();
								void waitForIRQ();
								
							private:
								friend void ihIDE();
							
							public:
								Device();									
								bool identify();
								void init(bool master, Channel* parent);
								void eject();
								void readData(uint64_t LBA, uint16_t noOfSectors, void* buffer, bool useDMA = false);
								void writeData(uint64_t LBA, uint16_t noOfSectors, void* buffer, bool useDMA = false);
						};
					
					private:
						struct Status : BitField<uint8_t>
						{
							bool	ERR	:1;		//Error
							uint8_t		:2;		//Obsolute Bits
							bool	DRQ	:1;		//Data Request
							bool	DSC	:1;		//Command Dependent
							bool	DF	:1;		//Device Fault
							bool	DRDY:1;		//Device Ready
							bool	BSY	:1;		//Busy
						}__attribute__((packed));

						struct Error : BitField<uint8_t>
						{
							bool	AMNF	:1;	//Command Dependent
							bool	TK0NF	:1;	//Command Dependent
							bool	ABRT	:1;	//Command aborted due to invalid arguments
							bool	MCR		:1;	//Command Dependent
							bool	IDNF	:1;	//Command Dependent
							bool	MC		:1;	//Command Dependent
							bool	UNC		:1;	//Command Dependent
							bool	BBK		:1;	//Command Dependent
						}__attribute__((packed));
						
					private:						
						Controller* parentController;
					
						uint32_t dataBAR;
						uint32_t ctrlBAR;
						uint32_t busmasterBAR;
						
						Device masterDevice;
						Device slaveDevice;
					
					private:
						uint8_t read(uint8_t reg);
						void write(uint8_t reg, uint8_t val);
						void selectDevice(bool master);
						Status getStatus();
						Status getAlternateStatus();
						Error getError();
						void waitBSY();
						void waitDRDY();
						bool checkERR();
						bool checkDF();
						void poll();
						
					public:
						Channel();
						void init(uint32_t dataBAR, uint32_t ctrlBAR, uint32_t busmasterBAR, Controller* parent);
				};
			
			private:
				Channel primaryChannel;
				Channel secondaryChannel;
			
			public:
				Controller(const PCI::Device& pciDevice);
				void init();
		};
		
	public:
		static void scan();
	};
	
	extern "C" LinkedList<IDE::Controller::Channel::Device*> IDE_DEVICES;
}