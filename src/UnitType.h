#pragma once
#include <iostream>
#include <array>
#include <cmath>
#include <algorithm>
#include <execution> 
#include <numeric>   

/**
 * @namespace unitype
 * @brief Главное пространство имен библиотеки универсальных N-мерных вычислений.
 */
namespace unitype {

constexpr size_t THREADING_THRESHOLD = 1000; 

/**
 * @brief Универсальный контейнер для N-мерного вектора или скаляра.
 * * Поддерживает базовую арифметику, векторную алгебру и статистические функции.
 * При N > THREADING_THRESHOLD автоматически включает многопоточную SIMD векторизацию (Intel TBB).
 * * @tparam T Базовый тип данных (double, int, std::complex, кастомные типы).
 * @tparam N Размерность пространства (N = 1 для скаляров).
 */
template <typename T, size_t N>
struct UnitType {
    /** @brief Внутренний массив данных. Инициализируется нулем по умолчанию. */
    std::array<T, N> data = {}; 

    // ==========================================
    // ДОСТУП К ЭЛЕМЕНТАМ
    // ==========================================
    
    /** @brief Доступ к элементу по индексу (0-based). */
    T& operator[](size_t i) { return data[i]; }
    const T& operator[](size_t i) const { return data[i]; }

    /** @brief Доступ к элементу по математическому индексу (1-based, как в Фортране/Матлабе). */
    T& operator()(size_t i) { return data[i - 1]; }
    const T& operator()(size_t i) const { return data[i - 1]; }

    /** @brief Быстрый доступ к первой координате (X). Доступен только при N >= 1. */
    T& x() { static_assert(N >= 1, "Размерность меньше 1!"); return data[0]; }
    const T& x() const { static_assert(N >= 1, "Размерность меньше 1!"); return data[0]; }

    /** @brief Быстрый доступ ко второй координате (Y). Доступен только при N >= 2. */
    T& y() { static_assert(N >= 2, "Размерность меньше 2!"); return data[1]; }
    const T& y() const { static_assert(N >= 2, "Размерность меньше 2!"); return data[1]; }

    /** @brief Быстрый доступ к третьей координате (Z). Доступен только при N >= 3. */
    T& z() { static_assert(N >= 3, "Размерность меньше 3!"); return data[2]; }
    const T& z() const { static_assert(N >= 3, "Размерность меньше 3!"); return data[2]; }

    // ==========================================
    // УНИВЕРСАЛЬНЫЕ ФУНКЦИИ APPLY
    // ==========================================
    
    /**
     * @brief Применяет бинарную операцию к двум объектам UnitType.
     * @tparam Func Тип лямбда-выражения или функции.
     * @param other Второй объект для операции.
     * @param op Функция вида `T op(const T& a, const T& b)`.
     * @return UnitType<T, N> Результат покомпонентной операции.
     */
    template <typename Func>
    UnitType<T, N> apply_binary(const UnitType<T, N>& other, Func op) const {
        UnitType<T, N> res;
        if constexpr (N > THREADING_THRESHOLD) {
            std::transform(std::execution::par_unseq, 
                           data.begin(), data.end(), 
                           other.data.begin(), res.data.begin(), op);
        } else {
            for (size_t i = 0; i < N; ++i) res[i] = op(data[i], other.data[i]);
        }
        return res;
    }

    /**
     * @brief Применяет унарную операцию к объекту UnitType.
     */
    template <typename Func>
    UnitType<T, N> apply_unary(Func op) const {
        UnitType<T, N> res;
        if constexpr (N > THREADING_THRESHOLD) {
            std::transform(std::execution::par_unseq, 
                           data.begin(), data.end(), res.data.begin(), op);
        } else {
            for (size_t i = 0; i < N; ++i) res[i] = op(data[i]);
        }
        return res;
    }

    // ==========================================
    // КОНСТРУКТОРЫ
    // ==========================================
    UnitType() = default; 

    UnitType(const UnitType<T, N>& other) {
        if constexpr (N > THREADING_THRESHOLD) {
            std::copy(std::execution::par_unseq, other.data.begin(), other.data.end(), data.begin());
        } else {
            std::copy(other.data.begin(), other.data.end(), data.begin());
        }
    }

    UnitType<T, N>& operator=(const UnitType<T, N>& other) {
        if (this != &other) { 
            if constexpr (N > THREADING_THRESHOLD) {
                std::copy(std::execution::par_unseq, other.data.begin(), other.data.end(), data.begin());
            } else {
                std::copy(other.data.begin(), other.data.end(), data.begin());
            }
        }
        return *this;
    }

    // ==========================================
    // БАЗОВАЯ АРИФМЕТИКА
    // ==========================================
    UnitType<T, N> operator-() const { return apply_unary([](const T& a) { return -a; }); }
    UnitType<T, N> operator+(const UnitType<T, N>& other) const { return apply_binary(other, [](const T& a, const T& b) { return a + b; }); }
    UnitType<T, N> operator+(const T& scalar) const { return apply_unary([&scalar](const T& a) { return a + scalar; }); }
    UnitType<T, N> operator-(const UnitType<T, N>& other) const { return (*this) + (-other); }
    UnitType<T, N> operator-(const T& scalar) const { return (*this) + (-scalar); }
    UnitType<T, N> operator*(const T& scalar) const { return apply_unary([&scalar](const T& a) { return a * scalar; }); }
    UnitType<T, N> operator/(const T& scalar) const { return apply_unary([&scalar](const T& a) { return a / scalar; }); }

