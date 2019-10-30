#include "pch.h"
#include "MultiTapHelper.h"

using namespace winrt;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;

MultiTapHelper::MultiTapHelper(CoreWindow const& window, uint32_t tapCount)
{
    m_window = window;
    m_tapCount = tapCount;

    m_gestureRecognizer = GestureRecognizer();
    m_gestureRecognizer.GestureSettings(GestureSettings::DoubleTap | GestureSettings::RightTap);

    m_pointerMoved = m_window.PointerMoved({ this, &MultiTapHelper::Window_PointerMoved });
    m_pointerPressed = m_window.PointerPressed({ this, &MultiTapHelper::Window_PointerPressed });
    m_pointerReleased = m_window.PointerReleased({ this, &MultiTapHelper::Window_PointerReleased });

    m_tapped = m_gestureRecognizer.Tapped({ this, &MultiTapHelper::OnTapped });
    m_gestureRightTapped = m_gestureRecognizer.RightTapped({ this, &MultiTapHelper::OnRightTapped });
}

void MultiTapHelper::OnTapped(
    GestureRecognizer const& sender,
    TappedEventArgs const& args)
{
    if (args.TapCount() == m_tapCount)
    {
        sender.CompleteGesture();
        m_multiTapped(nullptr, nullptr);
    }
}

void MultiTapHelper::OnRightTapped(
    GestureRecognizer const& sender,
    RightTappedEventArgs const&)
{
    sender.CompleteGesture();
    m_rightTapped(nullptr, nullptr);
}

void MultiTapHelper::Close()
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