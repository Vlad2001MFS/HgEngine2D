#include "GameObject.hpp"
#include "hd/Math/MathUtils.hpp"
#include "hd/IO/FileStream.hpp"

namespace hg {

GameObject::~GameObject() {
    destroyAllComponents();
    destroyAllChildren();
}

GameObject *GameObject::createChild(const std::string &name) {
    hd::StringHash nameHash = hd::StringHash(name);
    if (!name.empty() && mChildrenByNames.count(nameHash) != 0) {
        HD_LOG_FATAL("Failed to create child '{}'", name);
    }

    GameObject *go = new GameObject();
    mChildren.push_back(go);
    go->mParent = this;
    go->mName = name;
    if (!name.empty()) {
        mChildrenByNames.insert(std::make_pair(nameHash, go));
    }

    return go;
}

GameObject *GameObject::createChildFromFile(const std::string &name, const std::string &path) {
    hd::FileStream file = hd::FileStream(mGetFullPath(path), hd::FileMode::Read);
    std::string text = file.readAllText();

    hd::JSON data = hd::JSON::parse(text);
    GameObject *child = createChild(name);
    child->mOnSaveLoad(data, true);
    child->mName = name;
    return child;
}

void GameObject::saveToFile(const std::string &path) {
    hd::JSON data;
    mOnSaveLoad(data, false);
    std::string text = data.dump(2);

    hd::FileStream file = hd::FileStream(mGetFullPath(path), hd::FileMode::Write);
    file.writeLine(text);
}

void GameObject::destroyChild(const std::string &name) {
    GameObject *ptr = findChildByName(name);
    if (ptr) {
        mChildren.erase(std::remove(mChildren.begin(), mChildren.end(), ptr));
        mChildrenByNames.erase(hd::StringHash(name));
        delete ptr;
    }
}

Component *GameObject::createComponent(const hd::StringHash &typeHash) {
    Component *component = Factory::get().createObject(typeHash)->as<Component>();
    if (mAddComponent(component)) {
        return component;
    }
    else {
        return nullptr;
    }
}

void GameObject::destroyComponent(const hd::StringHash &typeHash) {
    auto it = std::find_if(mComponents.begin(), mComponents.end(), [&](Component *comp) { return comp->getTypeHash() == typeHash; });
    if (it != mComponents.end()) {
        HD_DELETE(*it);
        mComponents.erase(it);
    }
    else {
        HD_LOG_WARNING("Failed to destroy component '{}'", typeHash.getString());
    }
}

void GameObject::destroyAllChildren() {
    for (auto &it : mChildren) {
        HD_DELETE(it);
    }
    mChildren.clear();
    mChildrenByNames.clear();
}

void GameObject::destroyAllComponents() {
    for (auto &it : mComponents) {
        HD_DELETE(it);
    }
    mComponents.clear();
}

void GameObject::move(float x, float y) {
    glm::vec2 rotated = hd::MathUtils::rotate2D(x, y, getAbsoluteAngle());
    translate(rotated.x, rotated.y, 0);
}

void GameObject::move(const glm::vec2 &offset) {
    move(offset.x, offset.y);
}

void GameObject::translate(float x, float y, float z) {
    setPosition(mPos.x + x, mPos.y + y, mPos.z + z);
}

void GameObject::translate(const glm::vec3 &offset) {
    translate(offset.x, offset.y, offset.z);
}

void GameObject::scale(float x, float y) {
    setSize(mSize.x + x, mSize.y + y);
}

void GameObject::scale(const glm::vec2 &size) {
    scale(size.x, size.y);
}

void GameObject::rotate(float angle) {
    setAngle(mAngle + angle);
}

void GameObject::setActive(bool active) {
    mIsActive = active;
}

void GameObject::setPosition(float x, float y, float z) {
    mPos.x = x;
    mPos.y = y;
    mPos.z = z;
}

void GameObject::setPosition(const glm::vec3 &pos) {
    setPosition(pos.x, pos.y, pos.z);
}

void GameObject::setSize(float x, float y) {
    mSize.x = x;
    mSize.y = y;
}

void GameObject::setSize(const glm::vec2 &size) {
    setSize(size.x, size.y);
}

void GameObject::setAngle(float angle) {
    mAngle = angle;
}

GameObject *GameObject::findChildByName(const std::string &name) const {
    if (name.empty()) {
        HD_LOG_FATAL("Invalid name '{}'", name);
    }

    auto it = mChildrenByNames.find(hd::StringHash(name));
    if (it == mChildrenByNames.end()) {
        HD_LOG_FATAL("GameObject '{}' not found", name);
    }
    return it->second;
}

Component *GameObject::findComponent(const hd::StringHash &typeHash) const {
    auto it = std::find_if(mComponents.begin(), mComponents.end(), [&](Component *comp) { return comp->getTypeHash() == typeHash; });
    if (it != mComponents.end()) {
        return *it;
    }
    else {
        HD_LOG_WARNING("Component '{}' not found", typeHash.getString());
        return nullptr;
    }
}

GameObject *GameObject::getParent() const {
    return mParent;
}

const std::vector<GameObject*> &GameObject::getChildren() const {
    return mChildren;
}

const std::unordered_map<hd::StringHash, GameObject*> &GameObject::getChildrenByNames() const {
    return mChildrenByNames;
}

const std::string &GameObject::getName() const {
    return mName;
}

bool GameObject::isActive() const {
    return mIsActive;
}

const glm::vec3 &GameObject::getPosition() const {
    return mPos;
}

const glm::vec2 &GameObject::getSize() const {
    return mSize;
}

float GameObject::getAngle() const {
    return mAngle;
}

glm::vec3 GameObject::getAbsolutePosition() const {
    glm::vec3 pos = glm::vec3(0, 0, 0);
    const GameObject *go = this;
    while (go) {
        const GameObject *parent = go->getParent();
        if (parent) {
            glm::vec2 p = hd::MathUtils::rotate2D(go->getPosition(), parent->getAbsoluteAngle());
            pos += glm::vec3(p, go->getPosition().z);
        }
        else {
            pos += go->getPosition();
        }
        go = parent;
    }
    return pos;
}

float GameObject::getAbsoluteAngle() const {
    float angle = 0.0f;

    const GameObject *go = this;
    while (go) {
        angle += go->getAngle();
        go = go->mParent;
    }

    return angle;
}

void GameObject::mOnSaveLoad(hd::JSON &data, bool isLoad) {
    hd::JSON &components = data["components"];
    hd::JSON &children = data["children"];

    if (isLoad) {
        mName = data["name"];
        setActive(data["isActive"]);
        setPosition(data["position"]);
        setSize(data["size"]);
        setAngle(data["angle"]);

        for (auto &it : children) {
            GameObject *child = createChild(it["name"].get<std::string>());
            child->mOnSaveLoad(it, isLoad);
        }

        for (auto &it : components.items()) {
            std::string compName = it.key();
            Component *comp = createComponent(hd::StringHash(compName));
            comp->onSaveLoad(it.value(), isLoad);
        }
    }
    else {
        data["name"] = getName();
        data["isActive"] = isActive();
        data["position"] = getPosition();
        data["size"] = getSize();
        data["angle"] = getAngle();

        for (const auto &it : mChildren) {
            hd::JSON child;
            it->mOnSaveLoad(child, isLoad);
            children.push_back(child);
        }

        for (const auto &it : mComponents) {
            hd::JSON &comp = components[it->getTypeName()];
            it->onSaveLoad(comp, isLoad);
        }
    }
}

void GameObject::mOnEvent(const WindowEvent &event) {
    if (isActive()) {
        for (auto &it : mComponents) {
            it->onEvent(event);
        }

        for (const auto &it : mChildren) {
            it->mOnEvent(event);
        }
    }
}

void GameObject::mOnFixedUpdate() {
    if (isActive()) {
        for (auto &it : mComponents) {
            it->onFixedUpdate();
        }

        for (const auto &it : mChildren) {
            it->mOnFixedUpdate();
        }
    }
}

void GameObject::mOnUpdate(float dt) {
    if (isActive()) {
        for (auto &it : mComponents) {
            it->onUpdate(dt);
        }

        for (const auto &it : mChildren) {
            it->mOnUpdate(dt);
        }
    }
}

std::string GameObject::mGetFullPath(const std::string &path) {
    return "./data/configs/" + path;
}

bool GameObject::mAddComponent(Component *component) {
    component->mOwner = this;
    if (!component->onInitialize()) {
        HD_LOG_ERROR("Failed to add component '{}' because of it was not successfully initialized", component->getTypeName());
        HD_DELETE(component);
        return false;
    }

    auto it = std::find_if(mComponents.begin(), mComponents.end(), [&](Component *comp) {
        return comp->getTypeHash() == component->getTypeHash();
    });
    if (it == mComponents.end()) {
        mComponents.push_back(component);
        return true;
    }
    else {
        HD_LOG_ERROR("Failed to add component '{}' because of component such type already exist", component->getTypeName());
        HD_DELETE(component);
        return false;
    }
}

}
