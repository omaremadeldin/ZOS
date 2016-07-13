//==========================================
//
//	 	  ZapperOS - Conventional PCI
//   (Peripheral Component Interconnect)
//==========================================
//Used Acronyms:
//--------------
//* PCI  	= Peripheral Component Interconnect
//* ProgIF 	= Programming Interface
//* BIST  	= Built-In Self Test
//* BAR		= Base Address Register
//==========================================
//By Omar Emad Eldin
//==========================================

#include "pci.h"

#include "hal.h"

using namespace zos;

PCI::Device::Class PCI::Device::Classes::MassStorageController = { 0x01, "Mass Storage Controller", NULL, NULL };
	PCI::Device::Class PCI::Device::Subclasses::SCSIBusController = { 0x00, "SCSI Bus Controller", &PCI::Device::Classes::MassStorageController, NULL };
	PCI::Device::Class PCI::Device::Subclasses::IDEController = { 0x01, "IDE Controller", &PCI::Device::Classes::MassStorageController, NULL };
	PCI::Device::Class PCI::Device::Subclasses::FloppyDiskController = { 0x02, "Floppy Disk Controller", &PCI::Device::Classes::MassStorageController, NULL };
	PCI::Device::Class PCI::Device::Subclasses::IPIBusController = { 0x03, "IPI Bus Controller", &PCI::Device::Classes::MassStorageController, NULL };
	PCI::Device::Class PCI::Device::Subclasses::RAIDController = { 0x04, "RAID Controller", &PCI::Device::Classes::MassStorageController, NULL };
	PCI::Device::Class PCI::Device::Subclasses::ATAController = { 0x05, "ATA Controller", &PCI::Device::Classes::MassStorageController, NULL };
	PCI::Device::Class PCI::Device::Subclasses::SerialATAController = { 0x06, "Serial ATA Controller (SATA)", &PCI::Device::Classes::MassStorageController, NULL };
	PCI::Device::Class PCI::Device::Subclasses::SerialAttachedSCSI = { 0x07, "Serial Attached SCSI (SAS)", &PCI::Device::Classes::MassStorageController, NULL };
	
PCI::Device::Class PCI::Device::Classes::NetworkController = { 0x02, "Network Controller", NULL, NULL };
	PCI::Device::Class PCI::Device::Subclasses::EthernetController = { 0x00, "Ethernet Controller", &PCI::Device::Classes::NetworkController, NULL };
	PCI::Device::Class PCI::Device::Subclasses::TokenRingController = { 0x01, "Token Ring Controller", &PCI::Device::Classes::NetworkController, NULL };
	PCI::Device::Class PCI::Device::Subclasses::FDDIController = { 0x02, "FDDI Controller", &PCI::Device::Classes::NetworkController, NULL };
	PCI::Device::Class PCI::Device::Subclasses::ATMController = { 0x03, "ATM Controller", &PCI::Device::Classes::NetworkController, NULL };
	PCI::Device::Class PCI::Device::Subclasses::ISDNController = { 0x04, "ISDN Controller", &PCI::Device::Classes::NetworkController, NULL };
	PCI::Device::Class PCI::Device::Subclasses::WorldFipController = { 0x05, "World Fip Controller", &PCI::Device::Classes::NetworkController, NULL };

PCI::Device::Class PCI::Device::Classes::DisplayController = { 0x03, "Display Controller", NULL, NULL };
	PCI::Device::Class PCI::Device::Subclasses::VGACompatibleController = { 0x00, "VGA-Compatible Controller", &PCI::Device::Classes::DisplayController , NULL };
	PCI::Device::Class PCI::Device::Subclasses::XGAController = { 0x01, "XGA Controller", &PCI::Device::Classes::DisplayController , NULL };
	PCI::Device::Class PCI::Device::Subclasses::NonVGA3DController = { 0x02, "Non VGA-Compatible 3D Controller", &PCI::Device::Classes::DisplayController , NULL };

