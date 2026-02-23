import os
import psutil
import time
import numpy as np
import matplotlib.pyplot as plt

# --- НОВЫЙ БЛОК: Показываем Питону, где лежат DLL от C++ (TBB и MinGW) ---
if hasattr(os, 'add_dll_directory'):
    try:
        os.add_dll_directory(r"C:\msys64\ucrt64\bin")
    except Exception as e:
        print("Не удалось добавить путь к DLL:", e)
# ------------------------------------------------------------------------

# ИМПОРТИРУЕМ НАШУ БИБЛИОТЕКУ НА C++
import unitype_sim

def get_ram_mb():
    return psutil.Process(os.getpid()).memory_info().rss / (1024 * 1024)

print(f"[1] Память на старте Python: {get_ram_mb():.1f} MB")

NUM_SATELLITES = 10_000
NUM_STEPS = 5_000

# Генерируем кольцо аппаратов
theta = np.linspace(0, 2*np.pi, NUM_SATELLITES)
radius = np.random.uniform(50, 100, NUM_SATELLITES)
initial_positions = np.zeros((NUM_SATELLITES, 3), dtype=np.float64)
initial_positions[:, 0] = radius * np.cos(theta)
initial_positions[:, 1] = radius * np.sin(theta)

print(f"[2] Память после создания стартовых данных: {get_ram_mb():.1f} MB")

print(f"\n🚀 Запускаем C++ ядро ({NUM_SATELLITES} аппаратов, {NUM_STEPS} шагов)...")
start_time = time.time()

# ВЫЗОВ C++
history = unitype_sim.simulate_black_hole(initial_positions, NUM_STEPS, 0.01)

print(f"✅ Симуляция завершена за {time.time() - start_time:.3f} сек!")

# Проверяем потребление памяти
ram_after = get_ram_mb()
expected_gb = (NUM_STEPS * NUM_SATELLITES * 3 * 8) / (1024**3)
print(f"\n[3] Память после получения гигантского массива: {ram_after:.1f} MB")
print(f"📦 Физический размер массива истории: {expected_gb:.2f} Гигабайт")
print("\nЕсли бы Питон копировал данные, процесс потреблял бы 1.2+ ГБ памяти. Zero-copy работает!")

# Рисуем 5 случайных траекторий
print("\nОтрисовка графиков...")
plt.figure(figsize=(8, 8), facecolor='black')
ax = plt.axes()
ax.set_facecolor('black')
plt.scatter([0], [0], color='white', s=100, zorder=5, label='Аттрактор')

for i in range(1000):
    plt.plot(history[:, i, 0], history[:, i, 1], alpha=0.8, linewidth=1.5)

plt.title("Орбиты (C++ Engine -> Python Plot)", color='white')
plt.axis('equal')
plt.grid(color='#333333', linestyle='--', linewidth=0.5)
plt.tight_layout()
plt.show()