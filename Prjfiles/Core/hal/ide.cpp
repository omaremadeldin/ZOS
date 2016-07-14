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
//By Omar Emad Eldin
//==========================================

#include "ide.h"

#include "hal.h"
#include "pic.h"
#include "idt.h"

#include <string.h>

using namespace zos;

LinkedList<IDE::Controller::Channel::Device*> zos::IDE_DEVICES;

assembly_stub void aihIDE(void);
interrupt_handler void ihIDE(void)
{
	zos::ihIDE();
}

void IDE::Controller::Channel::Device::setNIEN(bool val)
{
	if (val)
		parentChannel->write(IDE_REG_CONTROL, IDE_CTRL_NIEN_ON);
	else
		parentChannel->write(IDE_REG_CONTROL, IDE_CTRL_NIEN_OFF);
	
	nIEN = val;
}

bool IDE::Controller::Channel::Device::isLBA48Supported()
{
	if ((CommandSets[1] & 0x400) == 0)
		return false;
		
	return true;
}

void IDE::Controller::Channel::Device::waitForIRQ()
{	
	IRQ = true;
	while (IRQ);
}

void zos::ihIDE()
{
	//Search for devices waiting IRQ because their operations are complete
	LinkedList<IDE::Controller::Channel::Device*>::Node* n = IDE_DEVICES.head;
	while (n != NULL)
	{
		if (n->value->IRQ)
		{
			n->value->IRQ = false;
			break;
		}
		
		n = n->nextNode;
	}
}

IDE::Controller::Channel::Device::Device()
{
	parentChannel = NULL;
	Master = true;
	IRQ = false;
	Available = false;
	nIEN = false;
}

bool IDE::Controller::Channel::Device::identify()
{
	//512-Bytes is the identification buffer size
	uint16_t buffer[256];
	memset(&buffer, 0x00, 512);
	
	//Initialize available to false till we make sure of device availablity
	Available = false;
	
	parentChannel->selectDevice(Master);
	
	//Status of 0xFF or 0x7F indicates an invalid device
	if (parentChannel->getAlternateStatus().value() == 0xFF || parentChannel->getAlternateStatus().value() == 0x7F)
		return false;
	
	//One of BSY or DRDY must be active at all times, if not it also indicates an invalid device
	if (parentChannel->getStatus().BSY == false && parentChannel->getStatus().DRDY == false)
		return false;
	
	//Disable interrupts
	setNIEN(true);
	
	//Wait for DRDY flag
	parentChannel->waitDRDY();
	
	//Issue the identify command
	parentChannel->write(IDE_REG_COMMAND, IDE_CMD_IDENTIFY);
	parentChannel->poll();
	
	//Wait for BSY
	parentChannel->waitBSY();
	
	//Check alternate status for error flag
	if (parentChannel->checkERR())
	{
		//Check for ABRT flag
		if (parentChannel->getError().ABRT)
		{
			//Invalid command so try the ATAPI identify command
			parentChannel->write(IDE_REG_COMMAND, IDE_CMD_IDENTIFY_PACKET);
			parentChannel->poll();
	
			//Wait for BSY
			parentChannel->waitBSY();
		}
		else
			return false;
	}
	
	//If there is an error again then this is an invalid device
	//If DRQ flag is not set then there is no device
	if ((parentChannel->checkERR()) || (!parentChannel->getStatus().DRQ))
		return false;
	
	//The device is indeed available
	Available = true;
	
	HAL::insportw(parentChannel->dataBAR + IDE_REG_DATA, 256, &buffer);
	
	if ((buffer[IDE_IDENT_GENERALCONF] & 0x8000) == 0x0000)
		deviceType = ATA;
	else
		deviceType = ATAPI;
	//Model String (40 ASCII Characters)
	uint8_t strI = 0;
	for (uint8_t i = 0; i < (40/2); i++)
	{
		char c1 = (char)((buffer[IDE_IDENT_MODEL + i] & 0xFF00) >> 8);
		char c2 = (char)(buffer[IDE_IDENT_MODEL + i] & 0xFF);
		
		Model[strI++] = c1;
		Model[strI++] = c2;
	}
	Model[40] = '\0';

	//Right Trimming Model String
	for (uint8_t i = 39; i > 0; i--)
	{
		if (Model[i] != ' ')
			break;
		
		Model[i] = '\0';
	}
	
	//Capabilities (2 WORDs)
	Capabilities[0] = buffer[IDE_IDENT_CAPABILITIES + 0];
	Capabilities[1] = buffer[IDE_IDENT_CAPABILITIES + 1];
	
	//Command Sets (6 WORDs)
	CommandSets[0] = buffer[IDE_IDENT_COMMANDSETS + 0];
	CommandSets[1] = buffer[IDE_IDENT_COMMANDSETS + 1];
	CommandSets[2] = buffer[IDE_IDENT_COMMANDSETS + 2];
	CommandSets[3] = buffer[IDE_IDENT_COMMANDSETS + 3];
	CommandSets[4] = buffer[IDE_IDENT_COMMANDSETS + 4];
	CommandSets[5] = buffer[IDE_IDENT_COMMANDSETS + 5];
	
	uint16_t LBA[4];
	
	if (!isLBA48Supported())
	{
		LBA[0] = buffer[IDE_IDENT_MAX_LBA + 0];
		LBA[1] = buffer[IDE_IDENT_MAX_LBA + 1];
	}
	else
	{
		LBA[0] = buffer[IDE_IDENT_MAX_LBA + 0];
		LBA[1] = buffer[IDE_IDENT_MAX_LBA + 1];
		LBA[2] = buffer[IDE_IDENT_MAX_LBA + 2];
		LBA[3] = buffer[IDE_IDENT_MAX_LBA + 3];
	}
	
	maxLBA = *((uint64_t*)&LBA);
	
	return true;
}