PCI::Device::Class PCI::Device::Classes::MultimediaController = { 0x04, "Multimedia Controller", NULL, NULL };
	PCI::Device::Class PCI::Device::Subclasses::VideoDevice = { 0x00, "Video Device", &PCI::Device::Classes::MultimediaController , NULL };
	PCI::Device::Class PCI::Device::Subclasses::AudioDevice = { 0x01, "Audio Device", &PCI::Device::Classes::MultimediaController , NULL };
	PCI::Device::Class PCI::Device::Subclasses::ComputerTelephony = { 0x02, "Computer Telephony", &PCI::Device::Classes::MultimediaController , NULL };

PCI::Device::Class PCI::Device::Classes::MemoryController = { 0x05, "Memory Controller", NULL, NULL };
	PCI::Device::Class PCI::Device::Subclasses::RAMController = { 0x00, "RAM Controller", &PCI::Device::Classes::MemoryController , NULL };
	PCI::Device::Class PCI::Device::Subclasses::FlashController = { 0x01, "Flash Controller", &PCI::Device::Classes::MemoryController , NULL };
				
PCI::Device::Class PCI::Device::Classes::BridgeDevice = { 0x06, "Bridge Device", NULL, NULL };
	PCI::Device::Class PCI::Device::Subclasses::HostBridge = { 0x00, "Host Bridge", &PCI::Device::Classes::BridgeDevice , NULL };
	PCI::Device::Class PCI::Device::Subclasses::ISABridge = { 0x01, "ISA Bridge", &PCI::Device::Classes::BridgeDevice , NULL };
	PCI::Device::Class PCI::Device::Subclasses::EISABridge = { 0x02, "EISA Bridge", &PCI::Device::Classes::BridgeDevice , NULL };
	PCI::Device::Class PCI::Device::Subclasses::MCABridge = { 0x03, "MCA Bridge", &PCI::Device::Classes::BridgeDevice , NULL };
	PCI::Device::Class PCI::Device::Subclasses::PCItoPCIBridge = { 0x04, "PCI-to-PCI Bridge", &PCI::Device::Classes::BridgeDevice , NULL };
	PCI::Device::Class PCI::Device::Subclasses::PCMCIABridge = { 0x05, "PCMCIA Bridge", &PCI::Device::Classes::BridgeDevice , NULL };
	PCI::Device::Class PCI::Device::Subclasses::NuBusBridge = { 0x06, "NuBus Bridge", &PCI::Device::Classes::BridgeDevice , NULL };
	PCI::Device::Class PCI::Device::Subclasses::CardBusBridge = { 0x07, "CardBus Bridge", &PCI::Device::Classes::BridgeDevice , NULL };
	PCI::Device::Class PCI::Device::Subclasses::SemiTransPCItoPCIBridge = { 0x09, "Semi-Transparent PCI-to-PCI Bridge", &PCI::Device::Classes::BridgeDevice , NULL };

PCI::Device::Class PCI::Device::Classes::SimpleCommController = { 0x07, "Simple Communication Controllers", NULL, NULL };

PCI::Device::Class PCI::Device::Classes::BaseSystemPeripheral = { 0x08, "BaseSystemPeripheral", NULL, NULL };

PCI::Device::Class PCI::Device::Classes::InputDevice = { 0x09, "InputDevice", NULL, NULL };
	PCI::Device::Class PCI::Device::Subclasses::KeyboardController = { 0x00, "Keyboard Controller", &PCI::Device::Classes::InputDevice , NULL };
	PCI::Device::Class PCI::Device::Subclasses::Digitizer = { 0x01, "Digitizer", &PCI::Device::Classes::InputDevice , NULL };
	PCI::Device::Class PCI::Device::Subclasses::MouseController = { 0x02, "Mouse Controller", &PCI::Device::Classes::InputDevice , NULL };
	PCI::Device::Class PCI::Device::Subclasses::ScannerController = { 0x03, "Scanner Controller", &PCI::Device::Classes::InputDevice , NULL };
	PCI::Device::Class PCI::Device::Subclasses::GameportController = { 0x04, "Gameport Controller", &PCI::Device::Classes::InputDevice , NULL };	

