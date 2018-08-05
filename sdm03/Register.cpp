// Register.cpp: implementation of the Register class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Register.h"
#include "driver.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Register::Register()
{
	InitializeCriticalSection(&m_lock);
	memset(m_instances,0,MAX_INSTANCES * sizeof(m_instances[0]));
}

Register::~Register()
{
	DeleteCriticalSection(&m_lock);
}


DWORD Register::Put(Driver* ptr)
{
	int cookie = 0;
	
	if (ptr)
	{
		EnterCriticalSection(&m_lock);

		int index = _Find(0);

		if (index != -1)
		{
			m_instances[index] = ptr;
			ptr->AddRef();
			cookie = (DWORD)ptr; 
		}

		LeaveCriticalSection(&m_lock);
	}

	return cookie;
}

Driver* Register::Get(DWORD cookie)
{
	
	Driver* ptr = 0;
	
	if (cookie != 0)
	{

		EnterCriticalSection(&m_lock);
		
		
		int indx = _Find((Driver*)cookie);

		if (indx != -1)
		{
			ptr = m_instances[indx];
			ptr->AddRef();
		}
		
		
		LeaveCriticalSection(&m_lock);
		
	}

	return ptr;
}


bool Register::Remove(DWORD index)
{
	bool ret = false;
	
	if (index != 0)
	{
		EnterCriticalSection(&m_lock);
			
		int indx;
		
		
		if ((indx = _Find((Driver*)index)) != -1)
		{
			Driver* ptr = m_instances[indx];
			ptr->Release();
			m_instances[indx] = 0;
			ret = true;
		}
		

		LeaveCriticalSection(&m_lock);
	
	}
	
	return ret;
}



int Register::_Find(Driver* ptr)
{
	
	for (int i = 0 ; i < MAX_INSTANCES;++i)
	{
		if (m_instances[i] == ptr)
			return i;
	}
	
	return -1;

}