void IDE::Controller::Channel::Device::init(bool master, IDE::Controller::Channel* parent)
{
	parentChannel = parent;
	Master = master;
}

void IDE::Controller::Channel::Device::eject()
{
	if (!Available)
		return;
	
	if (deviceType != ATAPI)
		return;
	
	//Enable interrupts
	setNIEN(false);
	
	//ATAPI Packet
	uint8_t ATAPIPacket[12];
	ATAPIPacket[0] = IDE_ATAPI_CMD_EJECT;
	ATAPIPacket[1] = 0x00;
	ATAPIPacket[2] = 0x00;
	ATAPIPacket[3] = 0x00;
	ATAPIPacket[4] = 0x02;
	ATAPIPacket[5] = 0x00;
	ATAPIPacket[6] = 0x00;
	ATAPIPacket[7] = 0x00;
	ATAPIPacket[8] = 0x00;
	ATAPIPacket[9] = 0x00;
	ATAPIPacket[10] = 0x00;
	ATAPIPacket[11] = 0x00;
	
	parentChannel->selectDevice(Master);
	
	//Send the packet command
	parentChannel->write(IDE_REG_COMMAND, IDE_CMD_PACKET);
	
	parentChannel->poll();
	
	HAL::outsportw(parentChannel->dataBAR + IDE_REG_DATA, sizeof(ATAPIPacket) / 2, &ATAPIPacket);
	waitForIRQ();
	
	if (parentChannel->checkERR())
		return;
	
	parentChannel->poll();
	
	if (!parentChannel->getStatus().DRQ)
		return;
	
	//Disable interrupts
	setNIEN(true);
}

