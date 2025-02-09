#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <atomic>
#include <memory>

enum class NetworkType {
    ReliableDynamic,
    ReliableStatic,
    UnreliableDynamic
};

class Instance {
public:
    using FactoryFunc = Instance * (*)();

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
    const std::vector<Instance*>& GetChildren() const { return m_children; }

    virtual char* GetNetworkPacket(float dt) { return nullptr; }
    virtual void ApplyNetworkPacket(float dt, const char* packet) {}
    int GetNetworkOwner() const { return m_net_owner; }

    const std::string& GetName() const { return m_name; }
    void SetName(const std::string& newName);

    static const std::vector<Instance*>& GetAllInstances();
    static Instance* Create(const std::string& classname);
    static void RegisterInstanceClass(const std::string& classname, FactoryFunc factory);
    static void RegisterInstanceClasses();

    NetworkType NetworkType = NetworkType::UnreliableDynamic;

private:
    int m_net_owner = 0;
    int m_id;
    bool m_destroyed = false;
    Instance* m_parent = nullptr;
    std::string m_name;
    std::vector<Instance*> m_children;
    std::unordered_map<std::string, Instance*> m_childLookup;

    static std::unordered_map<std::string, FactoryFunc> s_InstanceRegistry;
    static std::atomic_int s_nextID;
    static std::vector<Instance*> s_instances;

    void UpdateChildLookup(Instance* child, bool add, const std::string& name);
    void RemoveFromParent();
};