    // ==========================================
    // ВЕКТОРНАЯ АЛГЕБРА
    // ==========================================
    
    /** @brief Вычисляет скалярное произведение (Dot product) двух векторов. */
    T dot(const UnitType<T, N>& other) const {
        if constexpr (N > THREADING_THRESHOLD) {
            return std::transform_reduce(std::execution::par_unseq, data.begin(), data.end(), other.data.begin(), T{0});
        } else {
            T sum = T{0};
            for (size_t i = 0; i < N; ++i) sum += data[i] * other.data[i];
            return sum;
        }
    }

    /** * @brief Вычисляет векторное произведение (Cross product). 
     * @note Строго определено только для 3D пространства (N=3).
     */
    UnitType<T, N> cross(const UnitType<T, N>& other) const {
        static_assert(N == 3, "Векторное произведение определено только для 3D!");
        UnitType<T, 3> res;
        res[0] = data[1] * other.data[2] - data[2] * other.data[1];
        res[1] = data[2] * other.data[0] - data[0] * other.data[2];
        res[2] = data[0] * other.data[1] - data[1] * other.data[0];
        return res;
    }

    /** @brief Возвращает нормализованный вектор (длиной 1). */
    UnitType<T, N> normalized() const {
        T len = length();
        if (len == T{0}) return *this; 
        return (*this) / len;
    }

    /** @brief Вычисляет угол между двумя векторами (в радианах). */
    T angle(const UnitType<T, N>& other) const {
        T lenSq1 = this->norm_sq();
        T lenSq2 = other.norm_sq();
        if (lenSq1 == T{0} || lenSq2 == T{0}) return T{0};

        using std::sqrt; 
        T cos_theta = this->dot(other) / sqrt(lenSq1 * lenSq2);
        
        using std::clamp; 
        using std::acos;
        cos_theta = clamp(cos_theta, T{-1}, T{1}); 
        return acos(cos_theta);
    }

    // ==========================================
    // АГРЕГАТНЫЕ ФУНКЦИИ И СТАТИСТИКА
    // ==========================================
    T sum() const {
        if constexpr (N > THREADING_THRESHOLD) return std::reduce(std::execution::par_unseq, data.begin(), data.end(), T{0});
        else { T s = T{0}; for (const T& v : data) s += v; return s; }
    }

    T norm_sq() const {
        if constexpr (N > THREADING_THRESHOLD) return std::transform_reduce(std::execution::par_unseq, data.begin(), data.end(), T{0}, std::plus<>(), [](const T& a){ return a * a; });
        else { T s = T{0}; for (const T& v : data) s += v * v; return s; }
    }

    T length() const { using std::sqrt; return sqrt(norm_sq()); }
    T mean() const { return sum() / static_cast<T>(N); }

    T max() const {
        if constexpr (N > THREADING_THRESHOLD) return *std::max_element(std::execution::par_unseq, data.begin(), data.end());
        else return *std::max_element(data.begin(), data.end());
    }

    T min() const {
        if constexpr (N > THREADING_THRESHOLD) return *std::min_element(std::execution::par_unseq, data.begin(), data.end());
        else return *std::min_element(data.begin(), data.end());
    }

    T max_abs() const {
        using std::abs;
        auto cmp = [](const T& a, const T& b) { return abs(a) < abs(b); };
        if constexpr (N > THREADING_THRESHOLD) return abs(*std::max_element(std::execution::par_unseq, data.begin(), data.end(), cmp));
        else return abs(*std::max_element(data.begin(), data.end(), cmp));
    }

    T min_abs() const {
        using std::abs;
        auto cmp = [](const T& a, const T& b) { return abs(a) < abs(b); };
        if constexpr (N > THREADING_THRESHOLD) return abs(*std::min_element(std::execution::par_unseq, data.begin(), data.end(), cmp));
        else return abs(*std::min_element(data.begin(), data.end(), cmp));
    }

    T mean_abs() const {
        using std::abs;
        T s_abs = T{0};
        if constexpr (N > THREADING_THRESHOLD) {
            s_abs = std::transform_reduce(std::execution::par_unseq, data.begin(), data.end(), T{0}, std::plus<>(), [](const T& a){ return abs(a); });
        } else {
            for (const T& v : data) s_abs += abs(v);
        }
        return s_abs / static_cast<T>(N);
    }
};

// Глобальные операторы (Коммутативность скаляра)
template <typename T, size_t N>
UnitType<T, N> operator+(const T& scalar, const UnitType<T, N>& v) { return v + scalar; }

template <typename T, size_t N>
UnitType<T, N> operator-(const T& scalar, const UnitType<T, N>& v) { return (-v) + scalar; }

template <typename T, size_t N>
UnitType<T, N> operator*(const T& scalar, const UnitType<T, N>& v) { return v * scalar; }

} // namespace unitype