#include "pch.h"
#include "Capture.h"
#include "safe_flag.h"

using namespace winrt;

using namespace Windows;
using namespace Windows::Foundation;
using namespace Windows::System;
using namespace Windows::Graphics;
using namespace Windows::Graphics::Capture;
using namespace Windows::Graphics::DirectX;
using namespace Windows::Foundation::Numerics;
using namespace Windows::UI;
using namespace Windows::UI::Composition;
using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::UI::Composition;

SimpleCapture::SimpleCapture(
    CanvasDevice const& device, 
    GraphicsCaptureItem const& item)
{
    m_item = item;
    m_device = device;

    // TODO: Dpi?
    m_swapChain = CanvasSwapChain(m_device, (float)m_item.Size().Width, (float)m_item.Size().Height, 96);

    // Create our thread
    m_dispatcherQueueController = DispatcherQueueController::CreateOnDedicatedThread();
    m_dispatcherQueue = m_dispatcherQueueController.DispatcherQueue();

    // Don't return from the constructor until our work has finished on
    // the capture thread.
    auto initialized = std::make_shared<safe_flag>();
    // If we were to just capture the item and device in the lambda, we
    // would end up smuggling the objects to the other thread and get a 
    // marshalling error. Use an agile ref to get around this.
    auto itemAgile = agile_ref(m_item);
    auto deviceAgile = agile_ref(m_device);
    auto success = m_dispatcherQueue.TryEnqueue([=, &initialized]() -> void
    {
        auto itemTemp = itemAgile.get();
        auto deviceTemp = deviceAgile.get();

        m_framePool = Direct3D11CaptureFramePool::Create(
            deviceTemp,
            DirectXPixelFormat::B8G8R8A8UIntNormalized,
            2,
            itemTemp.Size());
        m_session = m_framePool.CreateCaptureSession(itemTemp);
        m_lastSize = itemTemp.Size();
        m_framePool.FrameArrived({ this, &SimpleCapture::OnFrameArrived });

        initialized->set();
    });

    if (!success)
    {
        throw hresult_error(E_UNEXPECTED);
    }

    initialized->wait();
    WINRT_ASSERT(m_session != nullptr);
}

void SimpleCapture::StartCapture()
{
    CheckClosed();
    m_session.StartCapture();
}

ICompositionSurface SimpleCapture::CreateSurface(
    Compositor const& compositor)
{
    CheckClosed();
    return CanvasComposition::CreateCompositionSurfaceForSwapChain(compositor, m_swapChain);
}

void SimpleCapture::Close()
{
    auto expected = false;
    if (m_closed.compare_exchange_strong(expected, true))
    {
		// We could explicitly call close, or we could just release
		// our references. When the ref count drops to 0, Close is 
		// called automatically. In garbage collected environments, 
		// you should call Close/Dispose, but you don't need to for C++.
        //m_session.Close();
        //m_framePool.Close();
        //m_swapChain.Close();

		m_session = nullptr;
		m_framePool = nullptr;
        m_item = nullptr;
		m_swapChain = nullptr;

        auto ignored = m_dispatcherQueueController.ShutdownQueueAsync();
        m_dispatcherQueueController = nullptr;
        m_dispatcherQueue = nullptr;
    }
}

void SimpleCapture::OnFrameArrived(
    Direct3D11CaptureFramePool const& sender, 
    IInspectable const&)
{
    auto newSize = false;

    {
        auto frame = sender.TryGetNextFrame();

        if (frame.ContentSize().Width != m_lastSize.Width ||
            frame.ContentSize().Height != m_lastSize.Height)
        {
            // The thing we have been capturing has changed size.
            // We need to resize our swap chain first, then blit the pixels.
            // After we do that, retire the frame and then recreate our frame pool.
            newSize = true;
            m_lastSize = frame.ContentSize();
            m_swapChain.ResizeBuffers((float)m_lastSize.Width, (float)m_lastSize.Height);
        }

        {
            auto bitmap = CanvasBitmap::CreateFromDirect3D11Surface(m_device, frame.Surface());
            auto drawingSession = m_swapChain.CreateDrawingSession(Colors::Transparent());

            drawingSession.DrawImage(bitmap);
        }
    }

    m_swapChain.Present();

    if (newSize)
    {
        m_framePool.Recreate(
            m_device,
            DirectXPixelFormat::B8G8R8A8UIntNormalized,
            2,
            m_lastSize);
    }
}