void IDE::Controller::Channel::Device::readData(uint64_t LBA, uint16_t noOfSectors, void* buffer, bool useDMA)
{
	if (!Available)
		return;
	
	if (deviceType == ATA)
	{
		uint8_t LBA0  = 0;
		uint8_t LBA1  = 0;
		uint8_t LBA2  = 0;
		uint8_t LBA3  = 0;
		uint8_t LBA4  = 0;
		uint8_t LBA5  = 0;
		uint8_t SECCOUNT0 = 0;
		uint8_t SECCOUNT1 = 0;
		
		bool LBA48 = isLBA48Supported();
		
		if (!LBA48)
		{
			LBA0  = (LBA & 0xFF);
			LBA1  = ((LBA & 0xFF00) >> 8);
			LBA2  = ((LBA & 0xFF0000) >> 16);
			SECCOUNT0 = (noOfSectors & 0xFF);
		}
		else
		{
			LBA0  = (LBA & 0xFF);
			LBA1  = ((LBA & 0xFF00) >> 8);
			LBA2  = ((LBA & 0xFF0000) >> 16);
			LBA3  = ((LBA & 0xFF000000) >> 24);
			LBA4  = ((LBA & 0xFF000000) >> 32);
			LBA5  = ((LBA & 0xFF00000000) >> 40);
			SECCOUNT0 = (noOfSectors & 0xFF);
			SECCOUNT1 = ((noOfSectors & 0xFF00) >> 8);
		}
		
		parentChannel->selectDevice(Master);
		
		if (LBA48)
		{
			parentChannel->write(IDE_REG_SECCOUNT1, SECCOUNT1);
			parentChannel->write(IDE_REG_LBA3, LBA3);
			parentChannel->write(IDE_REG_LBA4, LBA4);
			parentChannel->write(IDE_REG_LBA5, LBA5);
		}
		
		parentChannel->write(IDE_REG_SECCOUNT0, SECCOUNT0);
		parentChannel->write(IDE_REG_LBA0, LBA0);
		parentChannel->write(IDE_REG_LBA1, LBA1);
		parentChannel->write(IDE_REG_LBA2, LBA2);
		
		uint8_t cmd = 0;
		
		//Choose between diffrent commands
		if (!LBA48 && !useDMA)
			cmd = IDE_CMD_READ_PIO;			//LBA28 PIO Read
	   	if (LBA48 && !useDMA)
		   	cmd = IDE_CMD_READ_PIO_EXT;   	//LBA48 PIO Read
	   	if (!LBA48 && useDMA)
		   	cmd = IDE_CMD_READ_DMA;			//LBA28 DMA Read
	   	if (LBA48 && useDMA)
		   	cmd = IDE_CMD_READ_DMA_EXT;		//LBA48 DMA Read
		
		parentChannel->write(IDE_REG_COMMAND, cmd);
		
		if (useDMA)
		{
			//TODO:DMA Support
		}
		else
		{
			for (uint16_t i = 0; i < noOfSectors; i++)
			{
				parentChannel->poll();

				parentChannel->waitBSY();
				
				if (parentChannel->checkERR())
					return;

				if (parentChannel->checkDF())
					return;

				if (!parentChannel->getStatus().DRQ)
					return;
		
				HAL::insportw(parentChannel->dataBAR + IDE_REG_DATA, IDE_ATA_BYTESPERSECTOR / 2, buffer);
				buffer = ((uint8_t*)buffer + IDE_ATA_BYTESPERSECTOR);
			}
		}
	}
	else
	{
		//Enable interrupts
		setNIEN(false);
		
		//ATAPI Packet
		uint8_t ATAPIPacket[12];
		ATAPIPacket[0] = IDE_ATAPI_CMD_READ;
	    ATAPIPacket[1] = 0x00;
		ATAPIPacket[2] = ((LBA & 0xFF000000) >> 24);
		ATAPIPacket[3] = ((LBA & 0xFF0000) >> 16);
		ATAPIPacket[4] = ((LBA & 0xFF00) >> 8);
		ATAPIPacket[5] = (LBA & 0xFF);
		ATAPIPacket[6] = 0x00;
		ATAPIPacket[7] = 0x00;
		ATAPIPacket[8] = 0x00;
		ATAPIPacket[9] = (noOfSectors & 0xFF);
		ATAPIPacket[10] = 0x00;
		ATAPIPacket[11] = 0x00;
		
		parentChannel->selectDevice(Master);
		
		//Send to the features register (1st Bit 0=PIO, 1=DMA)
		parentChannel->write(IDE_REG_FEATURES, useDMA);
		
		//Send the sector size to the controller
		parentChannel->write(IDE_REG_LBA1, (IDE_ATAPI_BYTESPERSECTOR & 0xFF));
		parentChannel->write(IDE_REG_LBA2, ((IDE_ATAPI_BYTESPERSECTOR & 0xFF00) >> 8));
		
		//Send the packet command
		parentChannel->write(IDE_REG_COMMAND, IDE_CMD_PACKET);
		
		parentChannel->poll();

		HAL::outsportw(parentChannel->dataBAR + IDE_REG_DATA, sizeof(ATAPIPacket) / 2, &ATAPIPacket);
		waitForIRQ();
		
		for (uint16_t i = 0; i < noOfSectors; i++)
		{			
			parentChannel->poll();

			parentChannel->waitBSY();
			
			if (parentChannel->checkERR())
				return;

			if (parentChannel->checkDF())
				return;

			if (!parentChannel->getStatus().DRQ)
				return;
	
			HAL::insportw(parentChannel->dataBAR + IDE_REG_DATA, IDE_ATAPI_BYTESPERSECTOR / 2, buffer);
			buffer = ((uint8_t*)buffer + IDE_ATAPI_BYTESPERSECTOR);
		}
		
		//Disable interrupts
		setNIEN(true);
	}
}

