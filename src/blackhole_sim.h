/**
 * @file blackhole_sim.h
 * @brief Чистая C++ функция симуляции (без привязки к Python).
 */
#pragma once
#include "SystemState.h"
#include "History.h"

namespace unitype {

    /**
     * @brief Тяжелая математическая модель.
     * Вычисляет орбиты и возвращает динамически выделенный Архив.
     */
    inline HistoryContainer<double, 3>* run_blackhole_sim(SystemState<double, 3>& system, size_t steps, double dt) {
        // Создаем архив в куче (heap)
        auto* archive = new HistoryContainer<double, 3>(system.size(), steps);
        
        double time = 0.0;
        for (size_t s = 0; s < steps; ++s) {
            for (size_t i = 0; i < system.size(); ++i) {
                auto& r = system[i].coords();
                auto& v = system[i].velocity();
                
                // Упрощенная гравитация к центру
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