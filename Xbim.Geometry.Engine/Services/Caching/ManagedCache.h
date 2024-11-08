#pragma once
#include "./Cache.h"
using SH = System::Runtime::InteropServices::SafeHandle;


public ref class ManagedCache : SH
{
public:
    ManagedCache(Cache* p)
        : SH(System::IntPtr::Zero, true)
    {
        handle = System::IntPtr(p);
    }

    virtual property bool IsInvalid
    {
        bool get() override
        {
            return handle == System::IntPtr::Zero;
        }
    }

protected:

    virtual bool ReleaseHandle() override
    {
        if (!IsInvalid)
        {
            Cache* pCache = static_cast<Cache*>(handle.ToPointer());
            pCache->Clear();
            delete pCache;
            handle = System::IntPtr::Zero;
        }
        return true;
    }

    Cache* GetCache()
    {
        return static_cast<Cache*>(handle.ToPointer());
    }
};
