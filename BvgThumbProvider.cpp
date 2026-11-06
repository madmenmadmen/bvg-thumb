#include "BvgThumbProvider.h"
#include "bvg/BvgFile.hpp"
#include "bvg/Generated.hpp"

BvgThumbProvider::BvgThumbProvider() : m_ref(1), m_stream(nullptr) {}

BvgThumbProvider::~BvgThumbProvider() {
    if (m_stream) m_stream->Release();
}

// ---------- IUnknown ----------
IFACEMETHODIMP BvgThumbProvider::QueryInterface(REFIID riid, void** ppv) {
    if (riid == IID_IUnknown || riid == IID_IThumbnailProvider)
        *ppv = static_cast<IThumbnailProvider*>(this);
    else if (riid == IID_IInitializeWithStream)
        *ppv = static_cast<IInitializeWithStream*>(this);
    else {
        *ppv = nullptr;
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}

IFACEMETHODIMP_(ULONG) BvgThumbProvider::AddRef() {
    return InterlockedIncrement(&m_ref);
}

IFACEMETHODIMP_(ULONG) BvgThumbProvider::Release() {
    ULONG r = InterlockedDecrement(&m_ref);
    if (r == 0) delete this;
    return r;
}

// ---------- IInitializeWithStream ----------
IFACEMETHODIMP BvgThumbProvider::Initialize(IStream* s, DWORD) {
    m_stream = s;
    if (m_stream) m_stream->AddRef();
    return S_OK;
}

IFACEMETHODIMP BvgThumbProvider::GetThumbnail(UINT cx, HBITMAP* phbmp, WTS_ALPHATYPE* pdwAlpha) {
    if (!phbmp || !pdwAlpha) return E_POINTER;
    *phbmp = nullptr;
    *pdwAlpha = WTSAT_ARGB;
    if (!m_stream) return E_UNEXPECTED;

    try {
        std::vector<uint8_t> buf;
        HRESULT hr = ReadAll(m_stream, buf);
        if (FAILED(hr)) return hr;

        bvg::BvgFile f = bvg::BvgFile::read_from_bytes(buf);
        auto* thmb = f.getChunk("THMB");
        if (!thmb) return E_FAIL;

        return PngToHBITMAP(thmb->data(), thmb->size(), cx, phbmp);
    }
    catch (...) {
        return E_FAIL;
    }
}

HRESULT BvgThumbProvider::ReadAll(IStream* s, std::vector<uint8_t>& out) {
    STATSTG st{};
    if (FAILED(s->Stat(&st, STATFLAG_NONAME))) {
        return E_FAIL;
    }
    if (st.cbSize.QuadPart == 0 || st.cbSize.QuadPart > 100 * 1024 * 1024) {
        return E_FAIL;
    }

    out.resize(static_cast<size_t>(st.cbSize.QuadPart));

    LARGE_INTEGER zero{};
    HRESULT hr = s->Seek(zero, STREAM_SEEK_SET, nullptr);
    if (FAILED(hr)) return hr;

    ULONG read = 0;
    hr = s->Read(out.data(), static_cast<ULONG>(out.size()), &read);
    if (FAILED(hr) && hr != S_FALSE) return hr;

    if (read < out.size()) {
        out.resize(read);
    }

    return out.empty() ? E_FAIL : S_OK;
}

HRESULT BvgThumbProvider::PngToHBITMAP(const uint8_t* png, size_t len,
    UINT cx, HBITMAP* out) {
    *out = nullptr;
    ComPtr<IWICImagingFactory> factory;
    HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr,
        CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
    if (FAILED(hr)) return hr;

    ComPtr<IWICStream> stream;
    hr = factory->CreateStream(&stream);
    if (FAILED(hr)) return hr;
    hr = stream->InitializeFromMemory(const_cast<BYTE*>(png),
        static_cast<DWORD>(len));
    if (FAILED(hr)) return hr;

    ComPtr<IWICBitmapDecoder> dec;
    hr = factory->CreateDecoderFromStream(stream.Get(), nullptr,
        WICDecodeMetadataCacheOnDemand, &dec);
    if (FAILED(hr)) return hr;

    ComPtr<IWICBitmapFrameDecode> frame;
    hr = dec->GetFrame(0, &frame);
    if (FAILED(hr)) return hr;

    ComPtr<IWICFormatConverter> conv;
    hr = factory->CreateFormatConverter(&conv);
    if (FAILED(hr)) return hr;
    hr = conv->Initialize(frame.Get(), GUID_WICPixelFormat32bppPBGRA,
        WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);
    if (FAILED(hr)) return hr;

    UINT w = 0, h = 0;
    conv->GetSize(&w, &h);
    UINT tw = cx, th = cx;
    if (w && h) {
        if (w > h) th = max(1u, cx * h / w);
        else       tw = max(1u, cx * w / h);
    }

    ComPtr<IWICBitmapScaler> scaler;
    hr = factory->CreateBitmapScaler(&scaler);
    if (FAILED(hr)) return hr;
    hr = scaler->Initialize(conv.Get(), tw, th,
        WICBitmapInterpolationModeFant);
    if (FAILED(hr)) return hr;

    const UINT stride = tw * 4;
    std::vector<uint8_t> pixels(stride * th);
    hr = scaler->CopyPixels(nullptr, stride,
        static_cast<UINT>(pixels.size()),
        pixels.data());
    if (FAILED(hr)) return hr;

    BITMAPINFO bi{};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = static_cast<LONG>(tw);
    bi.bmiHeader.biHeight = -static_cast<LONG>(th);
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;

    void* bits = nullptr;
    HBITMAP hbmp = CreateDIBSection(nullptr, &bi, DIB_RGB_COLORS,
        &bits, nullptr, 0);
    if (!hbmp) return E_FAIL;

    std::memcpy(bits, pixels.data(), pixels.size());
    *out = hbmp;
    return S_OK;
}