PCI::Device::Class PCI::Device::Classes::DockingStation = { 0x0A, "DockingStation", NULL, NULL };

PCI::Device::Class PCI::Device::Classes::Processor = { 0x0B, "Processor", NULL, NULL };

PCI::Device::Class PCI::Device::Classes::SerialBusController = { 0x0C, "SerialBusController", NULL, NULL };
	PCI::Device::Class PCI::Device::Subclasses::FireWire = { 0x00, "FireWire", &PCI::Device::Classes::SerialBusController , NULL };
	PCI::Device::Class PCI::Device::Subclasses::AccessBus = { 0x01, "Access Bus", &PCI::Device::Classes::SerialBusController , NULL };
	PCI::Device::Class PCI::Device::Subclasses::SSA = { 0x02, "SSA", &PCI::Device::Classes::SerialBusController , NULL };
	PCI::Device::Class PCI::Device::Subclasses::USB = { 0x03, "USB", &PCI::Device::Classes::SerialBusController , NULL };
	PCI::Device::Class PCI::Device::Subclasses::FibreChannel = { 0x04, "Fibre Channel", &PCI::Device::Classes::SerialBusController , NULL };
	PCI::Device::Class PCI::Device::Subclasses::SMBus = { 0x05, "SMBus", &PCI::Device::Classes::SerialBusController , NULL };
	PCI::Device::Class PCI::Device::Subclasses::InfiniBand = { 0x06, "InfiniBand", &PCI::Device::Classes::SerialBusController , NULL };
	PCI::Device::Class PCI::Device::Subclasses::CANBus = { 0x09, "CAN Bus", &PCI::Device::Classes::SerialBusController , NULL };

PCI::Device::Class PCI::Device::Classes::WirelessController = { 0x0D, "WirelessController", NULL, NULL };
	PCI::Device::Class PCI::Device::Subclasses::IrDACompatibleController = { 0x00, "IrDA-Compatible Controller", &PCI::Device::Classes::WirelessController , NULL };
	PCI::Device::Class PCI::Device::Subclasses::IRController = { 0x01, "IR Controller", &PCI::Device::Classes::WirelessController , NULL };
	PCI::Device::Class PCI::Device::Subclasses::RFController = { 0x10, "RF Controller", &PCI::Device::Classes::WirelessController , NULL };
	PCI::Device::Class PCI::Device::Subclasses::BluetoothController = { 0x11, "Bluetooth Controller", &PCI::Device::Classes::WirelessController , NULL };
	PCI::Device::Class PCI::Device::Subclasses::BroadbandController = { 0x12, "Broadband Controller", &PCI::Device::Classes::WirelessController , NULL };
	PCI::Device::Class PCI::Device::Subclasses::EthernetController80211a = { 0x20, "Ethernet Controller 802.11a", &PCI::Device::Classes::WirelessController , NULL };
	PCI::Device::Class PCI::Device::Subclasses::EthernetController80211b = { 0x21, "Ethernet Controller 802.11b", &PCI::Device::Classes::WirelessController , NULL };

PCI::Device::Class PCI::Device::Classes::IntelligentIOController = { 0x0E, "IntelligentIOController", NULL, NULL };

PCI::Device::Class PCI::Device::Classes::SatelliteCommController = { 0x0F, "Satellite Communication Controllers", NULL, NULL };

PCI::Device::Class PCI::Device::Classes::EncryptDecryptController = { 0x10, "Encryption and Decryption Controllers", NULL, NULL };		

PCI::Device::Class PCI::Device::Classes::DataSignalController = { 0x11, "Data Acquisition and Signal Processing Controllers", NULL, NULL };

PCI::Device::Class PCI::Device::Subclasses::UndefinedSubclass = { 0xFF, "Undefined Subclass", NULL, NULL };

