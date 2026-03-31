#pragma once
#include <vector>
#include <cassert>
#include <algorithm>
#include <execution>
#include <cstring>
#include "UnitType.h"
#include "SystemState.h" // Добавили зависимость от SystemState

namespace unitype {

template <typename T, size_t N>
class HistoryContainer;

/**
 * @brief Линза-Траектория (Временной ряд) для одного объекта в Архиве.
 * * Позволяет анализировать историю изменения параметров одного объекта без 
 * копирования данных из глобального Архива. Работает через итерацию с шагом (stride).
 */
template <typename T, size_t N>
class TrackProxy {
private:
    HistoryContainer<T, N>* archive; 
    size_t obj_id;                   
    size_t stride;                   

public:
    TrackProxy(HistoryContainer<T, N>* hist, size_t id, size_t total_objects) 
        : archive(hist), obj_id(id), stride(total_objects) {}

    /** @brief Возвращает количество записанных шагов времени. */
    size_t steps_count() const;

    /** @brief Доступ к состоянию объекта на конкретном шаге времени. */
    UnitType<T, N>& operator[](size_t step_index);
    const UnitType<T, N>& operator[](size_t step_index) const;

    /** @brief Многопоточный сдвиг всей траектории объекта на заданный вектор. */
    void shift(const UnitType<T, N>& shift_vec);
    
    /** @brief Находит максимальное отклонение объекта (по модулю) за всю историю. */
    T max_abs() const;

    /** * @brief Экспорт траектории в непрерывный локальный массив.
     * Полезно для тяжелой математической аналитики, чтобы избежать промахов кэша.
     */
    std::vector<UnitType<T, N>> to_vector() const {
        std::vector<UnitType<T, N>> res;
        res.reserve(steps_count());
        for (size_t step = 0; step < steps_count(); ++step) {
            res.push_back((*this)[step]);
        }
        return res;
    }
};

/**
 * @brief Глобальный Архив (Многоканальный магнитофон) для хранения истории системы.
 * * Хранит историю изменений всех объектов в едином непрерывном массиве (Flat Memory).
 * Обеспечивает экстремально быструю запись (memmove) и поддержку многопоточной векторизации.
 */
template <typename T, size_t N>
class HistoryContainer {
private:
    friend class TrackProxy<T, N>; 

    size_t count;               
    std::vector<double> times;  
    std::vector<UnitType<T, N>> data; 

public:
    /**
     * @brief Конструктор Архива.
     * @param num_objects Количество отслеживаемых объектов в системе.
     * @param expected_steps Ожидаемое количество шагов для пред-аллокации памяти (защита от реаллокаций).
     */
    explicit HistoryContainer(size_t num_objects, size_t expected_steps = 1000) : count(num_objects) {
        times.reserve(expected_steps); 
        data.reserve(expected_steps * num_objects);
    }

    size_t objects_count() const { return count; }
    size_t steps_count() const { return times.size(); }

    /** @brief Создает Линзу-Траекторию для аналитики конкретного объекта. */
    TrackProxy<T, N> get_track(size_t object_id) {
        assert(object_id < count && "Такого объекта нет в архиве!");
        return TrackProxy<T, N>(this, object_id, count);
    }

    /** * @brief Сверхбыстрая запись полного кадра напрямую из Чанков SystemState.
     * Избегает создания временных векторов. Использует memcpy для максимальной скорости.
     */
    void push_step(double t, const SystemState<T, N>& current_state) {
        assert(current_state.size() == count && "Размер данных не совпадает с настройками Архива!");
        times.push_back(t); 

        // Выделяем место в конце архива (быстро, так как мы сделали reserve в конструкторе)
        size_t old_size = data.size();
        data.resize(old_size + count);

        // Быстро копируем данные из каждого чанка напрямую в плоскую память Архива
        double* dest_ptr = reinterpret_cast<double*>(data.data() + old_size);
        size_t offset = 0;

        // Внимание: мы обращаемся к приватным чанкам SystemState, 
        // поэтому HistoryContainer нужно сделать friend классом в SystemState.h
        for (const auto& chunk : current_state.chunks) {
            size_t elements_to_copy = std::min(CHUNK_SIZE, count - offset);
            std::memcpy(dest_ptr + offset * N, 
                        chunk->r.data(), 
                        elements_to_copy * sizeof(UnitType<T, N>));
            offset += elements_to_copy;
        }
    }

    /** @brief Глобальный многопоточный сдвиг всех сохраненных данных на вектор. */
    void shift_all(const UnitType<T, N>& shift_vec) {
        std::transform(std::execution::par_unseq, data.begin(), data.end(), data.begin(),
                       [&shift_vec](const UnitType<T, N>& p) { return p + shift_vec; });
    }

    /**
     * @brief Возвращает сырой указатель на внутренний непрерывный массив данных.
     * * @note Этот метод нарушает строгую инкапсуляцию Архива. Он предназначен ИСКЛЮЧИТЕЛЬНО 
     * для низкоуровневой интеграции с внешними C-API (например, для передачи данных 
     * в Python/NumPy через pybind11 по стандарту Buffer Protocol без копирования памяти).
     * Не используйте его для стандартной аналитики внутри C++!
     * * @return const T* Константный указатель на первый байт "ленты" памяти.
     */
    const T* get_raw_data() const {
        return reinterpret_cast<const T*>(data.data());
    }
};

// =================================================================
// РЕАЛИЗАЦИЯ МЕТОДОВ TrackProxy
// =================================================================

template <typename T, size_t N>
size_t TrackProxy<T, N>::steps_count() const { return archive->steps_count(); }

template <typename T, size_t N>
UnitType<T, N>& TrackProxy<T, N>::operator[](size_t step_index) {
    size_t real_index = step_index * stride + obj_id;
    return archive->data[real_index];
}

template <typename T, size_t N>
const UnitType<T, N>& TrackProxy<T, N>::operator[](size_t step_index) const {
    size_t real_index = step_index * stride + obj_id;
    return archive->data[real_index];
}

template <typename T, size_t N>
void TrackProxy<T, N>::shift(const UnitType<T, N>& shift_vec) {
    for (size_t step = 0; step < steps_count(); ++step) {
        (*this)[step] = (*this)[step] + shift_vec;
    }
}

template <typename T, size_t N>
T TrackProxy<T, N>::max_abs() const {
    T current_max = T{0};
    for (size_t step = 0; step < steps_count(); ++step) {
        T val = (*this)[step].max_abs();
        if (val > current_max) current_max = val;
    }
    return current_max;
}

} // namespace unitype