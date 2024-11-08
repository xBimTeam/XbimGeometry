#pragma once

#include <unordered_map>
#include <mutex>
#include <optional>
#include <Standard_Handle.hxx>
#include <Standard_Transient.hxx>

class Cache {
public:

    // Inserts a value into the cache.
    void Insert(int id, const Handle(Standard_Transient)& value) {
        std::lock_guard<std::mutex> lock(_mutex);
        _cache[id] = value;
    }

    std::optional<Handle(Standard_Transient)> Get(int id) const {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _cache.find(id);
        if (it != _cache.end()) {
            return it->second;
        }
        // Not found
        return std::nullopt;
    }

    void Remove(int id) {
        std::lock_guard<std::mutex> lock(_mutex);
        _cache.erase(id);
    }

    bool Contains(int id) const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _cache.find(id) != _cache.end();
    }

    void Clear() {
        std::lock_guard<std::mutex> lock(_mutex);
        _cache.clear();
    }

private:
    mutable std::mutex _mutex;
    std::unordered_map<int, Handle(Standard_Transient)> _cache;
};