void IDE::Controller::Channel::Device::writeData(uint64_t LBA, uint16_t noOfSectors, void* buffer, bool useDMA)
{
	if (!Available)
		return;
	
	if (deviceType == ATAPI)
		return;
	
	uint8_t LBA0  = 0;
	uint8_t LBA1  = 0;
	uint8_t LBA2  = 0;
	uint8_t LBA3  = 0;
	uint8_t LBA4  = 0;
	uint8_t LBA5  = 0;
	uint8_t SECCOUNT0 = 0;
	uint8_t SECCOUNT1 = 0;
	
	bool LBA48 = isLBA48Supported();
	
	if (!LBA48)
	{
		LBA0  = (LBA & 0xFF);
		LBA1  = ((LBA & 0xFF00) >> 8);
		LBA2  = ((LBA & 0xFF0000) >> 16);
		SECCOUNT0 = (noOfSectors & 0xFF);
	}
	else
	{
		LBA0  = (LBA & 0xFF);
		LBA1  = ((LBA & 0xFF00) >> 8);
		LBA2  = ((LBA & 0xFF0000) >> 16);
		LBA3  = ((LBA & 0xFF000000) >> 24);
		LBA4  = ((LBA & 0xFF000000) >> 32);
		LBA5  = ((LBA & 0xFF00000000) >> 40);
		SECCOUNT0 = (noOfSectors & 0xFF);
		SECCOUNT1 = ((noOfSectors & 0xFF00) >> 8);
	}
	
	parentChannel->selectDevice(Master);
	
	if (LBA48)
	{
		parentChannel->write(IDE_REG_SECCOUNT1, SECCOUNT1);
		parentChannel->write(IDE_REG_LBA3, LBA3);
		parentChannel->write(IDE_REG_LBA4, LBA4);
		parentChannel->write(IDE_REG_LBA5, LBA5);
	}
	
	parentChannel->write(IDE_REG_SECCOUNT0, SECCOUNT0);
	parentChannel->write(IDE_REG_LBA0, LBA0);
	parentChannel->write(IDE_REG_LBA1, LBA1);
	parentChannel->write(IDE_REG_LBA2, LBA2);
	
	uint8_t cmd = 0;
	
	//Choose between diffrent commands
	if (!LBA48 && !useDMA)
		cmd = IDE_CMD_WRITE_PIO;		//LBA28 PIO Write
	if (LBA48 && !useDMA)
		cmd = IDE_CMD_WRITE_PIO_EXT;	//LBA48 PIO Write
	if (!LBA48 && useDMA)
		cmd = IDE_CMD_WRITE_DMA;		//LBA28 DMA Write
	if (LBA48 && useDMA)
		cmd = IDE_CMD_WRITE_DMA_EXT;	//LBA48 DMA Write
	
	parentChannel->write(IDE_REG_COMMAND, cmd);
	
	if (useDMA)
	{
		//TODO:DMA Support
	}
	else
	{
		for (uint16_t i = 0; i < noOfSectors; i++)
		{
			parentChannel->poll();

			parentChannel->waitBSY();
			
			if (parentChannel->checkERR())
				return;

			if (parentChannel->checkDF())
				return;

			if (!parentChannel->getStatus().DRQ)
				return;
	
			HAL::outsportw(parentChannel->dataBAR + IDE_REG_DATA, IDE_ATA_BYTESPERSECTOR / 2, buffer);
			buffer = ((uint8_t*)buffer + IDE_ATA_BYTESPERSECTOR);
		}
		
		if (!LBA48)
			parentChannel->write(IDE_REG_COMMAND, IDE_CMD_CACHE_FLUSH);
		else
			parentChannel->write(IDE_REG_COMMAND, IDE_CMD_CACHE_FLUSH_EXT);
	}
}

