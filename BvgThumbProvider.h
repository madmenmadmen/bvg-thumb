#pragma once

#include <windows.h>
#include <shlobj.h>
#include <thumbcache.h>
#include <wincodec.h>
#include <wrl/client.h>
#include <vector>
#include <cstdint>

using Microsoft::WRL::ComPtr;

class BvgThumbProvider : public IThumbnailProvider,
    public IInitializeWithStream {
public:
    BvgThumbProvider();

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv) override;
    IFACEMETHODIMP_(ULONG) AddRef() override;
    IFACEMETHODIMP_(ULONG) Release() override;

    // IInitializeWithStream
    IFACEMETHODIMP Initialize(IStream* stream, DWORD grfMode) override;

    // IThumbnailProvider
    IFACEMETHODIMP GetThumbnail(UINT cx, HBITMAP* phbmp,
        WTS_ALPHATYPE* pdwAlpha) override;

private:
    ~BvgThumbProvider();
    LONG m_ref;
    IStream* m_stream;

    static HRESULT ReadAll(IStream* s, std::vector<uint8_t>& out);
    static HRESULT PngToHBITMAP(const uint8_t* png, size_t len,
        UINT cx, HBITMAP* out);
};