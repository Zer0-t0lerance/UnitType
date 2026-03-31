#pragma once
#include <vector>
#include <memory>
#include <cassert>
#include "UnitType.h" 

namespace unitype {

constexpr size_t CHUNK_SIZE = 4096;

template <typename T, size_t N>
class HistoryContainer;

template <typename T, size_t N>
class SystemState;

/**
 * @brief Универсальная Линза (Proxy) для доступа к данным объекта.
 */
template <typename T, size_t N>
class StateProxy {
private:
    SystemState<T, N>* sys; 
    size_t id;              

public:
    StateProxy(SystemState<T, N>* system, size_t index) : sys(system), id(index) {}

    // Единая точка доступа к N-мерному массиву параметров
    UnitType<T, N>& data();
    const UnitType<T, N>& data() const;
};

/**
 * @brief Абстрактное чанковое хранилище. Никакой привязки к физике!
 */
template <typename T, size_t N>
class SystemState {
private:
    friend class StateProxy<T, N>; 
    friend class HistoryContainer<T, N>; 

    struct Chunk {
        std::vector<UnitType<T, N>> data; // <--- Только сырые данные

        Chunk() {
            data.resize(CHUNK_SIZE);
        }
    };

    std::vector<std::unique_ptr<Chunk>> chunks; 
    size_t active_count; 

public:
    explicit SystemState(size_t initial_size = 0) : active_count(0) {
        allocate_up_to(initial_size);
    }

    size_t size() const { return active_count; }

    size_t allocate_new() {
        size_t id = active_count++;
        if (id >= chunks.size() * CHUNK_SIZE) {
            chunks.push_back(std::make_unique<Chunk>());
        }
        return id;
    }

    void allocate_up_to(size_t total_size) {
        while (chunks.size() * CHUNK_SIZE < total_size) {
            chunks.push_back(std::make_unique<Chunk>());
        }
        if (total_size > active_count) {
            active_count = total_size;
        }
    }

    StateProxy<T, N> operator[](size_t index) {
        assert(index < active_count && "Выход за пределы массива SystemState!");
        return StateProxy<T, N>(this, index);
    }

    const StateProxy<T, N> operator[](size_t index) const {
        assert(index < active_count && "Выход за пределы массива SystemState!");
        return StateProxy<T, N>(const_cast<SystemState<T, N>*>(this), index);
    }
};

template <typename T, size_t N>
UnitType<T, N>& StateProxy<T, N>::data() {
    return sys->chunks[id / CHUNK_SIZE]->data[id % CHUNK_SIZE];
}

template <typename T, size_t N>
const UnitType<T, N>& StateProxy<T, N>::data() const {
    return sys->chunks[id / CHUNK_SIZE]->data[id % CHUNK_SIZE];
}

} // namespace unitype