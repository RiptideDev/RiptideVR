#include <algorithm>
#include "Shader.h"
#include "Instance.h"



std::atomic_int Instance::s_nextID{ 0 };

std::unordered_map<std::string, Instance::FactoryFunc> Instance::s_InstanceRegistry;

Instance::Instance() : m_id(s_nextID++) {}

Instance::~Instance() {
    Destroy();
}

Instance* Instance::GetParent() const
{
    return m_parent;
}

// Hierarchy management
void Instance::SetParent(Instance* newParent) {
    if (m_parent == newParent || this == newParent) return;
    if (newParent && newParent->IsDestroyed()) return;

    RemoveFromParent();

    m_parent = newParent;
    if (m_parent) {
        m_parent->m_children.push_back(this);
        m_parent->UpdateChildLookup(this, true);
    }
}

void Instance::RemoveFromParent() {
    if (m_parent) {
        auto& siblings = m_parent->m_children;
        siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
        m_parent->UpdateChildLookup(this, false);
        m_parent = nullptr;
    }
}

// Fast child lookup
Instance* Instance::FindFirstChild(const std::string& name) const {
    auto it = m_childLookup.find(name);
    return it != m_childLookup.end() ? it->second : nullptr;
}

void Instance::UpdateChildLookup(Instance* child, bool add) {
    if (add) {
        m_childLookup[child->Name] = child;
    }
    else {
        auto it = m_childLookup.find(child->Name);
        if (it != m_childLookup.end() && it->second == child) {
            m_childLookup.erase(it);
        }
    }
}

// Memory management
void Instance::Destroy() {
    if (m_destroyed) return;
    m_destroyed = true;

    // Destroy children first
    while (!m_children.empty()) {
        m_children.back()->Destroy();
    }

    RemoveFromParent();
}

// Debugging
void Instance::PrintTree(int indentation) const {
    std::string indent(indentation, '\t');
    printf("%s%d: %s (%s)\n", indent.c_str(), m_id, Name.c_str(), GetClassName().c_str());

    for (const auto& child : m_children) {
        child->PrintTree(indentation + 1);
    }
}

std::vector<Instance*> Instance::GetChildren()
{
    return m_children;
}

// Static instance management
const std::vector<Instance*>& Instance::GetAllInstances() {
    static std::vector<Instance*> instances;
    return instances;
}

Instance* Instance::Create(std::string classname)
{
    return nullptr;
}

void Instance::RegisterInstanceClass(std::string classname, FactoryFunc factory)
{
    s_InstanceRegistry.insert_or_assign(classname, factory);
}

void Instance::RegisterInstanceClasses()
{
    RegisterInstanceClass("Shader", static_cast<FactoryFunc>([]() -> Instance* {
        return new Shader();
    }));

    RegisterInstanceClass("Instance", static_cast<FactoryFunc>([]() -> Instance* {
        return new Instance();
    }));
}
