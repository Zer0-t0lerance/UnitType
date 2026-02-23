#pragma once
#include <vector>
#include <cassert>
#include "UnitType.h" 

namespace unitype {

template <typename T, size_t N>
class SystemState;

/**
 * @brief Линза (Proxy) для доступа к параметрам одного объекта в системе.
 * * Не хранит собственных данных. Обеспечивает ООП-интерфейс к плоским массивам 
 * класса SystemState, позволяя обращаться к координатам и скоростям конкретного объекта.
 */
template <typename T, size_t N>
class StateProxy {
private:
    SystemState<T, N>* sys; 
    size_t id;              

public:
    StateProxy(SystemState<T, N>* system, size_t index) : sys(system), id(index) {}

    UnitType<T, N>& coords()       { return sys->r[id]; }
    UnitType<T, N>& velocity()     { return sys->v[id]; }
    UnitType<T, N>& acceleration() { return sys->a[id]; }
    T& time() { return sys->t[id]; }

    const UnitType<T, N>& coords() const       { return sys->r[id]; }
    const UnitType<T, N>& velocity() const     { return sys->v[id]; }
    const UnitType<T, N>& acceleration() const { return sys->a[id]; }
    const T& time() const { return sys->t[id]; }
};

/**
 * @brief Хранилище состояния N-мерной системы в конкретный момент времени (Слепок).
 * * Организует данные по принципу SoA (Structure of Arrays) для максимальной 
 * производительности кэша процессора и SIMD-векторизации.
 * * @tparam T Базовый тип данных (double, float и т.д.).
 * @tparam N Размерность пространства задачи.
 */
template <typename T, size_t N>
class SystemState {
private:
    friend class StateProxy<T, N>; 

    // Внутренние плоские массивы данных
    std::vector<UnitType<T, N>> r; /// Основные координаты / Значения
    std::vector<UnitType<T, N>> v; /// Первые производные (Скорости)
    std::vector<UnitType<T, N>> a; /// Вторые производные (Ускорения)
    std::vector<T> t;              /// Локальное время объектов

    size_t count; 

public:
    /**
     * @brief Конструктор системы.
     * @param size Количество независимых объектов (или размер ансамбля/многообразия).
     */
    explicit SystemState(size_t size) : count(size) {
        r.resize(size);
        v.resize(size);
        a.resize(size);
        t.resize(size, T{0});
    }

    /** @brief Возвращает количество объектов в системе. */
    size_t size() const { return count; }

    /** @brief Создает и возвращает Линзу для доступа к объекту по индексу. */
    StateProxy<T, N> operator[](size_t index) {
        assert(index < count && "Выход за пределы массива SystemState!");
        return StateProxy<T, N>(this, index);
    }

    const StateProxy<T, N> operator[](size_t index) const {
        assert(index < count && "Выход за пределы массива SystemState!");
        return StateProxy<T, N>(const_cast<SystemState<T, N>*>(this), index);
    }

    /** @brief Прямой доступ к сырому массиву координат (Zero-copy read). */
    const std::vector<UnitType<T, N>>& get_coords() const { return r; }
    
    /** @brief Прямой доступ к сырому массиву скоростей (Zero-copy read). */
    const std::vector<UnitType<T, N>>& get_velocities() const { return v; }
};

} // namespace unitype