#pragma once

#include <vector>
#include <exception>
#include <iostream>

template <typename K, typename V, typename Hash = std::hash<K>, typename KeyEqual = std::equal_to<K>>
class HashTable {
public:
    HashTable() {
        hash_map_.resize(MIN_SIZE);
        is_empty_.resize(MIN_SIZE, true);
        elements_count_ = 0;
    }
    template <typename Collection>
    HashTable(const Collection& collection) {
        hash_map_.resize(MIN_SIZE);
        is_empty_.resize(MIN_SIZE, true);
        elements_count_ = 0;
        for (const auto [key, value] : collection) {
            AddElement(key, value);
        }
    }

    HashTable(const HashTable& table)
        : hash_map_(table.hash_map_), is_empty_(table.is_empty_), elements_count_(table.elements_count_) {
    }

    HashTable(HashTable&& table) {
        std::swap(hash_map_, table.hash_map_);
        std::swap(is_empty_, table.is_empty_);
        elements_count_ = table.elements_count_;
    }

    HashTable& operator=(const HashTable& table) {
        hash_map_ = table.hash_map_;
        is_empty_ = table.is_empty_;
        elements_count_ = table.elements_count_;
        return *this;
    }

    HashTable& operator=(HashTable&& table) {
        std::swap(hash_map_, table.hash_map_);
        std::swap(is_empty_, table.is_empty_);
        elements_count_ = table.elements_count_;
        return *this;
    }

    V& operator[](const K& key) {
        size_t index = Find(key);
        if (is_empty_[index]) {
            AddElement(key, V{});
            return hash_map_[Find(key)].second;
        }
        return hash_map_[index].second;
    }

    V& at(const K& key) {
        size_t index = Find(key);
        if (is_empty_[index]) {
            throw std::exception();
        }
        return hash_map_[index].second;
    }

    size_t size() {
        return elements_count_;
    }

    bool empty() {
        return elements_count_ == 0;
    }

    void clear() {
        hash_map_.resize(MIN_SIZE);
        is_empty_.resize(MIN_SIZE, true);
        elements_count_ = 0;
    }

private:
    HashTable(const size_t size) {
        hash_map_.resize(size);
        is_empty_.resize(size, true);
        elements_count_ = 0;
    }

    size_t Find(const K& key) const {
        size_t index = Hash{}(key) % hash_map_.size();
        while (!is_empty_[index] && !KeyEqual{}(key, hash_map_[index].first)) {
            ++index;
            if (index == hash_map_.size()) {
                index = 0;
            }
        }
        return index;
    }

    void Rebuild() {
        HashTable new_table(hash_map_.size() * 2);
        for (size_t i = 0; i < hash_map_.size(); ++i) {
            if (!is_empty_[i]) {
                new_table.AddElement(hash_map_[i]);
            }
        }
        *this = new_table;
    }

    void AddElement(const std::pair<K, V>& pair) {
        AddElement(pair.first, pair.second);
    }

    void AddElement(const K& key, const V& val) {
        size_t index = Find(key);
        hash_map_[index] = {key, val};
        is_empty_[index] = false;
        ++elements_count_;
        if (static_cast<double>(elements_count_) > hash_map_.size() * MAX_LOAD_FACTOR) {
            Rebuild();
        }
    }

private:
    class Iterator {
    public:
        Iterator(HashTable& owner, size_t index) : owner_(owner), index_(index) {
        }

        Iterator& operator++() {
            do {
                ++index_;
            } while (index_ != owner_.hash_map_.size() && owner_.is_empty_[index_]);
            return *this;
        }
        bool operator!=(const Iterator& other) const {
            return (&owner_ != &other.owner_ || index_ != other.index_);
        }
        std::pair<K, V>& operator*() {
            return owner_.hash_map_[index_];
        }

        std::pair<K, V>* operator->() {
            return &owner_.hash_map_[index_];
        }

    private:
        HashTable& owner_;
        size_t index_;
    };

    class ConstIterator {
    public:
        ConstIterator(const HashTable& owner, size_t index) : owner_(owner), index_(index) {
        }

        ConstIterator& operator++() {
            do {
                ++index_;
            } while (index_ != owner_.hash_map_.size() && owner_.is_empty_[index_]);
            return *this;
        }

        bool operator!=(const ConstIterator& other) const {
            return (&owner_ != &other.owner_ || index_ != other.index_);
        }
        const std::pair<K, V>& operator*() const {
            return owner_.hash_map_[index_];
        }

        const std::pair<K, V>* operator->() const {
            return &owner_.hash_map_[index_];
        }

    private:
        const HashTable& owner_;
        size_t index_;
    };

public:
    Iterator end() {
        return Iterator(*this, hash_map_.size());
    }

    Iterator find(const K& key) {
        size_t index = Find(key);
        if (is_empty_[index]) {
            return end();
        }
        return Iterator(*this, index);
    }

    Iterator begin() {
        for (size_t index = 0; index < hash_map_.size(); ++index) {
            if (!is_empty_[index]) {
                return Iterator(*this, index);
            }
        }
        return end();
    }

    ConstIterator end() const {
        return ConstIterator(*this, hash_map_.size());
    }

    ConstIterator begin() const {
        for (size_t index = 0; index < hash_map_.size(); ++index) {
            if (!is_empty_[index]) {
                return ConstIterator(*this, index);
            }
        }
        return end();
    }

    std::pair<Iterator, bool> insert(const std::pair<K, V>& pair) {
        size_t index = Find(pair.first);
        if (is_empty_[index]) {
            AddElement(pair);
            return {Iterator(*this, Find(pair.first)), true};
        }
        return {Iterator(*this, index), false};
    }

    template <typename... Args>
    std::pair<Iterator, bool> emplace(const Args&... args) {
        std::pair<K, V> pair(args...);
        return insert(pair);
    }

private:
    std::vector<std::pair<K, V>> hash_map_;
    std::vector<bool> is_empty_;
    size_t elements_count_;

    const double MAX_LOAD_FACTOR = 1. / 4;
    const size_t MIN_SIZE = 12;
};
