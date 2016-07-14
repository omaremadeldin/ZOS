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
//TODO:
//* Use PCI classes to initialize devices
//	instead of doing it manually
//==========================================
//By Omar Emad Eldin
//==========================================

#pragma once

#include <stdlib.h>

#include "utils/linkedlist.h"

//About 4KBs of PCI Devices
#define PCI_MAX_STORED_DEVICES	400

#define PCI_MAX_BUSES		256
#define PCI_MAX_DEVICES		32	
#define PCI_MAX_FUNCTIONS	8

#define PCI_CONFIG_ADDRESS_REG	0xCF8
#define PCI_CONFIG_DATA_REG		0xCFC

#define PCI_CONFIG_INDEX_VENDORID		0x00
#define PCI_CONFIG_INDEX_DEVICEID		0x00
#define PCI_CONFIG_INDEX_CLASS			0x08
#define PCI_CONFIG_INDEX_SUBCLASS		0x08
#define PCI_CONFIG_INDEX_PROGIF			0x08
#define PCI_CONFIG_INDEX_BAR0			0x10
#define PCI_CONFIG_INDEX_BAR1			0x14
#define PCI_CONFIG_INDEX_HEADERTYPE		0x0C
#define PCI_CONFIG_INDEX_INTLINE		0x3C

#define PCI_CONFIG_INDEX_BAR2			0x18
#define PCI_CONFIG_INDEX_BAR3			0x1C
#define PCI_CONFIG_INDEX_BAR4			0x20
#define PCI_CONFIG_INDEX_BAR5			0x24

#define PCI_INVALID_VENDORID	0xFFFF

namespace zos
{	
	class PCI
	{
	public:
		class Device
		{
			
			public:
				struct Class
				{
					uint8_t code;
					const char* name;
					Class* parent;
					
					void (*initialize)(const Device& pciDevice);
				};
			
				struct Classes
				{
					static Class MassStorageController;
					static Class NetworkController;
					static Class DisplayController;
					static Class MultimediaController;
					static Class MemoryController;
					static Class BridgeDevice;
					static Class SimpleCommController;	
					static Class BaseSystemPeripheral;
					static Class InputDevice;
					static Class DockingStation;
					static Class Processor;
					static Class SerialBusController;
					static Class WirelessController;
					static Class IntelligentIOController;
					static Class SatelliteCommController;
					static Class EncryptDecryptController;
					static Class DataSignalController;
					
					static Class* classArray[];
					
					static Class* getClassByCode(uint8_t code);
				};
				
				struct Subclasses
				{	
					static Class SCSIBusController;
					static Class IDEController;
					static Class FloppyDiskController;
					static Class IPIBusController;
					static Class RAIDController;
					static Class ATAController;
					static Class SerialATAController;
					static Class SerialAttachedSCSI;
					
					static Class EthernetController;
					static Class TokenRingController;
					static Class FDDIController;
					static Class ATMController;
					static Class ISDNController;
					static Class WorldFipController;
				
					static Class VGACompatibleController;
					static Class XGAController;
					static Class NonVGA3DController;
				
					static Class VideoDevice;
					static Class AudioDevice;
					static Class ComputerTelephony;
				
					static Class RAMController;
					static Class FlashController;
				
					static Class HostBridge;
					static Class ISABridge;
					static Class EISABridge;
					static Class MCABridge;
					static Class PCItoPCIBridge;
					static Class PCMCIABridge;
					static Class NuBusBridge;
					static Class CardBusBridge;
					static Class SemiTransPCItoPCIBridge;			
				
					static Class KeyboardController;
					static Class Digitizer;
					static Class MouseController;
					static Class ScannerController;
					static Class GameportController;	
				
					static Class FireWire;
					static Class AccessBus;
					static Class SSA;
					static Class USB;
					static Class FibreChannel;
					static Class SMBus;
					static Class InfiniBand;
					static Class CANBus;
					
					static Class IrDACompatibleController;
					static Class IRController;
					static Class RFController;
					static Class BluetoothController;
					static Class BroadbandController;
					static Class EthernetController80211a;
					static Class EthernetController80211b;
					
					static Class UndefinedSubclass;
					
					static Class* subclassArray[];
					
					static Class* getSubclassByCode(Class* parent, uint8_t code);
				};
			
			public:
				uint8_t bus;
				uint8_t device;
				uint8_t function;
				uint16_t vendorID;
				uint16_t deviceID;
				Class* deviceClass;
				Class* deviceSubclass;
				uint8_t progIF;
				
			public:					
				Device(const Device& pciDevice);
				Device(uint8_t bus, uint8_t device, uint8_t function);
				uint32_t readConfigDWord(uint8_t registerNo) const;
				uint32_t getBAR0() const;
				uint32_t getBAR1() const;
				uint32_t getBAR2() const;
				uint32_t getBAR3() const;
				uint32_t getBAR4() const;
				uint32_t getBAR5() const;
				uint8_t getINTLine() const;
		};
	
	private:
		static uint32_t readConfigDWord(uint8_t bus, uint8_t device, uint8_t function, uint8_t registerNo);
		static bool isMultifunctionDevice(uint8_t bus, uint8_t device);
		
	public:
		static bool isValidDevice(uint8_t bus, uint8_t device, uint8_t function);
		static void scan();
		static Device* getFirstDeviceOfClass(Device::Class &deviceClass, Device::Class &deviceSubclass, Device* lastDevice = NULL);
	};
	
	extern "C" LinkedList<PCI::Device*> PCI_DEVICES;
}