PCI::Device::Class* PCI::Device::Classes::classArray[] = {	&MassStorageController,
														&NetworkController,
														&DisplayController,
														&MultimediaController,
														&MemoryController,
														&BridgeDevice,
														&SimpleCommController,	
														&BaseSystemPeripheral,
														&InputDevice,
														&DockingStation,
														&Processor,
														&SerialBusController,
														&WirelessController,
														&IntelligentIOController,
														&SatelliteCommController,
														&EncryptDecryptController,
														&DataSignalController};
													

PCI::Device::Class* PCI::Device::Subclasses::subclassArray[] = {&SCSIBusController,
															&IDEController,
															&FloppyDiskController,
															&IPIBusController,
															&RAIDController,
															&ATAController,
															&SerialATAController,
															&SerialAttachedSCSI,

															&EthernetController,
															&TokenRingController,
															&FDDIController,
															&ATMController,
															&ISDNController,
															&WorldFipController,

															&VGACompatibleController,
															&XGAController,
															&NonVGA3DController,

															&VideoDevice,
															&AudioDevice,
															&ComputerTelephony,

															&RAMController,
															&FlashController,

															&HostBridge,
															&ISABridge,
															&EISABridge,
															&MCABridge,
															&PCItoPCIBridge,
															&PCMCIABridge,
															&NuBusBridge,
															&CardBusBridge,
															&SemiTransPCItoPCIBridge,			

															&KeyboardController,
															&Digitizer,
															&MouseController,
															&ScannerController,
															&GameportController,	

															&FireWire,
															&AccessBus,
															&SSA,
															&USB,
															&FibreChannel,
															&SMBus,
															&InfiniBand,
															&CANBus,

															&IrDACompatibleController,
															&IRController,
															&RFController,
															&BluetoothController,
															&BroadbandController,
															&EthernetController80211a,
															&EthernetController80211b};

LinkedList<PCI::Device*> zos::PCI_DEVICES;

PCI::Device::Class* PCI::Device::Classes::getClassByCode(uint8_t code)
{
	for (uint8_t i = 0; i < (sizeof(classArray) / sizeof(Class*)); i++)
	{
		if (classArray[i]->code == code)
		{	
			return classArray[i];
		}
	}
	
	return NULL;
}

PCI::Device::Class* PCI::Device::Subclasses::getSubclassByCode(PCI::Device::Class* parent, uint8_t code)
{
	for (uint8_t i = 0; i < (sizeof(subclassArray) / sizeof(Class*)); i++)
		if ((subclassArray[i]->parent == parent) && (subclassArray[i]->code == code))
			return subclassArray[i];
		
	return &UndefinedSubclass;
}

PCI::Device::Device(uint8_t bus, uint8_t device, uint8_t function)
{
	this->bus = bus;
	this->device = device;
	this->function = function;
	
	vendorID = (uint16_t)(readConfigDWord(PCI_CONFIG_INDEX_VENDORID) & 0xFFFF);
	deviceID = (uint16_t)((readConfigDWord(PCI_CONFIG_INDEX_DEVICEID) & 0xFFFF0000) >> 16);

	uint8_t classCode = (uint8_t)((readConfigDWord(PCI_CONFIG_INDEX_CLASS) & 0xFF000000) >> 24);
	uint8_t subclassCode = (uint8_t)((readConfigDWord(PCI_CONFIG_INDEX_SUBCLASS) & 0xFF0000) >> 16);
	
	deviceClass = Classes::getClassByCode(classCode);
	deviceSubclass = Subclasses::getSubclassByCode(deviceClass, subclassCode);
	
	progIF = (uint8_t)((readConfigDWord(PCI_CONFIG_INDEX_PROGIF) & 0xFF00) >> 8);
}

PCI::Device::Device(const PCI::Device& pciDevice)
{
	bus = pciDevice.bus;
	device = pciDevice.device;
	function = pciDevice.function;
	
	vendorID = pciDevice.vendorID;
	deviceID = pciDevice.deviceID;

	deviceClass = pciDevice.deviceClass;
	deviceSubclass = pciDevice.deviceSubclass;
	
	progIF = pciDevice.progIF;
}

