#pragma once

class SimpleCapture
{
public:
    SimpleCapture(
        winrt::Microsoft::Graphics::Canvas::CanvasDevice const& device,
        winrt::Windows::Graphics::Capture::GraphicsCaptureItem const& item);
    ~SimpleCapture() { Close(); }

    void StartCapture();
    winrt::Windows::UI::Composition::ICompositionSurface CreateSurface(
        winrt::Windows::UI::Composition::Compositor const& compositor);

    void Close();

private:
    void OnFrameArrived(
        winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool const& sender,
        winrt::Windows::Foundation::IInspectable const& args);

    inline void CheckClosed()
    {
        if (m_closed.load() == true)
        {
            throw winrt::hresult_error(RO_E_CLOSED);
        }
    }

private:
    winrt::Windows::Graphics::Capture::GraphicsCaptureItem m_item{ nullptr };
    winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool m_framePool{ nullptr };
    winrt::Windows::Graphics::Capture::GraphicsCaptureSession m_session{ nullptr };
    winrt::Windows::Graphics::SizeInt32 m_lastSize;

    winrt::Microsoft::Graphics::Canvas::CanvasDevice m_device{ nullptr };
    winrt::Microsoft::Graphics::Canvas::CanvasSwapChain m_swapChain{ nullptr };

    winrt::Windows::System::DispatcherQueueController m_dispatcherQueueController{ nullptr };
    winrt::Windows::System::DispatcherQueue m_dispatcherQueue{ nullptr };

    std::atomic<bool> m_closed = false;
};