#pragma once

typedef std::shared_ptr<std::function<void(winrt::Windows::Foundation::Rect)>>
    RegionSelectHelper_RegionSelected;

class RegionSelectHelper
{
public:
    RegionSelectHelper(
        winrt::Windows::UI::Core::CoreWindow const& window,
        winrt::Windows::UI::Composition::ContainerVisual const& hostVisual);
    ~RegionSelectHelper() { Close(); }

    void Close();

    robmikh::event_token RegionSelected(RegionSelectHelper_RegionSelected const& handler) { return m_selected.add(handler); }
    void RegionSelected(robmikh::event_token const& token) { m_selected.remove(token); }

private:
    void OnDragging(
        winrt::Windows::UI::Input::GestureRecognizer const& sender,
        winrt::Windows::UI::Input::DraggingEventArgs const& args);

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
    robmikh::event<RegionSelectHelper_RegionSelected> m_selected;
    winrt::Windows::UI::Core::CoreWindow m_window{ nullptr };
    winrt::Windows::UI::Input::GestureRecognizer m_gestureRecognizer{ nullptr };

    winrt::Windows::UI::Composition::ContainerVisual m_hostVisual{ nullptr };
    winrt::Windows::UI::Composition::SpriteVisual m_visual{ nullptr };

    winrt::event_token m_pointerMoved;
    winrt::event_token m_pointerPressed;
    winrt::event_token m_pointerReleased;
    winrt::event_token m_dragging;

    winrt::Windows::Foundation::Numerics::float2 m_originalPosition;
};