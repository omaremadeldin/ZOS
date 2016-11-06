//==========================================
//
//	     	   ZapperOS - RTC
//		      (Real Time Clock)
//==========================================
//By Omar Emad Eldin
//==========================================

#pragma once

#include <stdlib.h>

#define RTC_CMOS_ADDRESS	0x70
#define RTC_CMOS_DATA		0x71

#define RTC_REG_SECONDS		0x00
#define RTC_REG_MINUTES		0x02
#define RTC_REG_HOURS		0x04
#define RTC_REG_WEEKDAY		0x06
#define RTC_REG_DAYOFMONTH	0x07
#define RTC_REG_MONTH		0x08
#define RTC_REG_YEAR		0x09
#define RTC_REG_STATUS_A	0x0A
#define RTC_REG_STATUS_B	0x0B

#define RTC_STATUS_B_12HRS	0x02	//0 = 12-Hours, 1 = 24-Hours
#define RTC_STATUS_B_BCD	0x04	//0 = BCD, 1 = Binary

namespace zos
{
	class RTC
	{
	public:
		static uint8_t Seconds;
		static uint8_t Minutes;
		static uint8_t Hours;
		static uint8_t Day;
		static uint8_t Month;
		static uint16_t Year;

	private:
		static uint8_t readFromCMOS(uint8_t registerNo);
		//static void writeToCMOS(uint8_t registerNo, uint8_t value);
		static bool isDeviceBusy();

	public:
		static void fetchTime();
	};
}