#pragma once
#include <ShObjIdl.h>
/*
MIDL_INTERFACE("a5cd92ff-29be-454c-8d04-d82879fb3f1b")
IVirtualDesktopManager : public IUnknown
{
public:
virtual HRESULT STDMETHODCALLTYPE IsWindowOnCurrentVirtualDesktop(
__RPC__in HWND topLevelWindow,
__RPC__out BOOL *onCurrentDesktop) = 0;

virtual HRESULT STDMETHODCALLTYPE GetWindowDesktopId(
__RPC__in HWND topLevelWindow,
__RPC__out GUID *desktopId) = 0;

virtual HRESULT STDMETHODCALLTYPE MoveWindowToDesktop(
__RPC__in HWND topLevelWindow,
__RPC__in REFGUID desktopId) = 0;
};
*/


// i love this guy! big hug for documenting what M$ has left undocumented (3 calls they gave us? they kiddin!)
// http://www.cyberforum.ru/blogs/105416/blog3671.html
CLSID const CLSID_ImmersiveShell = { 0xC2F03A33, 0x21F5, 0x47FA, 0xB4, 0xBB, 0x15, 0x63, 0x62, 0xA2, 0xF2, 0x39 };
CLSID const CLSID_VirtualDesktopManagerInternal = { 0xC2F03A33, 0x21F5, 0x47FA, 0xB4, 0xBB, 0x15, 0x63, 0x62, 0xA2, 0xF2, 0x39 };
IID const IID_IVirtualDesktopManagerInternal = { 0xEF9F1A6C, 0xD3CC, 0x4358, 0xB7, 0x12, 0xF8, 0x4B, 0x63, 0x5B, 0xEB, 0xE7 };
CLSID const CLSID_IVirtualNotificationService = { 0xA501FDEC, 0x4A09, 0x464C, 0xAE, 0x4E, 0x1B, 0x9C, 0x21, 0xB8, 0x49, 0x18 };
IID const IID_IVirtualDesktopNotification = { 0xC179334C, 0x4295, 0x40D3,{ 0xBE, 0xA1, 0xC6, 0x54, 0xD9, 0x65, 0x60, 0x5A }};
CLSID const CLSID_VirtualDesktopAPI_Unknown = { 0xC5E0CDCA, 0x7B6E, 0x41B2, 0x9F, 0xC4, 0xD9, 0x39, 0x75, 0xCC, 0x46, 0x7B };
IID const IID_IApplicationViewCollection  = { 0x2C08ADF0, 0xA386, 0x4B35, {0x92, 0x50, 0x0F, 0xE1, 0x83, 0x47, 0x6F, 0xCC }};

struct IApplicationView;

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
//MIDL_INTERFACE("AF8DA486-95BB-4460-B3B7-6E7A6B2962B5")
// 10536
MIDL_INTERFACE("f31574d6-b682-4cdc-bd56-1827860abec6")
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

EXTERN_C const IID IID_IApplicationViewCollection;
struct IImmersiveApplication;
struct IApplicationViewChangeListener;
// from http://pf-j.sakura.ne.jp/program/tips/nonamefn/iavcoll.htm
// struct IApplicationViewCollection : public IUnknown
DECLARE_INTERFACE_IID_(IApplicationViewCollection, IUnknown, "2C08ADF0-A386-4B35-9250-0FE183476FCC")
{
	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObject) PURE;
	STDMETHOD_(ULONG, AddRef)(THIS) PURE;
	STDMETHOD_(ULONG, Release)(THIS) PURE;

	/*** IApplicationViewCollection methods ***/
	STDMETHOD(GetViews)(THIS_ IObjectArray**) PURE;
	STDMETHOD(GetViewsByZOrder)(THIS_ IObjectArray**) PURE;
	STDMETHOD(GetViewsByAppUserModelId)(THIS_ PCWSTR, IObjectArray**) PURE;
	STDMETHOD(GetViewForHwnd)(THIS_ HWND, IApplicationView**) PURE;
	STDMETHOD(GetViewForApplication)(THIS_ IImmersiveApplication*, IApplicationView**) PURE;
	STDMETHOD(GetViewForAppUserModelId)(THIS_ PCWSTR, IApplicationView**) PURE;
	STDMETHOD(GetViewInFocus)(THIS_ IApplicationView**) PURE;
	STDMETHOD(RefreshCollection)(THIS) PURE;
	STDMETHOD(RegisterForApplicationViewChanges)(THIS_ IApplicationViewChangeListener*, DWORD*) PURE;
	STDMETHOD(RegisterForApplicationViewPositionChanges)(THIS_ IApplicationViewChangeListener*, DWORD*) PURE;
	STDMETHOD(UnregisterForApplicationViewChanges)(THIS_ DWORD) PURE;
};

