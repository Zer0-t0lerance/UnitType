#include <iostream>
#include <chrono>
#include <cassert>
#include "../src/SystemState.h"
#include "../src/History.h"

using namespace unitype;

int main() {
    #ifdef _WIN32
    system("chcp 65001 > nul");
    #endif

    const size_t NUM_OBJECTS = 100000;
    const size_t NUM_STEPS = 1000;

    std::cout << "=== ЧИСТОВОЙ СТРЕСС-ТЕСТ АБСТРАКТНОГО АРХИВА ===\n";
    std::cout << "Объектов: " << NUM_OBJECTS << " | Шагов: " << NUM_STEPS << "\n";

    // Создаем 3D-пул (N=3)
    SystemState<double, 3> current_state(NUM_OBJECTS);
    HistoryContainer<double, 3> archive(NUM_OBJECTS, NUM_STEPS);

    // [ФАЗА 1] Инициализация
    for (size_t i = 0; i < NUM_OBJECTS; ++i) {
        current_state[i].data()[0] = static_cast<double>(i); // Условный X
        current_state[i].data()[1] = 5.0;                    // Условный Y
        current_state[i].data()[2] = -1.0;                   // Условный Z
    }

    std::cout << "[ФАЗА 1] Интеграция и быстрая запись (memcpy из чанков)...\n";
    auto start = std::chrono::high_resolution_clock::now();
    
    double time = 0.0;
    for (size_t s = 0; s < NUM_STEPS; ++s) {
        // Архив должен вытянуть data() из новых чанков напрямую
        archive.push_step(time, current_state);
        time += 0.1;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "  Успешно! Время записи: " << diff.count() << " сек\n";

    // [ФАЗА 2] Многопоточный сдвиг (проверка TBB)
    std::cout << "[ФАЗА 2] Глобальный перенос системы координат...\n";
    UnitType<double, 3> shift_vec;
    shift_vec[0] = 10.0; shift_vec[1] = 20.0; shift_vec[2] = 30.0;

    start = std::chrono::high_resolution_clock::now();
    archive.shift_all(shift_vec);
    end = std::chrono::high_resolution_clock::now();
    diff = end - start;
    
    std::cout << "  Успешно! Все " << NUM_OBJECTS * NUM_STEPS << " точек сдвинуты.\n";
    std::cout << "  Время глобального многопоточного сдвига: " << diff.count() << " сек\n";

    return 0;
}