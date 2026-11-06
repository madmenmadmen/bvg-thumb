#include "BvgThumbProvider.h"
#include <olectl.h>
#include <string>

static const CLSID CLSID_BvgThumb =
{ 0x3F2504E0, 0x4F89, 0x11D3,
  { 0x9A, 0x0C, 0x03, 0x05, 0xE8, 0x2C, 0x33, 0x01 } };

static const wchar_t* kClsidStr =
L"{3F2504E0-4F89-11D3-9A0C-0305E82C3301}";

static const wchar_t* kThumbGuid =
L"{e357fccd-a995-4576-b01f-234630154e96}";

static HMODULE g_hModule = nullptr;

class BvgThumbFactory : public IClassFactory {
public:
    IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv) override {
        if (riid == IID_IUnknown || riid == IID_IClassFactory)
            *ppv = static_cast<IClassFactory*>(this);
        else {
            *ppv = nullptr;
            return E_NOINTERFACE;
        }
        AddRef();
        return S_OK;
    }
    IFACEMETHODIMP_(ULONG) AddRef() override {
        return 2;
    }
    IFACEMETHODIMP_(ULONG) Release() override {
        return 1;
    }

    IFACEMETHODIMP CreateInstance(IUnknown* pUnkOuter, REFIID riid,
        void** ppv) override {
        if (ppv == nullptr) return E_POINTER;
        if (pUnkOuter != nullptr) return CLASS_E_NOAGGREGATION;
        auto* p = new (std::nothrow) BvgThumbProvider();
        if (!p) return E_OUTOFMEMORY;
        HRESULT hr = p->QueryInterface(riid, ppv);
        p->Release();
        return hr;
    }
    IFACEMETHODIMP LockServer(BOOL) override { return S_OK; }
};

static LONG g_dllRefs = 0;

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv) {
    if (ppv == nullptr) return E_POINTER;
    if (rclsid != CLSID_BvgThumb) return CLASS_E_CLASSNOTAVAILABLE;
    auto* f = new (std::nothrow) BvgThumbFactory();
    if (!f) return E_OUTOFMEMORY;
    HRESULT hr = f->QueryInterface(riid, ppv);
    f->Release();
    return hr;
}

STDAPI DllCanUnloadNow() {
    return g_dllRefs == 0 ? S_OK : S_FALSE;
}

static HRESULT SetKey(HKEY root, const wchar_t* sub,
    const wchar_t* name, const wchar_t* value) {
    HKEY k = nullptr;
    LONG rc = RegCreateKeyExW(root, sub, 0, nullptr, 0,
        KEY_WRITE, nullptr, &k, nullptr);
    if (rc != ERROR_SUCCESS) return HRESULT_FROM_WIN32(rc);

    HRESULT hr = S_OK;
    if (value) {
        rc = RegSetValueExW(k, name, 0, REG_SZ,
            reinterpret_cast<const BYTE*>(value),
            static_cast<DWORD>((wcslen(value) + 1)
                * sizeof(wchar_t)));
        if (rc != ERROR_SUCCESS) hr = HRESULT_FROM_WIN32(rc);
    }
    RegCloseKey(k);
    return hr;
}

STDAPI DllRegisterServer() {
    wchar_t dllPath[MAX_PATH] = { 0 };
    if (!GetModuleFileNameW(g_hModule, dllPath, MAX_PATH))
        return HRESULT_FROM_WIN32(GetLastError());
    std::wstring clsidKey = L"CLSID\\";
    clsidKey += kClsidStr;
    SetKey(HKEY_CLASSES_ROOT, clsidKey.c_str(), nullptr,
        L"BVG Thumbnail Provider");
    std::wstring inproc = clsidKey + L"\\InprocServer32";
    SetKey(HKEY_CLASSES_ROOT, inproc.c_str(), nullptr, dllPath);
    SetKey(HKEY_CLASSES_ROOT, inproc.c_str(), L"ThreadingModel",
        L"Apartment");
    SetKey(HKEY_CLASSES_ROOT, L".bvg", nullptr, L"BVG vector image");
    std::wstring shellex = L".bvg\\ShellEx\\";
    shellex += kThumbGuid;
    SetKey(HKEY_CLASSES_ROOT, shellex.c_str(), nullptr, kClsidStr);
    return S_OK;
}

STDAPI DllUnregisterServer() {
    std::wstring clsidKey = L"CLSID\\";
    clsidKey += kClsidStr;
    RegDeleteTreeW(HKEY_CLASSES_ROOT, clsidKey.c_str());
    std::wstring shellex = L".bvg\\ShellEx\\";
    shellex += kThumbGuid;
    RegDeleteTreeW(HKEY_CLASSES_ROOT, shellex.c_str());

    HKEY hk;
    if (RegOpenKeyExW(HKEY_CLASSES_ROOT, L".bvg", 0, KEY_WRITE, &hk) == ERROR_SUCCESS) {
        RegDeleteValueW(hk, nullptr);
        RegCloseKey(hk);
    }

    return S_OK;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        g_hModule = hModule;
        DisableThreadLibraryCalls(hModule);
    }
    return TRUE;
}