enum APPLICATION_VIEW_CLOAK_TYPE
{
	AVCT_NONE = 0,
	AVCT_DEFAULT = 1,
	AVCT_VIRTUAL_DESKTOP = 2
};
enum APPLICATION_VIEW_COMPATIBILITY_POLICY
{
	AVCP_NONE = 0,
	AVCP_SMALL_SCREEN = 1,
	AVCP_TABLET_SMALL_SCREEN = 2,
	AVCP_VERY_SMALL_SCREEN = 3,
	AVCP_HIGH_SCALE_FACTOR = 4
};
struct IImmersiveMonitor;
struct IAsyncCallback;
struct IApplicationViewPosition;
struct IShellPositionerPriority;
struct IApplicationViewOperation;
// struct IApplicationView : public IUnknown
DECLARE_INTERFACE_IID_(IApplicationView, IUnknown, "855BCAAD-3177-47B5-8571-23803421F9D8")
{
	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObject) PURE;
	STDMETHOD_(ULONG, AddRef)(THIS) PURE;
	STDMETHOD_(ULONG, Release)(THIS) PURE;

	/*** IApplicationView methods ***/
	STDMETHOD(SetFocus)(THIS) PURE;
	STDMETHOD(SwitchTo)(THIS) PURE;
	STDMETHOD(TryInvokeBack)(THIS_ IAsyncCallback*) PURE;
	STDMETHOD(GetThumbnailWindow)(THIS_ HWND*) PURE;
	STDMETHOD(GetMonitor)(THIS_ IImmersiveMonitor**) PURE;
	STDMETHOD(GetVisibility)(THIS_ int*) PURE;
	STDMETHOD(SetCloak)(THIS_ APPLICATION_VIEW_CLOAK_TYPE, int) PURE;
	STDMETHOD(GetPosition)(THIS_ REFIID, void**) PURE;
	STDMETHOD(SetPosition)(THIS_ IApplicationViewPosition*) PURE;
	STDMETHOD(InsertAfterWindow)(THIS_ HWND) PURE;
	STDMETHOD(GetExtendedFramePosition)(THIS_ RECT*) PURE;
	STDMETHOD(GetAppUserModelId)(THIS_ PWSTR*) PURE;
	STDMETHOD(SetAppUserModelId)(THIS_ PCWSTR) PURE;
	STDMETHOD(IsEqualByAppUserModelId)(THIS_ PCWSTR, int*) PURE;
	STDMETHOD(GetViewState)(THIS_ UINT*) PURE;
	STDMETHOD(SetViewState)(THIS_ UINT) PURE;
	STDMETHOD(GetNeediness)(THIS_ int*) PURE;
	STDMETHOD(GetLastActivationTimestamp)(THIS_ ULONGLONG*) PURE;
	STDMETHOD(SetLastActivationTimestamp)(THIS_ ULONGLONG) PURE;
	STDMETHOD(GetVirtualDesktopId)(THIS_ GUID*) PURE;
	STDMETHOD(SetVirtualDesktopId)(THIS_ REFGUID) PURE;
	STDMETHOD(GetShowInSwitchers)(THIS_ int*) PURE;
	STDMETHOD(SetShowInSwitchers)(THIS_ int) PURE;
	STDMETHOD(GetScaleFactor)(THIS_ int*) PURE;
	STDMETHOD(CanReceiveInput)(THIS_ BOOL*) PURE;
	STDMETHOD(GetCompatibilityPolicyType)(THIS_ APPLICATION_VIEW_COMPATIBILITY_POLICY*) PURE;
	STDMETHOD(SetCompatibilityPolicyType)(THIS_ APPLICATION_VIEW_COMPATIBILITY_POLICY) PURE;
	STDMETHOD(GetPositionPriority)(THIS_ IShellPositionerPriority**) PURE;
	STDMETHOD(SetPositionPriority)(THIS_ IShellPositionerPriority*) PURE;
	STDMETHOD(GetSizeConstraints)(THIS_ IImmersiveMonitor*, SIZE*, SIZE*) PURE;
	STDMETHOD(GetSizeConstraintsForDpi)(THIS_ UINT, SIZE*, SIZE*) PURE;
	STDMETHOD(SetSizeConstraintsForDpi)(THIS_ const UINT*, const SIZE*, const SIZE*) PURE;
	STDMETHOD(QuerySizeConstraintsFromApp)(THIS) PURE;
	STDMETHOD(OnMinSizePreferencesUpdated)(THIS_ HWND) PURE;
	STDMETHOD(ApplyOperation)(THIS_ IApplicationViewOperation*) PURE;
	STDMETHOD(IsTray)(THIS_ BOOL*) PURE;
	STDMETHOD(IsInHighZOrderBand)(THIS_ BOOL*) PURE;
	STDMETHOD(IsSplashScreenPresented)(THIS_ BOOL*) PURE;
	STDMETHOD(Flash)(THIS) PURE;
	STDMETHOD(GetRootSwitchableOwner)(THIS_ IApplicationView**) PURE;
	STDMETHOD(EnumerateOwnershipTree)(THIS_ IObjectArray**) PURE;
	/*** (Windows 10 Build 10584 or later?) ***/
	STDMETHOD(GetEnterpriseId)(THIS_ PWSTR*) PURE;
	STDMETHOD(IsMirrored)(THIS_ BOOL*) PURE;
};