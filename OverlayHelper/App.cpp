#include "pch.h"
#include "Capture.h"
#include "DoubleTapHelper.h"

using namespace winrt;

using namespace Windows;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Numerics;
using namespace Windows::Graphics::Capture;
using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::UI::Composition;
using namespace Windows::UI::Popups;
using namespace Microsoft::Graphics::Canvas;

struct App : implements<App, IFrameworkViewSource, IFrameworkView>
{
    IFrameworkView CreateView()
    {
        return *this;
    }

    void Initialize(CoreApplicationView const &)
    {
    }

    void Load(hstring const&)
    {
    }

    void Uninitialize()
    {
    }

    void SetWindow(CoreWindow const &)
    {
    }

    void Run()
    {
        CoreWindow window = CoreWindow::GetForCurrentThread();
        CoreDispatcher dispatcher = window.Dispatcher();

        m_compositor = Compositor();
        m_target = m_compositor.CreateTargetForCurrentView();
        m_root = m_compositor.CreateSpriteVisual();
        m_content = m_compositor.CreateSpriteVisual();
        m_brush = m_compositor.CreateSurfaceBrush();

        m_root.RelativeSizeAdjustment({ 1, 1 });
        m_root.Brush(m_compositor.CreateColorBrush(Colors::White()));
        m_target.Root(m_root);

        if (GraphicsCaptureSession::IsSupported())
        {
            m_content.AnchorPoint({ 0.5f, 0.5f });
            m_content.RelativeOffsetAdjustment({ 0.5f, 0.5f, 0 });
            m_content.RelativeSizeAdjustment({ 1, 1 });
            m_content.Size({ -80, -80 });
            m_content.Brush(m_brush);
            m_brush.HorizontalAlignmentRatio(0.5f);
            m_brush.VerticalAlignmentRatio(0.5f);
            m_brush.Stretch(CompositionStretch::Uniform);
            auto shadow = m_compositor.CreateDropShadow();
            shadow.Mask(m_brush);
            m_content.Shadow(shadow);
            m_root.Children().InsertAtTop(m_content);

            m_device = Microsoft::Graphics::Canvas::CanvasDevice();

            // We can't just call the picker here, because no one is pumping messages yet.
            // By asking the dispatcher for our UI thread to run this, we ensure that the
            // message pump is pumping messages by the time this runs.
            auto ignored = dispatcher.RunAsync(CoreDispatcherPriority::Normal, [=]() -> void
            {
                auto ignoredAction = StartCaptureAsync();
            });

			m_doubleTapHelper = std::make_unique<DoubleTapHelper>(window);
			m_doubleTapHelper->DoubleTapped([=](auto &&, auto&&)
			{
				StopCapture();
				auto ignored = StartCaptureAsync();
			});
        }
        else
        {
            auto ignored = dispatcher.RunAsync(CoreDispatcherPriority::Normal, [=]() -> void
            {
                auto dialog = MessageDialog(L"Screen capture is not supported on this device for this release of Windows!");

                auto ignoredOperation = dialog.ShowAsync();
            });
        }
        
        window.Activate();
        dispatcher.ProcessEvents(CoreProcessEventsOption::ProcessUntilQuit);
    }

    IAsyncAction StartCaptureAsync()
    {
        auto picker = GraphicsCapturePicker();
        auto item = co_await picker.PickSingleItemAsync();

        if (item != nullptr)
        {
            m_capture = std::make_unique<SimpleCapture>(m_device, item);

            auto surface = m_capture->CreateSurface(m_compositor);
            m_brush.Surface(surface);

            m_capture->StartCapture();
        }
    }

	void StopCapture()
	{
		m_capture->Close();
		m_brush.Surface(nullptr);
	}

    Compositor m_compositor{ nullptr };
    CompositionTarget m_target{ nullptr };
    SpriteVisual m_root{ nullptr };
    SpriteVisual m_content{ nullptr };
    CompositionSurfaceBrush m_brush{ nullptr };
    
    CanvasDevice m_device{ nullptr };
    std::unique_ptr<SimpleCapture> m_capture{ nullptr };
	std::unique_ptr<DoubleTapHelper> m_doubleTapHelper{ nullptr };
};

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    CoreApplication::Run(App());
}
