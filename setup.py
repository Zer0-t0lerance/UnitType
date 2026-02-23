from setuptools import setup
# Специальный помощник pybind11 сам настроит все пути и библиотеки
from pybind11.setup_helpers import Pybind11Extension, build_ext

ext_modules = [
    Pybind11Extension(
        "unitype_sim",                # Имя модуля, которое мы будем импортировать в Python
        ["src/python_bindings.cpp"],  # Наш C++ файл-мост
        cxx_std=17,                   # Жестко требуем C++17
    ),
]

setup(
    name="unitype_sim",
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
)