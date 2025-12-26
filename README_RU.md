# Stirlitz

Программа шифрования текста и файлов. Шифрование может осуществляться как для передачи через публичные каналы связи (файлы и текст шифруются с использованием публичных ключей на базе алгоритма Ed25519 и алгоритма AES256), так и для локального хранения (простое шифрование файлов с помощью имени пользователя и пароля на базе алгоритма AES256). Графический интерфейс пользователя реализован на базе Qt6. Криптографические функции вынесены в отдельную библиотеку stirlitz (может быть использована независимо).

## Установка
### Linux
`git clone https://github.com/ProfessorNavigator/stirlitz.git`\
`cd stirlitz`\
`cmake -DCMAKE_BUILD_TYPE=Release -B _build`\
`cmake --build _build`\
`cmake --install _build`

Вам могут потребоваться привилегии суперпользователя для выполнения последней команды.

Также вам может потребоваться задать префикс опцией CMAKE_INSTALL_PREFIX (перфикс по умолчанию `/usr/local`).

В состав проекта входит библиотека stirlitz. Для сборки документации stirlitz в формате html необходимо установить опцию CREATE_HTML_DOCS в `ON`.

### Windows
Для сборки и установки вам потребуется [MSYS2](https://www.msys2.org/). Кроме того вам нужно установить зависимости из секции `Зависимости`. После установки необходимых зависимостей откройте консоль MinGW и выполните следующие команды (в примере предполагается, что скачивание кода происходит в C:\Stirlitz):

`mkdir -pv /c/Stirlitz`\
`cd /c/Stirlitz`\
`git clone https://github.com/ProfessorNavigator/stirlitz.git`\
`cd stirlitz`\
`cmake -DCMAKE_BUILD_TYPE=Release -B _build`\
`cmake --build _build`\
`cmake --install _build`

Вам также обязательно необходимо указать префикс опцией CMAKE_INSTALL_PREFIX (префикс может быть например `/ucrt64` или `/mingw64`).

В состав проекта входит библиотека stirlitz. Для сборки документации stirlitz в формате html необходимо установить опцию CREATE_HTML_DOCS в `ON`.

### Android
Пользователи Android могут собрать проект самостоятельно с помощью соответствующих средств сборки (Android NDK) или воспользоваться готовыми экспериментальными сборками из релизов проекта на [Github](https://github.com/ProfessorNavigator/stirlitz). 

## Зависимости
Для сборки необходима библиотека [Qt6](https://www.qt.io/) (компоненты Core, Widgets), [libgcrypt](https://www.gnupg.org/software/libgcrypt/), [libgpg-error](https://www.gnupg.org/software/libgpg-error/). Сборка осуществляется с помощью [СMake](https://cmake.org/). Для сборки документации stirlitz необходим doxygen.

## Замечания
В состав проекта входит библиотека stirlitz (расположена в директории stirlitz). Для её независимой сборки могут быть использованы команды, аналогичные указанным в разделе `Установка`. Для сборки потребуются [libgcrypt](https://www.gnupg.org/software/libgcrypt/), [libgpg-error](https://www.gnupg.org/software/libgpg-error/). Сборка осуществляется с помощью [СMake](https://cmake.org/).

## Лицензия
GPLv3 (см. `COPYING`).

## Поддержка
Если есть желание поддержать проект, то можно пройти по следующей ссылке: [поддержка](https://yoomoney.ru/to/4100117795409573)

## Контакты автора
Вопросы, пожелания, предложения и отзывы можно направлять на следующий адрес: \
bobilev_yury@mail.ru