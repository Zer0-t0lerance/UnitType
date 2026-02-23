/**
 * @file blackhole_sim.h
 * @brief Модуль физической модели: инициализация и симуляция.
 * Ничего не знает про Python. Работает только с сырыми указателями и ядром UniType.
 */
#pragma once
#include "SystemState.h"
#include "History.h"
#include <cmath>

namespace unitype {

    /**
     * @brief Вспомогательная функция настройки орбитальной механики (Первая космическая).
     */
    inline void init_circular_orbits(SystemState<double, 3>& system, const double* initial_coords, double central_mass) {
        for (size_t i = 0; i < system.size(); ++i) {
            system[i].coords().x() = initial_coords[i * 3 + 0];
            system[i].coords().y() = initial_coords[i * 3 + 1];
            system[i].coords().z() = initial_coords[i * 3 + 2];
            
            // Физика: расчет вектора скорости
            double R = system[i].coords().length();
            double v_orb = std::sqrt(central_mass / R);
            
            auto dir = system[i].coords().normalized();
            system[i].velocity().x() = -dir.y() * v_orb;
            system[i].velocity().y() =  dir.x() * v_orb;
            system[i].velocity().z() = 0.0;
        }
    }

    /**
     * @brief Главный цикл интеграции (Решатель).
     */
    inline HistoryContainer<double, 3>* run_blackhole_sim(const double* initial_coords, size_t num_objects, size_t steps, double dt) {
        // 1. Инициализация DOD-контейнера
        SystemState<double, 3> system(num_objects);
        init_circular_orbits(system, initial_coords, 1000.0); // Центральная масса = 1000

        // 2. Выделение памяти под Архив
        auto* archive = new HistoryContainer<double, 3>(num_objects, steps);
        
        // 3. Вычисление физики
        double time = 0.0;
        for (size_t s = 0; s < steps; ++s) {
            for (size_t i = 0; i < system.size(); ++i) {
                auto& r = system[i].coords();
                auto& v = system[i].velocity();
                
                double dist_sq = r.norm_sq() + 0.01; 
                double force = 1000.0 / dist_sq;
                
                auto dir = r.normalized();
                v = v - (dir * (force * dt)); 
                r = r + (v * dt);             
            }
            archive->push_step(time, system.get_coords());
            time += dt;
        }
        
        return archive;
    }

} // namespace unitype