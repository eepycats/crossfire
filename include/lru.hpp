#pragma once
#include <list>
#include <unordered_map>

template<typename Key, typename Value>
class LRUCache {
public:
    using KeyValuePair = std::pair<Key, Value>;
    using ListIterator = typename std::list<KeyValuePair>::iterator;

    explicit LRUCache(size_t maxSize) : _capacity(maxSize) {}

    bool get(const Key& key, Value& value) {
        auto it = _cacheMap.find(key);
        if (it == _cacheMap.end()) return false;

        _cacheItems.splice(_cacheItems.begin(), _cacheItems, it->second);
        value = it->second->second;
        return true;
    }

    void put(const Key& key, const Value& value) {
        auto it = _cacheMap.find(key);

        if (it != _cacheMap.end()) {
            it->second->second = value;
            _cacheItems.splice(_cacheItems.begin(), _cacheItems, it->second);
        } else {
            if (_cacheItems.size() >= _capacity) {
                auto last = _cacheItems.back();
                _cacheMap.erase(last.first);
                _cacheItems.pop_back();
            }

            _cacheItems.emplace_front(key, value);
            _cacheMap[key] = _cacheItems.begin();
        }
    }

    bool update(const Key& key, const Value& newValue) {
        auto it = _cacheMap.find(key);
        if (it == _cacheMap.end()) return false;

        it->second->second = newValue;
        _cacheItems.splice(_cacheItems.begin(), _cacheItems, it->second);
        return true;
    }

    bool touch(const Key& key) {
        auto it = _cacheMap.find(key);
        if (it == _cacheMap.end()) return false;

        _cacheItems.splice(_cacheItems.begin(), _cacheItems, it->second);
        return true;
    }

    bool contains(const Key& key) const {
        return _cacheMap.find(key) != _cacheMap.end();
    }

    void clear() {
        _cacheItems.clear();
        _cacheMap.clear();
    }

    size_t size() const {
        return _cacheItems.size();
    }

private:
    size_t _capacity;
    std::list<KeyValuePair> _cacheItems;
    std::unordered_map<Key, ListIterator> _cacheMap;
};
