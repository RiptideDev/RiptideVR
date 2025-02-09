#include <algorithm>
#include "Shader.h"
#include "Instance.h"

std::atomic_int Instance::s_nextID{ 0 };
std::unordered_map<std::string, Instance::FactoryFunc> Instance::s_InstanceRegistry;
std::vector<Instance*> Instance::s_instances;

Instance::Instance() : m_id(s_nextID++) {
    s_instances.push_back(this);
}

Instance::~Instance() {
    Destroy();
    auto it = std::remove(s_instances.begin(), s_instances.end(), this);
    s_instances.erase(it, s_instances.end());
}

Instance* Instance::GetParent() const {
    return m_parent;
}

void Instance::SetParent(Instance* newParent) {
    if (m_parent == newParent || this == newParent || (newParent && newParent->m_destroyed)) return;

    if (newParent && newParent->FindFirstChild(GetName())) {
        return;
    }

    RemoveFromParent();

    m_parent = newParent;
    if (m_parent) {
        m_parent->m_children.push_back(this);
        m_parent->UpdateChildLookup(this, true, GetName());
    }
}

void Instance::RemoveFromParent() {
    if (m_parent) {
        auto& siblings = m_parent->m_children;
        siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
        m_parent->UpdateChildLookup(this, false, GetName());
        m_parent = nullptr;
    }
}

Instance* Instance::FindFirstChild(const std::string& name) const {
    auto it = m_childLookup.find(name);
    return it != m_childLookup.end() ? it->second : nullptr;
}

void Instance::UpdateChildLookup(Instance* child, bool add, const std::string& name) {
    if (add) {
        m_childLookup[name] = child;
    }
    else {
        auto it = m_childLookup.find(name);
        if (it != m_childLookup.end() && it->second == child) {
            m_childLookup.erase(it);
        }
    }
}

void Instance::Destroy() {
    if (m_destroyed) return;
    m_destroyed = true;
    RemoveFromParent();

    while (!m_children.empty()) {
        m_children.back()->Destroy();
    }
}

void Instance::PrintTree(int indentation) const {
    std::string indent(indentation, '\t');
    printf("%s%d: %s (%s)\n", indent.c_str(), m_id, GetName().c_str(), GetClassName().c_str());

    for (const auto& child : m_children) {
        child->PrintTree(indentation + 1);
    }
}

const std::vector<Instance*>& Instance::GetAllInstances() {
    return s_instances;
}

Instance* Instance::Create(const std::string& classname) {
    auto it = s_InstanceRegistry.find(classname);
    return it != s_InstanceRegistry.end() ? it->second() : nullptr;
}

void Instance::RegisterInstanceClass(const std::string& classname, FactoryFunc factory) {
    s_InstanceRegistry[classname] = factory;
}

void Instance::RegisterInstanceClasses() {
    RegisterInstanceClass("Shader", []() -> Instance* { return new Shader(); });
    RegisterInstanceClass("Instance", []() -> Instance* { return new Instance(); });
}

void Instance::SetName(const std::string& newName) {
    if (m_name == newName) return;

    if (m_parent && m_parent->FindFirstChild(newName)) {
        return;
    }

    std::string oldName = std::move(m_name);
    m_name = newName;

    if (m_parent) {
        m_parent->UpdateChildLookup(this, false, oldName);
        m_parent->UpdateChildLookup(this, true, newName);
    }
}