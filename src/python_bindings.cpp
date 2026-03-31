/**
 * @file python_bindings.cpp
 * @brief Модуль связи (API): Экспорт N-мерного абстрактного ядра в Python.
 */
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <string>
#include "SystemState.h"
#include "History.h"

namespace py = pybind11;
using namespace unitype;

// Шаблонная функция для автоматической регистрации классов любой размерности N
template <size_t N>
void declare_unitype_system(py::module& m, const std::string& suffix) {
    std::string unit_name = "UnitType" + suffix;
    std::string proxy_name = "StateProxy" + suffix;
    std::string state_name = "SystemState" + suffix;
    std::string history_name = "HistoryContainer" + suffix;

    // 1. Экспорт UnitType (учим Питон понимать индексы vec[i])
    py::class_<UnitType<double, N>>(m, unit_name.c_str())
        .def(py::init<>())
        .def("__getitem__", [](const UnitType<double, N>& self, size_t i) {
            if (i >= N) throw py::index_error("Индекс выходит за пределы размерности!");
            return self[i];
        })
        .def("__setitem__", [](UnitType<double, N>& self, size_t i, double val) {
            if (i >= N) throw py::index_error("Индекс выходит за пределы размерности!");
            self[i] = val;
        });

    // 2. Экспорт Линзы (StateProxy)
    py::class_<StateProxy<double, N>>(m, proxy_name.c_str())
        // КРИТИЧЕСКИ ВАЖНО: return_value_policy::reference_internal
        // Это гарантирует, что Питон не скопирует данные, а получит ссылку на C++ память!
        .def_property_readonly("data", [](StateProxy<double, N>& self) -> UnitType<double, N>& {
            return self.data();
        }, py::return_value_policy::reference_internal);

    // 3. Экспорт Глобального Пула (SystemState)
    py::class_<SystemState<double, N>>(m, state_name.c_str())
        .def(py::init<size_t>(), py::arg("initial_size") = 0)
        .def("size", &SystemState<double, N>::size)
        .def("allocate_new", &SystemState<double, N>::allocate_new)
        .def("allocate_up_to", &SystemState<double, N>::allocate_up_to)
        .def("get_proxy", [](SystemState<double, N>& self, size_t index) {
            return self[index];
        }, py::return_value_policy::reference_internal); // Линза ссылается на пул
        
    // 4. Экспорт Архиватора (HistoryContainer)
    py::class_<HistoryContainer<double, N>>(m, history_name.c_str())
        .def(py::init<size_t, size_t>(), py::arg("num_objects"), py::arg("expected_steps") = 1000)
        .def("push_step", &HistoryContainer<double, N>::push_step)
        // Zero-copy экспорт истории в NumPy!
        .def("get_raw_data", [](HistoryContainer<double, N>& self) {
            size_t steps = self.steps_count();
            size_t objs = self.objects_count();
            return py::array_t<double>(
                {steps, objs, N}, 
                {objs * N * sizeof(double), N * sizeof(double), sizeof(double)}, 
                self.get_raw_data(), 
                py::cast(&self) // Привязываем время жизни NumPy-массива к Архиватору
            );
        });
}

// Регистрация интерфейса для Python
PYBIND11_MODULE(unitype_sim, m) {
    m.doc() = "UniType: Abstract N-Dimensional DOD Engine";
    
    // Генерируем классы для нужных нам размерностей!
    declare_unitype_system<1>(m, "1D"); // Скаляры (например, масса или радиус)
    declare_unitype_system<3>(m, "3D"); // Классические 3D координаты
    declare_unitype_system<4>(m, "4D"); // Для UV-данных (u, v, Re, Im) или 3D+время
    declare_unitype_system<6>(m, "6D"); // Для полной фазовой физики (x, y, z, vx, vy, vz)
}