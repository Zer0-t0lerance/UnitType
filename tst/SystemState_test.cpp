#include <iostream>
#include <chrono>
#include <iomanip>
#include "../src/SystemState.h"

using namespace unitype;

int main() {
    #ifdef _WIN32
    system("chcp 65001 > nul");
    #endif

    size_t N = 100'000;
    // Используем новое универсальное название
    SystemState<double, 3> system_state(N);

    // Дадим всем стартовую скорость
    for (size_t i = 0; i < system_state.size(); ++i) {
        system_state[i].velocity().x() = 1.0;
        system_state[i].velocity().y() = 2.0;
        system_state[i].velocity().z() = 3.0;
    }

    std::cout << "Начинаем интегрирование " << N << " N-мерных объектов...\n";
    
    auto start = std::chrono::high_resolution_clock::now();

    double dt = 0.5;
    for (size_t i = 0; i < system_state.size(); ++i) {
        system_state[i].coords() = system_state[i].coords() + system_state[i].velocity() * dt;
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;

    std::cout << "Интегрирование завершено!\n";
    std::cout << "Время выполнения: " << std::fixed << std::setprecision(6) << diff.count() << " секунд\n";
    std::cout << "Проверка Z последнего объекта: " << system_state[N - 1].coords().z() << "\n";

    return 0;
}