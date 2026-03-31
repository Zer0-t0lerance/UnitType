#pragma once
#include <vector>
#include <memory>
#include <cassert>
#include "UnitType.h" 

namespace unitype {

// Размер одного блока (страницы) памяти. 4096 гарантирует идеальную SIMD-векторизацию
// и полное отсутствие кэш-промахов внутри одного блока.
constexpr size_t CHUNK_SIZE = 4096;

template <typename T, size_t N>
class HistoryContainer;

template <typename T, size_t N>
class SystemState;

/**
 * @brief Линза (Proxy) для доступа к параметрам одного объекта в системе.
 * Теперь она автоматически маршрутизирует запросы в нужный чанк памяти.
 */
template <typename T, size_t N>
class StateProxy {
private:
    SystemState<T, N>* sys; 
    size_t id;              

public:
    StateProxy(SystemState<T, N>* system, size_t index) : sys(system), id(index) {}

    UnitType<T, N>& coords();
    UnitType<T, N>& velocity();
    UnitType<T, N>& acceleration();
    T& time();

    const UnitType<T, N>& coords() const;
    const UnitType<T, N>& velocity() const;
    const UnitType<T, N>& acceleration() const;
    const T& time() const;
};

/**
 * @brief Хранилище состояния N-мерной системы.
 * Использует Чанковую (Страничную) память. Гарантирует, что при добавлении
 * новых объектов сырые указатели на старые объекты НИКОГДА не инвалидируются.
 */
template <typename T, size_t N>
class SystemState {
private:
    friend class StateProxy<T, N>;
    friend class HistoryContainer<T, N>;

    // Внутренний непрерывный блок памяти на CHUNK_SIZE элементов
    struct Chunk {
        std::vector<UnitType<T, N>> r; 
        std::vector<UnitType<T, N>> v; 
        std::vector<UnitType<T, N>> a; 
        std::vector<T> t;              

        Chunk() {
            r.resize(CHUNK_SIZE);
            v.resize(CHUNK_SIZE);
            a.resize(CHUNK_SIZE);
            t.resize(CHUNK_SIZE, T{0});
        }
    };

    // Массив указателей на чанки. Указатели гарантируют, 
    // что при росте вектора chunks сами блоки памяти остаются на местах.
    std::vector<std::unique_ptr<Chunk>> chunks; 
    size_t active_count; 

public:
    explicit SystemState(size_t initial_size = 0) : active_count(0) {
        allocate_up_to(initial_size);
    }

    size_t size() const { return active_count; }

    /** @brief Добавляет один объект и возвращает его ID. Автоматически выделяет новый чанк при необходимости. */
    size_t allocate_new() {
        size_t id = active_count++;
        if (id >= chunks.size() * CHUNK_SIZE) {
            chunks.push_back(std::make_unique<Chunk>());
        }
        return id;
    }

    /** @brief Безопасно расширяет систему до нужного количества объектов. */
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

// =================================================================
// РЕАЛИЗАЦИЯ МЕТОДОВ StateProxy
// =================================================================

template <typename T, size_t N>
UnitType<T, N>& StateProxy<T, N>::coords() {
    return sys->chunks[id / CHUNK_SIZE]->r[id % CHUNK_SIZE];
}

template <typename T, size_t N>
UnitType<T, N>& StateProxy<T, N>::velocity() {
    return sys->chunks[id / CHUNK_SIZE]->v[id % CHUNK_SIZE];
}

template <typename T, size_t N>
UnitType<T, N>& StateProxy<T, N>::acceleration() {
    return sys->chunks[id / CHUNK_SIZE]->a[id % CHUNK_SIZE];
}

template <typename T, size_t N>
T& StateProxy<T, N>::time() {
    return sys->chunks[id / CHUNK_SIZE]->t[id % CHUNK_SIZE];
}

// Константные версии
template <typename T, size_t N>
const UnitType<T, N>& StateProxy<T, N>::coords() const { return sys->chunks[id / CHUNK_SIZE]->r[id % CHUNK_SIZE]; }
template <typename T, size_t N>
const UnitType<T, N>& StateProxy<T, N>::velocity() const { return sys->chunks[id / CHUNK_SIZE]->v[id % CHUNK_SIZE]; }
template <typename T, size_t N>
const UnitType<T, N>& StateProxy<T, N>::acceleration() const { return sys->chunks[id / CHUNK_SIZE]->a[id % CHUNK_SIZE]; }
template <typename T, size_t N>
const T& StateProxy<T, N>::time() const { return sys->chunks[id / CHUNK_SIZE]->t[id % CHUNK_SIZE]; }

} // namespace unitype