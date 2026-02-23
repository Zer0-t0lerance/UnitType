/**
 * @file python_bindings.cpp
 * @brief Обертка (Wrapper) для связи чистого C++ кода с Python.
 */
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include "SystemState.h"
#include "History.h"
#include "blackhole_sim.h" // Подключаем нашу чистую логику!

namespace py = pybind11;
using namespace unitype;

// Функция-переводчик
py::array_t<double> py_simulate(py::array_t<double> initial_coords, size_t steps, double dt) {
    
    // 1. Читаем NumPy массив из Питона
    py::buffer_info buf = initial_coords.request();
    if (buf.ndim != 2 || buf.shape[1] != 3) {
        throw std::runtime_error("Ожидается массив размерностью (N, 3)!");
    }
    
    size_t num_objects = buf.shape[0];
    SystemState<double, 3> system(num_objects);

    // 2. Инициализируем наш C++ контейнер начальными данными
    // 2. Инициализируем наш C++ контейнер начальными данными
    double* ptr = static_cast<double*>(buf.ptr);
    for (size_t i = 0; i < num_objects; ++i) {
        system[i].coords().x() = ptr[i * 3 + 0];
        system[i].coords().y() = ptr[i * 3 + 1];
        system[i].coords().z() = ptr[i * 3 + 2];
        
        // --- ПРАВИЛЬНАЯ ОРБИТАЛЬНАЯ МЕХАНИКА ---
        double R = system[i].coords().length();
        double v_orb = std::sqrt(1000.0 / R); // Первая космическая (GM = 1000)
        
        auto dir = system[i].coords().normalized();
        // Вектор скорости перпендикулярен радиус-вектору (поворот на 90 градусов)
        system[i].velocity().x() = -dir.y() * v_orb;
        system[i].velocity().y() =  dir.x() * v_orb;
        system[i].velocity().z() = 0.0;
    }

    // 3. ВЫЗЫВАЕМ ЧИСТУЮ ФУНКЦИЮ ИЗ ФАЙЛА 1
    auto* archive = run_blackhole_sim(system, steps, dt);

    // 4. Настраиваем удаление памяти (Сборщик мусора Питона удалит Архив C++)
    py::capsule free_when_done(archive, [](void *f) {
        delete reinterpret_cast<HistoryContainer<double, 3>*>(f);
    });

    // 5. Возвращаем Zero-copy обертку
    return py::array_t<double>(
        {steps, num_objects, (size_t)3}, 
        {num_objects * 3 * sizeof(double), 3 * sizeof(double), sizeof(double)}, 
        archive->get_raw_data(), 
        free_when_done 
    );
}

// Регистрируем модуль
PYBIND11_MODULE(unitype_sim, m) {
    m.def("simulate_black_hole", &py_simulate, "Запуск C++ симуляции с Zero-Copy");
}