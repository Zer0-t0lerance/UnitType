import os
import time
import numpy as np

# Лечим DLL Hell
msys_bin_path = r"C:\msys64\ucrt64\bin"
if hasattr(os, 'add_dll_directory') and os.path.exists(msys_bin_path):
    os.add_dll_directory(msys_bin_path)

import unitype_sim

N_OBJS = 100000
N_STEPS = 1000

print(f"=== БЕНЧМАРК: {N_OBJS} объектов, {N_STEPS} шагов ===")
print(f"Ожидаемый объем истории: ~{N_OBJS * N_STEPS * 3 * 8 / 1024 / 1024:.1f} МБ\n")

# ---------------------------------------------------------
# ТЕСТ 1: Чистый NumPy
# ---------------------------------------------------------
print("[1] Запуск NumPy...")
start_np = time.time()

# Выделяем память под текущее состояние и под всю историю
state_np = np.zeros((N_OBJS, 3))
history_np = np.zeros((N_STEPS, N_OBJS, 3))

for step in range(N_STEPS):
    # Копируем кадр в историю (встроенный Си-код NumPy)
    history_np[step] = state_np

time_np = time.time() - start_np
print(f"  Время NumPy: {time_np:.3f} сек")

# ---------------------------------------------------------
# ТЕСТ 2: UniType (С++ Чанки + Zero-copy)
# ---------------------------------------------------------
print("\n[2] Запуск UniType C++...")
start_ut = time.time()

# Создаем пул и архив
pool = unitype_sim.SystemState3D(N_OBJS)
pool.allocate_up_to(N_OBJS)
archive = unitype_sim.HistoryContainer3D(N_OBJS, N_STEPS)

for step in range(N_STEPS):
    # С++ делает прямой memcpy из чанков в плоскую память Архива
    archive.push_step(step * 0.1, pool)

# Мгновенно оборачиваем готовую память в NumPy без копирования
history_ut = np.array(archive.get_raw_data(), copy=False)

time_ut = time.time() - start_ut
print(f"  Время UniType: {time_ut:.3f} сек")

# ---------------------------------------------------------
# ИТОГИ
# ---------------------------------------------------------
print("\n=== РЕЗУЛЬТАТЫ ===")
if time_ut < time_np:
    print(f"UniType быстрее NumPy в {time_np / time_ut:.2f} раз(а)!")
else:
    print(f"NumPy быстрее UniType в {time_ut / time_np:.2f} раз(а)!")