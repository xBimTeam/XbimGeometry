#pragma once

#include <unordered_map>
#include <mutex>
#include <optional>
#include <Standard_Handle.hxx>
#include <TopoDS_Shape.hxx>
#include <Standard_Transient.hxx>

class Cache {
public:

    void Insert(int id, const Handle(Standard_Transient)& value) {
        std::lock_guard<std::mutex> lock(_mutex);
        _cache[id] = value;
    }

    void InsertShape(int id, const TopoDS_Shape& value) {
        std::lock_guard<std::mutex> lock(_mutex);
        _shapeCache[id] = value;
    }

    std::optional<Handle(Standard_Transient)> Get(int id) const {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _cache.find(id);
        if (it != _cache.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    std::optional<TopoDS_Shape> GetShape(int id) const {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _shapeCache.find(id);
        if (it != _shapeCache.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    void Remove(int id) {
        std::lock_guard<std::mutex> lock(_mutex);
        _cache.erase(id);
    }

    void RemoveShape(int id) {
        std::lock_guard<std::mutex> lock(_mutex);
        _shapeCache.erase(id);
    }

    bool Contains(int id) const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _cache.find(id) != _cache.end();
    }

    bool ContainsShape(int id) const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _shapeCache.find(id) != _shapeCache.end();
    }

    void Clear() {
        std::lock_guard<std::mutex> lock(_mutex);
        _cache.clear();
        _shapeCache.clear();
    }

private:
    mutable std::mutex _mutex;
    std::unordered_map<int, Handle(Standard_Transient)> _cache;
    std::unordered_map<int, TopoDS_Shape> _shapeCache;
};