uint32_t PCI::Device::readConfigDWord(uint8_t registerNo) const
{
	return PCI::readConfigDWord(bus, device, function, registerNo);
}

uint32_t PCI::Device::getBAR0() const
{
	return readConfigDWord(PCI_CONFIG_INDEX_BAR0);
}

uint32_t PCI::Device::getBAR1() const
{
	return readConfigDWord(PCI_CONFIG_INDEX_BAR1);
}

uint32_t PCI::Device::getBAR2() const
{
	return readConfigDWord(PCI_CONFIG_INDEX_BAR2);
}

uint32_t PCI::Device::getBAR3() const
{
	return readConfigDWord(PCI_CONFIG_INDEX_BAR3);
}

uint32_t PCI::Device::getBAR4() const
{
	return readConfigDWord(PCI_CONFIG_INDEX_BAR4);
}

uint32_t PCI::Device::getBAR5() const
{
	return readConfigDWord(PCI_CONFIG_INDEX_BAR5);
}

uint8_t PCI::Device::getINTLine() const
{
	return (uint8_t)(readConfigDWord(PCI_CONFIG_INDEX_INTLINE) & 0xFF);
}

uint32_t PCI::readConfigDWord(uint8_t bus, uint8_t device, uint8_t function, uint8_t registerNo)
{
	uint32_t configAddress = (	(1 << 31) | 	//Enable Bit
								(bus << 16) |
								((device & 0b00011111) << 11) |
								((function & 0b00000111) << 8) |
								((registerNo & 0b11111100)));
								
	HAL::outportl(PCI_CONFIG_ADDRESS_REG, configAddress);
	
	return HAL::inportl(PCI_CONFIG_DATA_REG);
}

bool PCI::isMultifunctionDevice(uint8_t bus, uint8_t device)
{
	uint8_t val = ((readConfigDWord(bus, device, 0, PCI_CONFIG_INDEX_HEADERTYPE) & 0xFF0000) >> 16);
	
	return ((val & 0b10000000) == 0b10000000);
}

bool PCI::isValidDevice(uint8_t bus, uint8_t device, uint8_t function)
{
	uint16_t vendorID = (uint16_t)(readConfigDWord(bus, device, function, PCI_CONFIG_INDEX_VENDORID) & 0xFFFF);
	
	if (vendorID == PCI_INVALID_VENDORID)
		return false;
	
	return true;
}

void PCI::scan()
{	
	//Loop through buses and devices
	for (uint16_t i = 0; i < PCI_MAX_BUSES; i++)
	{
		for (uint16_t j = 0; j < PCI_MAX_DEVICES; j++)
		{
			if (!isValidDevice(i, j, 0))
				continue;
			
			//Check for Multi-Function Devices
			for (uint16_t k = 0; (isMultifunctionDevice(i, j) ? (k < PCI_MAX_FUNCTIONS) : (k < 1)); k++)
			{
				//If the current function is not valid skip this iteration
				if (!isValidDevice(i, j, k))
					continue;
				
				PCI_DEVICES.add(new Device(i, j, k));
			}
		}
	}
}

PCI::Device* PCI::getFirstDeviceOfClass(PCI::Device::Class &deviceClass, PCI::Device::Class &deviceSubclass, Device* lastDevice)
{
	LinkedList<Device*>::Node* currentNode = PCI_DEVICES.head;
	
	while (currentNode != NULL)
	{
		Device* currentDevice = currentNode->value;
		
		if (lastDevice == NULL)
		{
			if ((currentDevice->deviceClass->code == deviceClass.code) &&
				(currentDevice->deviceSubclass->code == deviceSubclass.code))
			{
				return currentDevice;
			}
		}
		
		if (lastDevice == currentDevice)
			lastDevice = NULL;
		
		currentNode = currentNode->nextNode;
	}
	
	return NULL;
}