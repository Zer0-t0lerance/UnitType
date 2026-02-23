#include <iostream>
#include <chrono>
#include <iomanip>
#include <cassert>
#include <cmath>

#include "../src/UnitType.h"
#include "../src/SystemState.h"
#include "../src/History.h"

using namespace unitype;

bool is_close(double a, double b) {
    return std::abs(a - b) < 1e-7;
}

int main() {
    #ifdef _WIN32
    system("chcp 65001 > nul");
    #endif

    const size_t NUM_OBJECTS = 100'000;
    const size_t NUM_STEPS = 1000;

    std::cout << "=== ЧИСТОВОЙ СТРЕСС-ТЕСТ КОНТЕЙНЕРА ИСТОРИИ ===\n";
    std::cout << "Объектов: " << NUM_OBJECTS << " | Шагов: " << NUM_STEPS << "\n";
    std::cout << "Ожидаемый объем RAM: ~" << (NUM_OBJECTS * NUM_STEPS * 3 * 8) / (1024 * 1024) << " МБ\n\n";

    // Новое название "Молотилки"
    SystemState<double, 3> current_state(NUM_OBJECTS);
    HistoryContainer<double, 3> archive(NUM_OBJECTS, NUM_STEPS);

    for (size_t i = 0; i < current_state.size(); ++i) {
        current_state[i].velocity().x() = 1.0;
        current_state[i].velocity().y() = 2.0;
        current_state[i].velocity().z() = 3.0;
    }

    std::cout << "[ФАЗА 1] Интеграция и быстрая запись (insert)...\n";
    auto start = std::chrono::high_resolution_clock::now();

    double dt = 0.5;
    double time = 0.0;

    for (size_t step = 0; step < NUM_STEPS; ++step) {
        for (size_t i = 0; i < current_state.size(); ++i) {
            current_state[i].coords() = current_state[i].coords() + current_state[i].velocity() * dt;
        }
        
        // Сверхбыстрая запись сырого массива!
        archive.push_step(time, current_state.get_coords());
        time += dt;
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "  Успешно! Время симуляции и записи: " << std::fixed << std::setprecision(4) << diff.count() << " сек\n\n";

    std::cout << "[ФАЗА 2] Аналитика (Zero-copy Stride vs Contiguous Copy)...\n";
    
    auto track42 = archive.get_track(42);
    assert(is_close(track42[100].x(), 50.5 * 1.0));

    auto cache_start = std::chrono::high_resolution_clock::now();
    std::vector<UnitType<double, 3>> cached_track = track42.to_vector();
    auto cache_end = std::chrono::high_resolution_clock::now();
    
    std::cout << "  Успешно! Трек извлечен в локальный кэш за " 
              << std::chrono::duration<double>(cache_end - cache_start).count() << " сек\n\n";

    std::cout << "[ФАЗА 3] Глобальный перенос системы координат...\n";
    start = std::chrono::high_resolution_clock::now();
    UnitType<double, 3> global_shift;
    global_shift.x() = 3.14; global_shift.y() = 2.71; global_shift.z() = 1.61;

    archive.shift_all(global_shift);
    end = std::chrono::high_resolution_clock::now();
    
    std::cout << "  Успешно! Все " << NUM_OBJECTS * NUM_STEPS << " точек сдвинуты.\n";
    std::cout << "  Время глобального многопоточного сдвига: " << std::chrono::duration<double>(end - start).count() << " сек\n";

    return 0;
}