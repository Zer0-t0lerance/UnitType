import os

# 1. МАГИЯ ПРОТИВ DLL HELL: Явно указываем Питону, где лежат наши плюсовые библиотеки (TBB и GCC)
msys_bin_path = r"D:\msys64\ucrt64\bin"
if hasattr(os, 'add_dll_directory') and os.path.exists(msys_bin_path):
    os.add_dll_directory(msys_bin_path)

# Теперь импорт пройдет как по маслу!
import unitype_sim
import numpy as np

print("=== ТЕСТ N-МЕРНОГО PYTHON-МОСТА ===")

# 1. Создаем 4D-пул (идеально для u, v, Re, Im)

print("\n[1] Создаем пустой пул SystemState4D...")
pool = unitype_sim.SystemState4D(0) # Стартуем с нуля!

# 2. Регистрируем новую точку (выдаст ID 0)
id_0 = pool.allocate_new() # Теперь размер пула ровно 1
print(f"Размер пула: {pool.size()}")

# 3. Получаем Линзу (Proxy)
proxy = pool.get_proxy(id_0)

# 4. Пишем данные напрямую в С++ Чанк
proxy.data[0] = 150.5  # u
proxy.data[1] = -42.0  # v
proxy.data[2] = 1.0    # Re
proxy.data[3] = 0.99   # Im

print(f"\n[2] Записали данные в С++ через Линзу: {proxy.data[0]}, {proxy.data[1]}, {proxy.data[2]}, {proxy.data[3]}")

# 5. Проверим Архиватор
print("\n[3] Записываем шаг в Архив и вытягиваем NumPy массив (Zero-copy)...")
# Архив ждет 1 объект, и в пуле у нас ровно 1 объект. Идеальное совпадение!
archive = unitype_sim.HistoryContainer4D(1, 5) 
archive.push_step(0.0, pool)

# Получаем NumPy массив напрямую из памяти Архива
history_array = np.array(archive.get_raw_data(), copy=False)

print("Форма массива (шаги, объекты, N):", history_array.shape)
print("Содержимое архива (Шаг 0, Объект 0):", history_array[0, 0])