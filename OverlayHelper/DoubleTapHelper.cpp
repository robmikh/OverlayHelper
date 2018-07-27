#include "pch.h"
#include "DoubleTapHelper.h"

using namespace winrt;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;

DoubleTapHelper::DoubleTapHelper(CoreWindow const& window)
{
    m_window = window;

    m_gestureRecognizer = GestureRecognizer();
    m_gestureRecognizer.GestureSettings(GestureSettings::DoubleTap);

    m_pointerMoved = m_window.PointerMoved({ this, &DoubleTapHelper::Window_PointerMoved });
    m_pointerPressed = m_window.PointerPressed({ this, &DoubleTapHelper::Window_PointerPressed });
    m_pointerReleased = m_window.PointerReleased({ this, &DoubleTapHelper::Window_PointerReleased });

    m_tapped = m_gestureRecognizer.Tapped({ this, &DoubleTapHelper::OnTapped });
}

void DoubleTapHelper::OnTapped(
    GestureRecognizer const& sender,
    TappedEventArgs const& args)
{
    if (args.TapCount() == 2)
    {
        sender.CompleteGesture();
        m_doubleTapped(nullptr, nullptr);
    }
}

void DoubleTapHelper::Close()
{
    if (m_window != nullptr)
    {
        m_window.PointerMoved(m_pointerMoved);
        m_window.PointerPressed(m_pointerPressed);
        m_window.PointerReleased(m_pointerReleased);
        m_gestureRecognizer.Tapped(m_tapped);

        m_window = nullptr;
        m_gestureRecognizer = nullptr;
    }
}