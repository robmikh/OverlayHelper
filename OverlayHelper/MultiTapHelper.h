#pragma once

typedef winrt::Windows::Foundation::TypedEventHandler<
    winrt::Windows::Foundation::IInspectable,
    winrt::Windows::Foundation::IInspectable>
MultiTapHelper_MultiTapped;

typedef winrt::Windows::Foundation::TypedEventHandler<
    winrt::Windows::Foundation::IInspectable,
    winrt::Windows::Foundation::IInspectable>
MultiTapHelper_RightTapped;

class MultiTapHelper
{
public:
    MultiTapHelper(
        winrt::Windows::UI::Core::CoreWindow const& window,
        uint32_t tapCount);
    ~MultiTapHelper() { Close(); }

    void Close();

    winrt::event_token MultiTapped(MultiTapHelper_MultiTapped const& handler) { return m_multiTapped.add(handler); }
    void MultiTapped(winrt::event_token const& token) { m_multiTapped.remove(token); }

    winrt::event_token RightTapped(MultiTapHelper_RightTapped const& handler) { return m_rightTapped.add(handler); }
    void RightTapped(winrt::event_token const& token) { m_rightTapped.remove(token); }

private:
    void OnTapped(
        winrt::Windows::UI::Input::GestureRecognizer const& sender,
        winrt::Windows::UI::Input::TappedEventArgs const& args);
    void OnRightTapped(
        winrt::Windows::UI::Input::GestureRecognizer const& sender,
        winrt::Windows::UI::Input::RightTappedEventArgs const& args);

    void Window_PointerReleased(
        winrt::Windows::UI::Core::CoreWindow const&,
        winrt::Windows::UI::Core::PointerEventArgs const& args)
    {
        m_gestureRecognizer.ProcessUpEvent(args.CurrentPoint());
    }

    void Window_PointerPressed(
        winrt::Windows::UI::Core::CoreWindow const&,
        winrt::Windows::UI::Core::PointerEventArgs const& args)
    {
        m_gestureRecognizer.ProcessDownEvent(args.CurrentPoint());
    }

    void Window_PointerMoved(
        winrt::Windows::UI::Core::CoreWindow const&,
        winrt::Windows::UI::Core::PointerEventArgs const& args)
    {
        m_gestureRecognizer.ProcessMoveEvents(args.GetIntermediatePoints());
    }

private:
    uint32_t m_tapCount = 0;
    winrt::event<MultiTapHelper_MultiTapped> m_multiTapped;
    winrt::event<MultiTapHelper_RightTapped> m_rightTapped;
    winrt::Windows::UI::Core::CoreWindow m_window{ nullptr };
    winrt::Windows::UI::Input::GestureRecognizer m_gestureRecognizer{ nullptr };

    winrt::event_token m_pointerMoved;
    winrt::event_token m_pointerPressed;
    winrt::event_token m_pointerReleased;
    winrt::event_token m_tapped;
    winrt::event_token m_gestureRightTapped;
};