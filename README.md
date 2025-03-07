# fenix-fx500-hello-world

Репозиторій для тестування публічних компонентів системи Fenix Auto FX500.

**Увага розробникам, хто долучений до розробки Fenix Auto.!**

Не публікуйте тут нічого, шо має відношення до системи Fenix Auto FX500.
Будь якої, SSL-сертифікати, адреси серверів, тощо.

Цей репозиторій створений виключно для тестування збірки окремих компонентів,
які є публічно-доспупними, драйвери, модулі, бібліотеки, та інше.
Тестування збірки в Zephyr RTOS, тощо.

Цей репозиторій створено в основному для того, шоб описати як встановити оточення,
компілятори, бібліотеки. Інструкції по збірці, по прошивці, та інше.


# Встановлення інструментов та залежностей

## Встановлення Zephyr SDK.

Встановіть базові речі, в залежності від вашої операційної системи:

- CMake
- Python
- Device compiller

Дивись розділ Install dependencies (https://docs.zephyrproject.org/latest/develop/getting_started/index.html).

Сам Zephyr встановлювати не треба, він буде встановлений в процесі підготовки до збірки цого проєкту.

/від Денис:/ Я тут буду залишати команди для MacOS, для інших систем будуть трохи відрізнятись.

# Підготовка до збірки

Склонуйте цей репозиторій.

```
git clone git@github.com:FenixUkraine/fenix-fx500-hello-world.git
cd fenix-fx500-hello-world
```

Бажано створювати віртуальне оточення venv, активувати, встановити west, дивись [інструкцію](https://docs.zephyrproject.org/latest/develop/getting_started/index.html).

```
python3 -m venv .venv
source .venv/bin/activate
python3 -m pip install west
west init -l app
west update -o=--depth=1 -n
west packages pip --install
west sdk install
```

/Примітка:/
Після `python3 -m pip install west`, якшо west ще встановлен і глобально, то при виконані `west ...` виконується глобальний, а не той шо встановлен
у `.venv`. Доводиться або виконувати через `./.venv/bin/west ...`, або
через `python3 -m west...`.

В моєму випадку допомогло деактивувати та активувати .venv. Але зазвичай цього не трапляється.

# Збірка проєкта

Виконайте:

```
west build app --build-dir build-app -b fxa500_02
```

Прошити плату можна командою:

```
west build app --build-dir build-app -b fxa500_02 -t flash
```

## До речі. Під Windows та Linux, можна тестувати шось через натівну компіляцію

```
west build app --build-dir build-app-native_posix -b native_posix
```

Ще є сімулятор, дивись документацію до Zephyr.

```
west build app --build-dir build-app-qemu -b qemu_cortex_m0
```

Можна на іншій платі шось відлагоджувати. Як приклад, додан `hal_wch`, і можна
шось тестувати на (мабуть) на найдешевшому (біля $0.1) на планеті мікроконтроллері:

```
west build app --build-dir build-app-ch32v003evt -b ch32v003evt
```
