#include <iostream>
#include <complex>
#include <string>
#include <cmath>
#include <iomanip>
#include <cassert>

#include "../src/UnitType.h"

using namespace unitype;

// =================================================================
// 1. ПОДГОТОВКА КАСТОМНОГО ТИПА (Для проверки шаблонов и ADL)
// =================================================================
struct MyFloat {
    double v;
    MyFloat() : v(0) {}
    MyFloat(double val) : v(val) {}
    
    MyFloat operator+(const MyFloat& o) const { return MyFloat(v + o.v); }
    MyFloat operator-(const MyFloat& o) const { return MyFloat(v - o.v); }
    MyFloat operator-() const { return MyFloat(-v); }
    MyFloat operator*(const MyFloat& o) const { return MyFloat(v * o.v); }
    MyFloat operator/(const MyFloat& o) const { return MyFloat(v / o.v); }
    MyFloat& operator+=(const MyFloat& o) { v += o.v; return *this; }
    
    bool operator<(const MyFloat& o) const { return v < o.v; }
};

MyFloat abs(const MyFloat& m) { return MyFloat(std::abs(m.v)); }
MyFloat sqrt(const MyFloat& m) { return MyFloat(std::sqrt(m.v)); }
MyFloat acos(const MyFloat& m) { return MyFloat(std::acos(m.v)); }

// =================================================================
// 2. ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ
// =================================================================
std::ostream& operator<<(std::ostream& os, const MyFloat& m) { 
    return os << m.v << "_custom"; 
}

template <typename T, size_t N>
std::ostream& operator<<(std::ostream& os, const UnitType<T, N>& vec) {
    os << "[";
    for (size_t i = 0; i < N; ++i) {
        os << vec[i];
        if (i < N - 1) os << ", ";
    }
    os << "]";
    return os;
}

template <typename T>
bool is_close(const T& a, const T& b) {
    if constexpr (std::is_same_v<T, int>) {
        return a == b;
    } else if constexpr (std::is_same_v<T, std::complex<double>>) {
        return std::abs(a - b) < 1e-7;
    } else if constexpr (std::is_same_v<T, MyFloat>) {
        return std::abs(a.v - b.v) < 1e-7;
    } else {
        return std::abs(a - b) < 1e-7;
    }
}

template <typename T, size_t N>
bool is_close(const UnitType<T, N>& a, const UnitType<T, N>& b) {
    for (size_t i = 0; i < N; ++i) {
        if (!is_close(a[i], b[i])) return false;
    }
    return true;
}

template <typename T1, typename T2>
void check(const std::string& op_name, const T1& expected, const T2& result) {
    std::cout << "  [Операция] " << op_name << "\n"
              << "    Ожидается: " << expected << "\n"
              << "    Результат: " << result << "\n"
              << "    Статус:    " << (is_close(expected, result) ? "[СОВПАДАЕТ]" : "[!!! ОШИБКА !!!]") 
              << "\n\n";
}

// =================================================================
// 3. ТЕСТЫ
// =================================================================
void test_int() {
    std::cout << "=== ТЕСТ 1: int (Целые числа), Размерность: 3 ===\n";
    UnitType<int, 3> v1; v1.x() = 10; v1.y() = 20; v1.z() = 30;
    UnitType<int, 3> v2; v2.x() = 2;  v2.y() = 4;  v2.z() = 6;
    UnitType<int, 3> exp_add; exp_add.x()=12; exp_add.y()=24; exp_add.z()=36;
    check("Сложение (v1 + v2)", exp_add, v1 + v2);
}

void test_double() {
    std::cout << "=== ТЕСТ 2: double (Дробные числа), Размерность: 3 ===\n";
    UnitType<double, 3> v1; v1.x() = 1.0; v1.y() = 2.0; v1.z() = 3.0;
    UnitType<double, 3> v2; v2.x() = 4.0; v2.y() = 5.0; v2.z() = 6.0;
    UnitType<double, 3> exp_add; exp_add.x()=5.0; exp_add.y()=7.0; exp_add.z()=9.0;
    check("Сложение (v1 + v2)", exp_add, v1 + v2);
    check("Длина вектора (v1.length())", std::sqrt(14.0), v1.length());
}

void test_complex() {
    std::cout << "=== ТЕСТ 3: std::complex (Комплексные числа), Раз-ть: 2 ===\n";
    using C = std::complex<double>;
    UnitType<C, 2> v1; v1(1) = C(1.0, 2.0); v1(2) = C(3.0, 4.0);
    UnitType<C, 2> v2; v2(1) = C(5.0, 6.0); v2(2) = C(7.0, 8.0);
    check("Скалярное произведение (v1.dot(v2))", C(-18.0, 68.0), v1.dot(v2));
}

void test_custom() {
    std::cout << "=== ТЕСТ 4: Кастомный тип MyFloat, Размерность: 3 ===\n";
    UnitType<MyFloat, 3> v1; v1.x() = MyFloat(1.5); v1.y() = MyFloat(2.5); v1.z() = MyFloat(3.5);
    check("Длина вектора через кастомный ADL sqrt", MyFloat(std::sqrt(1.5*1.5 + 2.5*2.5 + 3.5*3.5)), v1.length());
}

int main() {
    #ifdef _WIN32
    system("chcp 65001 > nul");
    #endif
    std::cout << "\nНАЧАЛО ВСЕОБЪЕМЛЮЩЕГО ТЕСТИРОВАНИЯ unitype::UnitType\n\n";
    try {
        test_int(); test_double(); test_complex(); test_custom();
    } catch (const std::exception& e) {
        std::cerr << "Случилась ошибка: " << e.what() << '\n';
    }
    std::cout << "ТЕСТИРОВАНИЕ ЗАВЕРШЕНО.\n";
    return 0;
}