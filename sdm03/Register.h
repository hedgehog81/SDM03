// Register.h: interface for the Register class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_REGISTER_H__957B9266_0416_472B_B570_ABA49E6FEB3D__INCLUDED_)
#define AFX_REGISTER_H__957B9266_0416_472B_B570_ABA49E6FEB3D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class Driver;

class Register  
{
public:
	Register();
	~Register();

	DWORD Put(Driver* ptr);
	bool  Remove(DWORD index);

	Driver* Get(DWORD cookie);

	
private:
	Register(const Register& other);
	Register& operator=(const Register& other);
	
	int _Find(Driver* ptr);

private:
	enum {MAX_INSTANCES = 10};
	Driver* m_instances[MAX_INSTANCES];
	CRITICAL_SECTION m_lock;
};

#endif // !defined(AFX_REGISTER_H__957B9266_0416_472B_B570_ABA49E6FEB3D__INCLUDED_)
