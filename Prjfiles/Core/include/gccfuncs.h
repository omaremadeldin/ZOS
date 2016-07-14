//==========================================
//
//		    ZapperOS - GCC Functions
//
//==========================================
//These are a couple of functions GCC
//requires in order for C++ to function
//correctly.
//==========================================
//By Omar Emad Eldin
//==========================================

#define ATEXIT_MAX_FUNCS 128

typedef unsigned uarch_t;

struct atexit_func_entry_t
{
	void (*destructor_func)(void *);
	void *obj_ptr;
	void *dso_handle;
};

extern "C"
{
	atexit_func_entry_t __atexit_funcs[ATEXIT_MAX_FUNCS];
	uarch_t __atexit_func_count = 0;

	extern void *__dso_handle;

	int __cxa_atexit(void (*f) (void *), void *objptr, void *dso)
	{
		if (__atexit_func_count >= ATEXIT_MAX_FUNCS)
			return -1;
		
		__atexit_funcs[__atexit_func_count].destructor_func = f;
		__atexit_funcs[__atexit_func_count].obj_ptr = objptr;
		__atexit_funcs[__atexit_func_count].dso_handle = dso;
		__atexit_func_count++;
		
		return 0;
	}

	void __cxa_finalize(void *f)
	{
		uarch_t i = __atexit_func_count;
		
		if (f == NULL)
		{
			while (i--)
			{
				if (__atexit_funcs[i].destructor_func != NULL)
				{
					(*__atexit_funcs[i].destructor_func)(__atexit_funcs[i].obj_ptr);
					__atexit_funcs[i].destructor_func = NULL;
				}
			}
		}
		else
		{
			while (i--)
			{
				if (__atexit_funcs[i].destructor_func == f)
				{
					(*__atexit_funcs[i].destructor_func)(__atexit_funcs[i].obj_ptr);
					__atexit_funcs[i].destructor_func = NULL;
				}
			}
		}
	}

	void __cxa_pure_virtual()
	{}
	
	uint64_t __udivmoddi4(uint64_t num, uint64_t den, uint64_t* rem_p)
	{
		uint64_t quot = 0, qbit = 1;

		//Intentional division by zero to trigger divide by zero exception
		if (den == 0)
			return 1/den;

		//Left-justify denominator and count shift
		while ((int64_t)den >= 0)
		{
			den <<= 1;
			qbit <<= 1;
		}

		while (qbit)
		{
			if (den <= num)
			{
				num -= den;
				quot += qbit;
			}

			den >>= 1;
			qbit >>= 1;
		}

		if (rem_p)
			*rem_p = num;

		return quot;
	}

	uint64_t __udivdi3(uint64_t num, uint64_t den)
	{
		return __udivmoddi4(num, den, NULL);
	}
	
	uint64_t __umoddi3(uint64_t num, uint64_t den)
	{
		uint64_t v;
		
		__udivmoddi4(num, den, &v);

		return v;
	}
}