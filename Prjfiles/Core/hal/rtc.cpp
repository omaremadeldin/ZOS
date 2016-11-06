//==========================================
//
//	     	   ZapperOS - RTC
//		      (Real Time Clock)
//==========================================
//By Omar Emad Eldin
//==========================================

#include "rtc.h"

#include "hal.h"

using namespace zos;

uint8_t RTC::Seconds = 0;
uint8_t RTC::Minutes = 0;
uint8_t RTC::Hours = 0;
uint8_t RTC::Day = 0;
uint8_t RTC::Month = 0;
uint16_t RTC::Year = 0;

uint8_t RTC::readFromCMOS(uint8_t registerNo)
{
	HAL::outportb(RTC_CMOS_ADDRESS, registerNo);
	return HAL::inportb(RTC_CMOS_DATA);
}

bool RTC::isDeviceBusy()
{
	return (readFromCMOS(RTC_REG_STATUS_A) & 0x80);
}

void RTC::fetchTime()
{
	while (isDeviceBusy());

	uint8_t last_seconds = Seconds = readFromCMOS(RTC_REG_SECONDS);
	uint8_t last_minutes = Minutes = readFromCMOS(RTC_REG_MINUTES);
	uint8_t last_hours = Hours = readFromCMOS(RTC_REG_HOURS);
	uint8_t last_day = Day = readFromCMOS(RTC_REG_DAYOFMONTH);
	uint8_t last_month = Month = readFromCMOS(RTC_REG_MONTH);
	uint16_t last_year = Year = readFromCMOS(RTC_REG_YEAR);

	do
	{
		last_seconds = Seconds;
		last_minutes = Minutes;
		last_hours = Hours;
		last_day = Day;
		last_month = Month;
		last_year = Year;

		Seconds = readFromCMOS(RTC_REG_SECONDS);
		Minutes = readFromCMOS(RTC_REG_MINUTES);
		Hours = readFromCMOS(RTC_REG_HOURS);
		Day = readFromCMOS(RTC_REG_DAYOFMONTH);
		Month = readFromCMOS(RTC_REG_MONTH);
		Year = readFromCMOS(RTC_REG_YEAR);
	}
	while 	((Seconds != last_seconds) ||
			(Minutes != last_minutes) ||
			(Hours != last_hours) ||
			(Day != last_day) ||
			(Month != last_month) ||
			(Year != last_year));

	uint8_t status_b = readFromCMOS(RTC_REG_STATUS_B);

	if (!(status_b & RTC_STATUS_B_BCD))
	{
		Seconds = (Seconds & 0x0F) + ((Seconds / 16) * 10);
		Minutes = (Minutes & 0x0F) + ((Minutes / 16) * 10);
		Hours = ((Hours & 0x0F) + (((Hours & 0x70) / 16) * 10)) | (Hours & 0x80);
		Day = (Day & 0x0F) + ((Day / 16) * 10);
		Month = (Month & 0x0F) + ((Month / 16) * 10);
		Year = (Year & 0x0F) + ((Year / 16) * 10);
	}

	if (!(status_b & RTC_STATUS_B_12HRS) && (Hours & 0x80))
    	Hours = (((Hours & 0x7F) + 12) % 24);

	Year = 2000 + Year;
}