#pragma once
#include <ShObjIdl.h>

// i love this guy! big hug for documenting what M$ has left undocumented (3 calls they gave us? they kiddin!)
// http://www.cyberforum.ru/blogs/105416/blog3671.html
CLSID const CLSID_ImmersiveShell = { 0xC2F03A33, 0x21F5, 0x47FA, 0xB4, 0xBB, 0x15, 0x63, 0x62, 0xA2, 0xF2, 0x39 };
CLSID const CLSID_VirtualDesktopManagerInternal = { 0xC2F03A33, 0x21F5, 0x47FA, 0xB4, 0xBB, 0x15, 0x63, 0x62, 0xA2, 0xF2, 0x39 };
IID const IID_IVirtualDesktopManagerInternal = { 0xEF9F1A6C, 0xD3CC, 0x4358, 0xB7, 0x12, 0xF8, 0x4B, 0x63, 0x5B, 0xEB, 0xE7 };
CLSID const CLSID_IVirtualNotificationService = { 0xA501FDEC, 0x4A09, 0x464C, 0xAE, 0x4E, 0x1B, 0x9C, 0x21, 0xB8, 0x49, 0x18 };
IID const IID_IVirtualDesktopNotification = { 0xC179334C, 0x4295, 0x40D3,{ 0xBE, 0xA1, 0xC6, 0x54, 0xD9, 0x65, 0x60, 0x5A }};
CLSID const CLSID_VirtualDesktopAPI_Unknown = { 0xC5E0CDCA, 0x7B6E, 0x41B2, 0x9F, 0xC4, 0xD9, 0x39, 0x75, 0xCC, 0x46, 0x7B };

struct IApplicationView : public IUnknown { };

EXTERN_C const IID IID_IVirtualDesktop;

MIDL_INTERFACE("FF72FFDD-BE7E-43FC-9C03-AD81681E88E4")
IVirtualDesktop : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE IsViewVisible (IApplicationView * pView, int * pfVisible) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetID (GUID * pGuid) = 0;
};

EXTERN_C const IID IID_IVirtualDesktopManagerInternal;

// 10130
//MIDL_INTERFACE("EF9F1A6C-D3CC-4358-B712-F84B635BEBE7")
// 10240
MIDL_INTERFACE("AF8DA486-95BB-4460-B3B7-6E7A6B2962B5")
// 10536
//MIDL_INTERFACE("f31574d6-b682-4cdc-bd56-1827860abec6")
IVirtualDesktopManagerInternal : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE GetCount (UINT * pCount) = 0;
	virtual HRESULT STDMETHODCALLTYPE MoveViewToDesktop (IApplicationView * pView, IVirtualDesktop * pDesktop) = 0;
	// 10240
	virtual HRESULT STDMETHODCALLTYPE CanViewMoveDesktops (IApplicationView *pView, int * pfCanViewMoveDesktops) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetCurrentDesktop (IVirtualDesktop** desktop) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetDesktops (IObjectArray **ppDesktops) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetAdjacentDesktop (IVirtualDesktop * pDesktopReference, AdjacentDesktop uDirection, IVirtualDesktop ** ppAdjacentDesktop) = 0;
	virtual HRESULT STDMETHODCALLTYPE SwitchDesktop (IVirtualDesktop * pDesktop) = 0;
	virtual HRESULT STDMETHODCALLTYPE CreateDesktopW (IVirtualDesktop ** ppNewDesktop) = 0;
	virtual HRESULT STDMETHODCALLTYPE RemoveDesktop (IVirtualDesktop * pRemove, IVirtualDesktop * pFallbackDesktop) = 0;
	// 10240
	virtual HRESULT STDMETHODCALLTYPE FindDesktop (GUID * desktopId, IVirtualDesktop ** ppDesktop) = 0;
};

EXTERN_C const IID IID_IVirtualDesktopNotification;
MIDL_INTERFACE("C179334C-4295-40D3-BEA1-C654D965605A")
IVirtualDesktopNotification : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE VirtualDesktopCreated (IVirtualDesktop * pDesktop) = 0;
	virtual HRESULT STDMETHODCALLTYPE VirtualDesktopDestroyBegin (IVirtualDesktop * pDesktopDestroyed, IVirtualDesktop * pDesktopFallback) = 0;
	virtual HRESULT STDMETHODCALLTYPE VirtualDesktopDestroyFailed (IVirtualDesktop * pDesktopDestroyed, IVirtualDesktop * pDesktopFallback) = 0;
	virtual HRESULT STDMETHODCALLTYPE VirtualDesktopDestroyed (IVirtualDesktop * pDesktopDestroyed, IVirtualDesktop * pDesktopFallback) = 0;
	virtual HRESULT STDMETHODCALLTYPE ViewVirtualDesktopChanged (IApplicationView * pView) = 0;
	virtual HRESULT STDMETHODCALLTYPE CurrentVirtualDesktopChanged (IVirtualDesktop * pDesktopOld, IVirtualDesktop * pDesktopNew) = 0;
};

EXTERN_C const IID IID_IVirtualDesktopNotificationService;
MIDL_INTERFACE("0CD45E71-D927-4F15-8B0A-8FEF525337BF")
IVirtualDesktopNotificationService : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE Register (IVirtualDesktopNotification * pNotification, DWORD * pdwCookie) = 0;
	virtual HRESULT STDMETHODCALLTYPE Unregister (DWORD dwCookie) = 0;
};

