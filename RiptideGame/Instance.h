#pragma once
#include <unordered_map>
#include <string>
#include <vector>

#include <atomic>
#include <memory>


class Instance {
public:
    using FactoryFunc = Instance * (*)();
    std::string Name;

    Instance();
    virtual ~Instance();

    Instance* GetParent() const;
    void SetParent(Instance* newParent);
    Instance* FindFirstChild(const std::string& name) const;

    virtual void Destroy();
    bool IsDestroyed() const { return m_destroyed; }
    int GetID() const { return m_id; }

    void PrintTree(int indentation = 0) const;

    virtual std::string GetClassName() const { return "Instance"; }

    virtual void Update(float dt) {}
    std::vector<Instance*> GetChildren();
    static const std::vector<Instance*>& GetAllInstances();
    static Instance* Create(std::string classname);
    static void RegisterInstanceClass(std::string classname, FactoryFunc);
    static void RegisterInstanceClasses();

private:
    int m_id;
    bool m_destroyed = false;
    Instance* m_parent = nullptr;
    std::vector<Instance*> m_children;
    std::unordered_map<std::string, Instance*> m_childLookup;

    static std::unordered_map<std::string, FactoryFunc> s_InstanceRegistry;

    static std::atomic_int s_nextID;

    void UpdateChildLookup(Instance* child, bool add);
    void RemoveFromParent();
};