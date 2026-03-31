#include <iostream>
#include <cassert>
#include "../src/SystemState.h"

using namespace unitype;

int main() {
    #ifdef _WIN32
    system("chcp 65001 > nul");
    #endif

    std::cout << "=== ТЕСТ АБСТРАКТНОЙ N-МЕРНОЙ ПАМЯТИ ===\n";

    // Создаем пул на 4 параметра (например: u, v, Re, Im)
    SystemState<double, 4> pool;
    
    // Создаем первую сущность
    size_t id_0 = pool.allocate_new();

    // Пишем данные через новую Линзу (.data()[индекс])
    pool[id_0].data()[0] = 150.5; // u
    pool[id_0].data()[3] = 0.99;  // Im

    std::cout << "Точка 0 создана. Значение u: " << pool[id_0].data()[0] << "\n";

    // Симулируем добавление 10 000 телескопов / точек
    pool.allocate_up_to(10000);

    // Проверяем выживаемость
    assert(pool[id_0].data()[0] == 150.5 && "Память сломалась!");
    assert(pool[id_0].data()[3] == 0.99 && "Память сломалась!");

    std::cout << "УСПЕХ! Чанки работают с любой размерностью без привязки к физике.\n";
    return 0;
}