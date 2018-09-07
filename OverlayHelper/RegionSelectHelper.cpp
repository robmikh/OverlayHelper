#include "pch.h"
#include "RegionSelectHelper.h"

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Numerics;
using namespace Windows::UI;
using namespace Windows::UI::Composition;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;

RegionSelectHelper::RegionSelectHelper(
    CoreWindow const& window,
    ContainerVisual const& hostVisual)
{
    m_window = window;
    m_hostVisual = hostVisual;

    auto compositor = m_hostVisual.Compositor();
    m_visual = compositor.CreateSpriteVisual();
    m_visual.Brush(compositor.CreateColorBrush(Colors::Red()));
    m_visual.IsVisible(false);
    m_hostVisual.Children().InsertAtTop(m_visual);

    m_gestureRecognizer = GestureRecognizer();
    m_gestureRecognizer.GestureSettings(GestureSettings::Drag);

    m_pointerMoved = m_window.PointerMoved({ this, &RegionSelectHelper::Window_PointerMoved });
    m_pointerPressed = m_window.PointerPressed({ this, &RegionSelectHelper::Window_PointerPressed });
    m_pointerReleased = m_window.PointerReleased({ this, &RegionSelectHelper::Window_PointerReleased });

    m_dragging = m_gestureRecognizer.Dragging({ this, &RegionSelectHelper::OnDragging });
}

void RegionSelectHelper::OnDragging(
    GestureRecognizer const& sender,
    DraggingEventArgs const& args)
{
    switch (args.DraggingState())
    {
    case DraggingState::Started:
        {
            m_originalPosition = { args.Position().X, args.Position().Y };
            m_visual.Offset({ args.Position().X, args.Position().Y, 0 });
            m_visual.Size({ 0, 0 });
            m_visual.IsVisible(true);
        }
        break;
    case DraggingState::Continuing:
        {
            auto position = float2{ args.Position().X, args.Position().Y };
            
            auto delta = position - m_originalPosition;
            auto newSize = m_visual.Size();
            auto newOffset = m_visual.Offset();

            if (delta.x < 0)
            {
                newSize.x = delta.x * -1.0f;
                newOffset.x = position.x;
            }
            else
            {
                newSize.x = delta.x;
            }

            if (delta.y < 0)
            {
                newSize.y = delta.y * -1.0f;
                newOffset.y = position.y;
            }
            else
            {
                newSize.y = delta.y;
            }

            m_visual.Size(newSize);
            m_visual.Offset(newOffset);
        }
        break;
    case DraggingState::Completed:
        {
            sender.CompleteGesture();
            m_visual.IsVisible(false);
            auto rect = Rect(m_visual.Offset().x, m_visual.Offset().y, m_visual.Size().x, m_visual.Size().y);
            m_selected(rect);
        }
        break;
    default:
        throw std::exception("Unknown dragging state");
    }
}

void RegionSelectHelper::Close()
{
    if (m_window != nullptr)
    {
        m_window.PointerMoved(m_pointerMoved);
        m_window.PointerPressed(m_pointerPressed);
        m_window.PointerReleased(m_pointerReleased);
        m_gestureRecognizer.Dragging(m_dragging);

        m_window = nullptr;
        m_gestureRecognizer = nullptr;
    }
}