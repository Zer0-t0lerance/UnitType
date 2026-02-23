/**
 * @file python_bindings.cpp
 * @brief Модуль связи (API): Трансляция типов между Python (NumPy) и C++.
 * Не содержит никакой бизнес-логики или физики.
 */
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include "blackhole_sim.h" // Подключаем слой Модели

namespace py = pybind11;
using namespace unitype;

py::array_t<double> py_simulate(py::array_t<double> initial_coords, size_t steps, double dt) {
    
    // 1. Трансляция входных данных (NumPy -> сырой указатель C++)
    py::buffer_info buf = initial_coords.request();
    if (buf.ndim != 2 || buf.shape[1] != 3) {
        throw std::runtime_error("Ожидается массив размерностью (N, 3)!");
    }
    
    size_t num_objects = buf.shape[0];
    const double* ptr = static_cast<const double*>(buf.ptr);

    // 2. Делегирование работы Модели (C++ Engine)
    auto* archive = run_blackhole_sim(ptr, num_objects, steps, dt);

    // 3. Трансляция выходных данных (C++ Архив -> NumPy Zero-copy)
    py::capsule free_when_done(archive, [](void *f) {
        delete reinterpret_cast<HistoryContainer<double, 3>*>(f);
    });

    return py::array_t<double>(
        {steps, num_objects, (size_t)3}, 
        {num_objects * 3 * sizeof(double), 3 * sizeof(double), sizeof(double)}, 
        archive->get_raw_data(), 
        free_when_done 
    );
}

// Регистрация интерфейса для Python
PYBIND11_MODULE(unitype_sim, m) {
    m.doc() = "UniType: Python <-> C++ DOD Bridge";
    m.def("simulate_black_hole", &py_simulate, "Запуск симуляции (Zero-copy)");
}