#pragma once

using SH = System::Runtime::InteropServices::SafeHandle;
using PtrType = System::IntPtr;

// native template, default deleter, T is a native class
template <typename T>
struct DefDeleter
{
	void operator()(T* p) const { delete p; }
};
#define OccHandle() (*(this->Ptr()))


template <typename T, typename D = DefDeleter<T>>

public ref class XbimHandle : SH
{
internal:
	T* Ptr() { return static_cast<T*>(handle.ToPointer()); }
	T& Ref() { return (*(this->Ptr())); }
	T* Exec() { return static_cast<T*>(handle.ToPointer()); }
	XbimHandle(T* p) : SH(PtrType::Zero, true)
	{
		handle = PtrType(p);
	}

	
protected:
	bool ReleaseHandle() override
	{
		if (!IsInvalid)
		{
			D del;
			del(Ptr());
			handle = PtrType::Zero;
		}
		return true;
	}

public:
	 
	property bool IsInvalid
	{
		bool get() override
		{
			return (handle == PtrType::Zero);
		}
	}
	
	
};


