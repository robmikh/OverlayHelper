#pragma once

typedef std::shared_ptr<std::function<void(void)>>
    DoubleTapHelper_DoubleTapped;

class DoubleTapHelper
{
public:
    DoubleTapHelper(
        winrt::Windows::UI::Core::CoreWindow const& window);
    ~DoubleTapHelper() { Close(); }

    void Close();

    robmikh::event_token DoubleTapped(DoubleTapHelper_DoubleTapped const& handler) { return m_doubleTapped.add(handler); }
    void DoubleTapped(robmikh::event_token const& token) { m_doubleTapped.remove(token); }

private:
    void OnTapped(
        winrt::Windows::UI::Input::GestureRecognizer const& sender,
        winrt::Windows::UI::Input::TappedEventArgs const& args);

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
    robmikh::event<DoubleTapHelper_DoubleTapped> m_doubleTapped;
    winrt::Windows::UI::Core::CoreWindow m_window{ nullptr };
    winrt::Windows::UI::Input::GestureRecognizer m_gestureRecognizer{ nullptr };

    winrt::event_token m_pointerMoved;
    winrt::event_token m_pointerPressed;
    winrt::event_token m_pointerReleased;
    winrt::event_token m_tapped;
};