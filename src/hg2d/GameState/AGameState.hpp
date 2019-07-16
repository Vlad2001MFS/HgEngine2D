#pragma once
#include "../Core/AEngineObject.hpp"
#include "hd/System/hdWindowEvent.hpp"

namespace hg2d {

class Engine;

class AGameState : public AEngineObject {
public:
    explicit AGameState(Engine &engine);
    virtual ~AGameState();

    virtual void onInitialize() {}
    virtual void onShutdown() {}
    virtual void onEvent(const hd::WindowEvent &event) {}
    virtual void onFixedUpdate() {}
    virtual void onUpdate() {}
    virtual void onDraw() {}
};

}
