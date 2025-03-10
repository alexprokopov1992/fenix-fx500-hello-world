# Fenix FX500 Hello World

Репозиторій для тестування публічних компонентів системи Fenix Auto FX500.

**Увага розробникам, залученим до розробки Fenix Auto!**
Не публікуйте тут нічого, що стосується системи Fenix Auto FX500: SSL-сертифікати, адреси серверів, тощо.

Цей репозиторій створено виключно для тестування збірки окремих компонентів, які є публічно доступними: драйвери, модулі, бібліотеки та інше. Тестування збірки в Zephyr RTOS, тощо.

## Зміст

- [Вступ](#вступ)
- [Встановлення інструментів та залежностей](#встановлення-інструментів-та-залежностей)
- [Підготовка до збірки](#підготовка-до-збірки)
- [Збірка проєкту](#збірка-проєкту)
- [Внесок у проєкт](#внесок-у-проєкт)
- [Ліцензія](#ліцензія)

## Вступ

Цей репозиторій містить опис процесу встановлення необхідного оточення, компіляторів, бібліотек, а також інструкції зі збірки, прошивки та іншого.

## Встановлення інструментів та залежностей

### Встановлення Zephyr SDK

Встановіть необхідні компоненти, залежно від вашої операційної системи:

- CMake
- Python
- Device compiler

Дивіться розділ Install dependencies [тут](https://docs.zephyrproject.org/latest/develop/getting_started/index.html).

Zephyr встановлювати не потрібно, він буде встановлений під час підготовки до збірки проєкту.

*Від Дениса:*  
Тут будуть команди для MacOS. Для інших систем вони можуть трохи відрізнятися.

## Підготовка до збірки

Склонуйте цей репозиторій:

```bash
git clone git@github.com:FenixUkraine/fenix-fx500-hello-world.git
cd fenix-fx500-hello-world
```

Рекомендується створити віртуальне оточення `venv`, активувати його, встановити `west`. Дивіться [інструкцію](https://docs.zephyrproject.org/latest/develop/getting_started/index.html).

```bash
python3 -m venv .venv
source .venv/bin/activate
python3 -m pip install west
west init -l app
west update -o=--depth=1 -n
west packages pip --install
west sdk install
```

*Примітка:*  
Якщо після `python3 -m pip install west`, `west` вже встановлений глобально, то при виконанні `west ...` виконується глобальна версія, а не з `.venv`. У такому випадку використовуйте `./.venv/bin/west ...` або `python3 -m west ...`.

## Збірка проєкту

Виконайте:

```bash
west build app --build-dir build-app -b fxa500_02
```

Прошити плату можна командою:

```bash
west build app --build-dir build-app -b fxa500_02 -t flash
```

Можна редагувати безпосередньо файли device tree, або використовувати оверлеї.

Приклад збірки прошивки без датчика удару:

```bash
west build app --build-dir build-app -b fxa500_02 -p -- -DEXTRA_DTC_OVERLAY_FILE=no-shock-sensor.overlay
```

### Тестування нативних компіляцій (Windows, Linux)

```bash
west build app --build-dir build-app-native_posix -b native_posix
```

Також є симулятор. Дивіться документацію до Zephyr.

```bash
west build app --build-dir build-app-qemu -b qemu_cortex_m0
```

Можна налагоджувати на іншій платі. Наприклад, з `hal_wch` можна тестувати на найдешевшому мікроконтролері (біля $0.1):

```bash
west build app --build-dir build-app-ch32v003evt -b ch32v003evt
```

## Внесок у проєкт

Ми вітаємо ваш внесок! Будь ласка, дотримуйтеся наступних кроків:

1. Зробіть форк цього репозиторію.
2. Створіть нову гілку (`git checkout -b feature-branch`).
3. Внесіть зміни та закомітьте їх (`git commit -am 'Додано нову функцію'`).
4. Відправте зміни у форк (`git push origin feature-branch`).
5. Створіть Pull Request.

## Інструкції та поради щодо роботи з репозиторіями компонентів

Проєкт має модульну архітектуру. Зміни зазвичай вносяться в модулі, а не в основний проєкт. Нижче наведено нагадування щодо зручної роботи з субмодулями.

Прості зміни можна вносити у відповідних репозиторіях модулів, відправляти зміни, а потім в основному проєкті виконувати:

```shell
west update -o=--depth=1 -n
```

Під час активного налагодження зручніше вносити правки в локальних копіях модулів. Механізми `west` імпортують модулі в режимі `detached HEAD`. Щоб відправити зміни в модулі, виконайте:

- Перейдіть до каталогу з модулем або відкрийте його в десктопному Git-клієнті.
- Створіть нову гілку.
- Закомітьте зміни.
- Відправте гілку.
- Створіть pull request на GitHub.
- Прийміть pull request на GitHub.
- Оновіть проєкт через `west`.

```shell
cd modules/shock-sensor-driver
git status
git checkout -b feature/my-changes-1
git add .
git commit -m "Опис змін."
git push -u baden feature/my-changes-1
```

Перед створенням коміту оновіть локальний стан гілки `main` і усуньте конфлікти:

```shell
cd modules/shock-sensor-driver
git status
git checkout -b feature/my-changes-1
git add .
git commit -m "Опис змін."
git checkout main
git pull origin main
git checkout feature/my-changes-1
git rebase main
```

Вирішення конфліктів:

```shell
git status  # Показує файли з конфліктами (позначені червоним)
```

Приклад конфлікту у файлі:

```diff
<<<<<<< HEAD
// Зміни з main
public void calculate() {
    int x = 10;
}
=======
// Ваші зміни з feature/login
public void calculate() {
    int x = 20;
}
>>>>>>> 1234567... Commit message
```

Виправте конфлікт. Плагін у VSCode або десктопний клієнт Git спрощують це.

- Позначте файл як вирішений.
- Продовжуйте rebase.

```shell
git add <file>
git rebase --continue
```

Або, якщо файл потрібно видалити:

```shell
git rm <file>
git rebase --continue
```

Після вирішення конфліктів відправте коміт. Він буде від актуальної гілки `main`.

```shell
git push -u baden feature/my-changes-1
```

Спробуйте `git rebase -i main` в інтерактивному режимі. Можливо, у вашій десктопній версії Git є зручний механізм для rebase.