uint8_t IDE::Controller::Channel::read(uint8_t reg)
{
	uint32_t address = 0;
	
	if (reg < 0x08)
		address = dataBAR + reg;
	else if (reg < 0x0C)
		address = dataBAR + reg - 0x06;
	else if (reg < 0x0E)
		address = ctrlBAR + reg - 0x0A;
	//else
		//throwSoftwareFault("IDE: Invalid register sent to ideRead");
	
	return HAL::inportb(address);
}

void IDE::Controller::Channel::write(uint8_t reg, uint8_t val)
{
	uint32_t address = 0;
	
	if (reg < 0x08)
		address = dataBAR + reg;
	else if (reg < 0x0C)
		address = dataBAR + reg - 0x06;
	else if (reg < 0x0E)
		address = ctrlBAR + reg - 0x0A;
	//else
		//throwSoftwareFault("IDE: Invalid register sent to ideWrite");	
	
	HAL::outportb(address, val);
}

void IDE::Controller::Channel::selectDevice(bool master)
{
	if (master)
		write(IDE_REG_HDDEVSEL, IDE_HDDEVSEL_LBA_MASTER);
	else
		write(IDE_REG_HDDEVSEL, IDE_HDDEVSEL_LBA_SLAVE);
	
	poll();
}

IDE::Controller::Channel::Status IDE::Controller::Channel::getStatus()
{
	uint8_t val = read(IDE_REG_STATUS);
	return *((Status*)&val);
}

IDE::Controller::Channel::Status IDE::Controller::Channel::getAlternateStatus()
{
	uint8_t val = read(IDE_REG_ALTSTATUS);
	return *((Status*)&val);
}

IDE::Controller::Channel::Error IDE::Controller::Channel::getError()
{
	uint8_t val = read(IDE_REG_ERROR);
	return *((Error*)&val);
}

void IDE::Controller::Channel::waitBSY()
{
	Status sts = getStatus();

		while (sts.BSY)
			sts = getStatus();
}

void IDE::Controller::Channel::waitDRDY()
{
	Status sts = getStatus();

		while (!sts.DRDY)
			sts = getStatus();
}

bool IDE::Controller::Channel::checkERR()
{
	return getAlternateStatus().ERR;
}

bool IDE::Controller::Channel::checkDF()
{
	return getStatus().DF;
}

