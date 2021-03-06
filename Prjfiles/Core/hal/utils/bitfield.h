//==========================================
//
//	    	  ZapperOS - Bitfield
//
//==========================================
//By Omar Emad Eldin
//==========================================

#pragma once

#include <stdlib.h>

template <typename T> 
class BitField
{	
public:
	BitField()
	{
		*((T*)this) = 0;
	}
	
	T value()
	{
		return *((T*)this);
	}

	void clear()
	{
		*((T*)this) = 0;
	}
};

// template <typename T>
// class LargeBitField
// {
// public:
	// LargeBitField()
	// {
		// memclr((void*)this, sizeof(T));
	// }
// };