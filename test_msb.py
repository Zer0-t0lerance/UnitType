import os
msys_bin_path = r"C:\msys64\ucrt64\bin"
if hasattr(os, 'add_dll_directory') and os.path.exists(msys_bin_path):
    os.add_dll_directory(msys_bin_path)

import unitype_sim
from unitype_msb_bridge import CppIndexProperty, CppBackedEntity

# Создаем глобальный 4D-пул
global_uv_pool = unitype_sim.SystemState4D(100)

class RadioTelescopeUV(CppBackedEntity):
    u = CppIndexProperty(0)
    v = CppIndexProperty(1)
    Re = CppIndexProperty(2)
    Im = CppIndexProperty(3)

    def __init__(self, pool, name):
        # pool забирает CppBackedEntity, а name=name летит дальше в BaseEntity
        super().__init__(pool, name=name)

print("=== ТЕСТ ИНТЕГРАЦИИ MSB + C++ ===\n")

# MSB теперь доволен, он получил свое имя
telescope1 = RadioTelescopeUV(global_uv_pool, "Спектр-Р")

telescope1.u = 100.5
telescope1.v = -50.2

print(f"Объект: {telescope1.name}")
print(f"Координаты u, v прочитаны из С++: {telescope1.u}, {telescope1.v}")

raw_proxy = global_uv_pool.get_proxy(telescope1.cpp_id)
print(f"Проверка через сырой C++ API: u={raw_proxy.data[0]}, v={raw_proxy.data[1]}")