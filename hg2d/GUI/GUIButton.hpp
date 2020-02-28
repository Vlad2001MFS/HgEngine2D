#pragma once
#include "GUIImage.hpp"
#include "GUILabel.hpp"

namespace hg2d {

class GUIButton : public GUIImage {
public:
    GUIButton();

    void setText(const std::string &text);
    const std::string &getText() const;

private:
    void mOnMouseButtonPressed();
    void mOnMouseButtonReleased();
    void mOnMouseEnter();
    void mOnMouseLeave();

    GUILabel *mLabel;
};

HG2D_REGISTER_OBJECT("GUIButton", GUIButton);

}