#include <iostream>
#include <cassert>
#include "../src/SystemState.h"

using namespace unitype;

int main() {
    
    #ifdef _WIN32
    system("chcp 65001 > nul");
    #endif

    std::cout << "=== ТЕСТ ЧАНКОВОЙ ПАМЯТИ SYSTEM STATE ===\n";

    // Создаем пустую систему
    SystemState<double, 3> state;

    // 1. Создаем первый объект (ID 0)
    size_t id_0 = state.allocate_new();
    
    // Получаем сырой указатель прямо в оперативку
    double* raw_x_ptr = &(state[id_0].coords().x());
    
    // Пишем туда данные
    state[id_0].coords().x() = 42.5;

    std::cout << "Объект 0 создан. Значение X: " << *raw_x_ptr << " по адресу " << raw_x_ptr << "\n";

    // 2. Симулируем подключение MSB! Добавляем 100 000 новых объектов
    // В старой версии с std::vector это бы вызвало реаллокацию памяти и краш.
    std::cout << "Выделяем память еще под 100 000 сущностей...\n";
    state.allocate_up_to(100001);

    // 3. ПРОВЕРКА ИСТИНЫ
    // Если чанки работают правильно, сырой указатель все еще указывает на 42.5
    std::cout << "Проверяем выживаемость указателя: " << *raw_x_ptr << "\n";
    
    assert(*raw_x_ptr == 42.5 && "ФАТАЛЬНО: Указатель инвалидировался!");
    assert(state[id_0].coords().x() == 42.5 && "ФАТАЛЬНО: Линза потеряла данные!");

    std::cout << "УСПЕХ! Память стабильна, реаллокации не повредили старые данные.\n";
    return 0;
}