# Скрипт для сборки C++ ядра в виде Python-модуля (pybind11)

Write-Host "=== Начинаем сборку unitype_sim.pyd ===" -ForegroundColor Yellow

# 1. Получаем пути
$includes = python -m pybind11 --includes
$pylib_dir = python -c "import sys; print(sys.base_prefix + '\\libs')"
$py_lib = python -c "import sys; print(f'python{sys.version_info.major}{sys.version_info.minor}')"

# 2. Формируем команду компилятора
$cmd = "g++ -O3 -Wall -shared -std=c++17 $includes src/python_bindings.cpp -o unitype_sim.pyd -L`"$pylib_dir`" -l$py_lib -L C:/msys64/ucrt64/lib -ltbb12"

# 3. Выполняем
Write-Host "Выполняю команду: " -NoNewline
Write-Host $cmd -ForegroundColor Cyan
Invoke-Expression $cmd

Write-Host "=== Сборка успешно завершена! Модуль готов к импорту в Python. ===" -ForegroundColor Green