void IDE::Controller::Channel::poll()
{
	for(uint8_t i = 0; i < 4; i++)
		read(IDE_REG_ALTSTATUS);
	
	for(uint8_t i = 0; i < 4; i++)
		read(IDE_REG_ALTSTATUS);
	
	for(uint8_t i = 0; i < 4; i++)
		read(IDE_REG_ALTSTATUS);
	
	for(uint8_t i = 0; i < 4; i++)
		read(IDE_REG_ALTSTATUS);
	
	//if (checkDF())
		//return throwSoftwareFault("IDE: Device Fault");
}

IDE::Controller::Channel::Channel()
{
	dataBAR = 0x00;
	ctrlBAR = 0x00;
	busmasterBAR = 0x00;
	parentController = NULL;
}

void IDE::Controller::Channel::init(uint32_t dataBAR, uint32_t ctrlBAR, uint32_t busmasterBAR, IDE::Controller* parent)
{	
	this->dataBAR = dataBAR;
	this->ctrlBAR = ctrlBAR;
	this->busmasterBAR = busmasterBAR;
	this->parentController = parent;

	masterDevice.init(true, this);
	slaveDevice.init(false, this);
	
	//Identify the device and fill in its details
	if (masterDevice.identify())
		IDE_DEVICES.add(&masterDevice);
	
	//Identify the device and fill in its details
	if (slaveDevice.identify())
		IDE_DEVICES.add(&slaveDevice);
}

IDE::Controller::Controller(const PCI::Device& pciDevice) : PCI::Device(pciDevice)
{}

void IDE::Controller::init()
{	
	//Get the BARs (4-Byte aligned)
	uint32_t BAR0 = getBAR0() & 0xFFFFFFFC;
	uint32_t BAR1 = getBAR1() & 0xFFFFFFFC;
	uint32_t BAR2 = getBAR2() & 0xFFFFFFFC;
	uint32_t BAR3 = getBAR3() & 0xFFFFFFFC;
	uint32_t BAR4 = getBAR4() & 0xFFFFFFFC;
	uint8_t  INTLine = getINTLine();
	
	//If BARs are empty, fill them with the default values
	BAR0 = ((BAR0 == 0x00) ? IDE_DEFAULT_BAR0 : BAR0);
	BAR1 = ((BAR1 == 0x00) ? IDE_DEFAULT_BAR1 : BAR1);
	BAR2 = ((BAR2 == 0x00) ? IDE_DEFAULT_BAR2 : BAR2);
	BAR3 = ((BAR3 == 0x00) ? IDE_DEFAULT_BAR3 : BAR3);	
	
	//Install interrupt handlers
	if ((progIF == IDE_PATA_PROGIF1 || progIF == IDE_PATA_PROGIF2) &&
		(INTLine == 0x00 || INTLine == 0xFF))
	{
		IDT::installHandler((void*)&aihIDE, PIC::IRQ0 + IDE_DEFAULT_INTLINE1);
		IDT::installHandler((void*)&aihIDE, PIC::IRQ0 + IDE_DEFAULT_INTLINE2);
	}
	else
	{
		IDT::installHandler((void*)&aihIDE, PIC::IRQ0 + INTLine);
	}
	
	//Initialize primary channel
	primaryChannel.init(BAR0, BAR1, BAR4, this);
	
	//Initialize secondary channel
	secondaryChannel.init(BAR2, BAR3, BAR4 + 0x08, this);
}

void IDE::scan()
{
	PCI::Device* ideCtrl = PCI::getFirstDeviceOfClass(PCI::Device::Classes::MassStorageController, PCI::Device::Subclasses::IDEController);
	
	while (ideCtrl != NULL)
	{
		Controller* ctrl = new Controller(*ideCtrl);
		ctrl->init();
		
		ideCtrl = PCI::getFirstDeviceOfClass(PCI::Device::Classes::MassStorageController, PCI::Device::Subclasses::IDEController, ideCtrl);
	}
}