# UniType Engine 🚀

![C++17](https://img.shields.io/badge/C++-17-blue.svg)
![Python](https://img.shields.io/badge/Python-3.8+-yellow.svg)
![License](https://img.shields.io/badge/License-MIT-green.svg)

**UniType** — это высокопроизводительная C++ библиотека для N-мерных симуляций, построенная на принципах **Data-Oriented Design (DOD)** с полной поддержкой бесшовной интеграции в **Python / NumPy** (Zero-copy).

Изначально движок создавался для расчета орбитальной механики (N-тел), но его математическое ядро абсолютно универсально и подходит для любых физических симуляций, вычисления градиентов или задач Data Science.

## ✨ Главные фичи
* **Экстремальная производительность (DOD):** Использование архитектуры SoA (Structure of Arrays) обеспечивает идеальное попадание в кэш процессора L1/L2.
* **Магия Zero-Copy для Python:** Вы можете симулировать миллионы частиц в C++, а затем анализировать и рисовать их в Python (через NumPy и Matplotlib) **без копирования памяти** благодаря Buffer Protocol.
* **Многопоточность из коробки:** Автоматическая SIMD-векторизация и распараллеливание (через Intel TBB `std::execution::par_unseq`) для массивов > 1000 элементов.
* **N-мерность:** Шаблонная архитектура `template <typename T, size_t N>` позволяет решать задачи в пространствах любой размерности.

## 🏗 Архитектура

Проект разбит на три независимых модуля (Header-only):
1. `UnitType.h` — Математическое ядро (векторы, скаляры, тензоры, статистика).
2. `OrbitSystem.h` — Контейнер состояния. Хранит координаты, скорости и ускорения объектов в непрерывных (flat) массивах.
3. `History.h` — Многоканальный архив. Сверхбыстро (через `insert`) пишет историю симуляции и отдает её в виде временных рядов (Tracks) для аналитики.

## 🐍 Интеграция с Python (pybind11)

В проекте есть готовый мост для Python. Чтобы собрать C++ ядро в Python-модуль на Windows (с использованием MinGW GCC):

1. Установите зависимости:
   ```powershell
   pip install pybind11 numpy matplotlib psutil

2. Запустите скрипт сборки в корне проекта:
   ```powershell
   .\build_module.ps1

3. Запустите демо-симуляцию черной дыры:
   ```powershell
   python demo.py

C++ ядро просчитает 50 000 000 состояний за ~1 секунду и мгновенно отдаст 1+ ГБ данных в Python для отрисовки графиков.

🛠 Сборка C++ тестов

Если вы хотите использовать движок чисто на C++, скомпилируйте тесты следующими командами (требуется флаг -ltbb12):
   ```powershell
   g++ -std=c++17 -O3 -I./src tst/UnitType_test.cpp -o tst/test_unit.exe -L C:/msys64/ucrt64/lib -ltbb12 "-Wl,--stack,268435456"
   g++ -std=c++17 -O3 -I./src tst/SystemState_test.cpp -o tst/test_state.exe -L C:/msys64/ucrt64/lib -ltbb12 "-Wl,--stack,268435456"
   g++ -std=c++17 -O3 -I./src tst/History_test.cpp -o tst/test_history.exe -L C:/msys64/ucrt64/lib -ltbb12 "-Wl,--stack,268435456"