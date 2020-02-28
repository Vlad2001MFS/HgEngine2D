#pragma once
#include "../Core/Node.hpp"
#include <functional>

namespace hg2d {

enum class GUIHAlign {
    None,
    Left,
    Center,
    Right
};

enum class GUIVAlign {
    None,
    Top,
    Center,
    Bottom
};

class GUIWidget : public Node {
public:
    GUIWidget();
    
    virtual void onEvent(const hd::WindowEvent &event) override;
    virtual void onFixedUpdate() override;

    void setHAlign(GUIHAlign align);
    void setVAlign(GUIVAlign align);
    void setAlign(GUIHAlign hAlign, GUIVAlign vAlign);

    GUIHAlign getHAlign() const;
    GUIVAlign getVAlign() const;
    bool isMouseHovered() const;

    std::function<void()> onMouseButtonPressed, onMouseButtonReleased;
    std::function<void()> onMouseEnter, onMouseLeave;

private:
    void mApplyHAlign();
    void mApplyVAlign();

    bool mIsMouseHovered;
    GUIHAlign mHAlign;
    GUIVAlign mVAlign;
};

HG2D_REGISTER_OBJECT("GUIWidget", GUIWidget);

}