from msb_arch import BaseEntity

class CppIndexProperty:
    def __init__(self, index):
        self.index = index

    def __get__(self, instance, owner):
        if instance is None:
            return self
        return instance._cpp_proxy.data[self.index]

    def __set__(self, instance, value):
        instance._cpp_proxy.data[self.index] = value

class CppBackedEntity(BaseEntity):
    def __init__(self, pool, *args, **kwargs):
        # 1. Честно инициализируем MSB
        super().__init__(*args, **kwargs)
        
        # 2. Взламываем защиту MSB: пишем напрямую в обход __setattr__
        new_id = pool.allocate_new()
        object.__setattr__(self, 'cpp_id', new_id)
        object.__setattr__(self, '_cpp_proxy', pool.get_proxy(new_id))

    def __setattr__(self, key, value):
        # Смотрим, есть ли такой атрибут на уровне класса
        cls = self.__class__
        attr_obj = getattr(cls, key, None)
        
        # Если это наш плюсовый дескриптор (u, v, Re и т.д.)
        if isinstance(attr_obj, CppIndexProperty):
            # Отдаем данные напрямую в С++, минуя паранойю MSB!
            attr_obj.__set__(self, value)
        else:
            # Иначе отдаем атрибут оригинальному MSB (пусть сам с ним разбирается)
            super().__setattr__(key, value)