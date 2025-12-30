# Stirlitz
Stirlitz is designed for text and files encryption. Encryption can be carried out for text and files transmission by public communications channels (encryption based on Ed25519 public keys algorithm and on AES256 algorithm) and for local files (based on simple user name, password and AES256 algorithm). GUI is based on Qt6. Cryptographic methods are moved to stirlitz library (can be used independently).

## Installation
### Linux
`git clone https://github.com/ProfessorNavigator/stirlitz.git`\
`cd stirlitz`\
`cmake -DCMAKE_BUILD_TYPE=Release -B _build`\
`cmake --build _build`\
`cmake --install _build`

You may need superuser privileges to execute last command.

You may need to set install prefix by option CMAKE_INSTALL_PREFIX (default prefix is `/usr/local`).

Stirlitz includes stirlitz library. To build html documentation for this library set CREATE_HTML_DOCS to `ON`.

### Windows
You can build Stirlitz from sources by [MSYS2](https://www.msys2.org/) project assistance. Follow installation instructions from projects site, install dependencies from `Dependencies` section and git, then create directory you want to download source code to (path must not include spaces or non ASCII symbols). Open MinGW console and execute following commands (in example we download code to C:\Stirlitz):

`mkdir -pv /c/Stirlitz`\
`cd /c/Stirlitz`\
`git clone https://github.com/ProfessorNavigator/stirlitz.git`\
`cd stirlitz`\
`cmake -DCMAKE_BUILD_TYPE=Release -B _build`\
`cmake --build _build`\
`cmake --install _build`

Also you need to set CMAKE_INSTALL_PREFIX (depends from your choice it can be for example `/ucrt64` or `/mingw64`).

Stirlitz includes stirlitz library. To build html documentation for this library set CREATE_HTML_DOCS to `ON`.

### Android
Android users can build project from sources by Android NDK or can use experimental packages from [Github](https://github.com/ProfessorNavigator/stirlitz) project releases.

## Dependencies
You need [Qt6](https://www.qt.io/) library (Core and Widgets components), [libgcrypt](https://www.gnupg.org/software/libgcrypt/), [libgpg-error](https://www.gnupg.org/software/libgpg-error/). Also you need [СMake](https://cmake.org/) for building. If you plan to build stirlitz documentation, you also need doxygen.

## Notes
Stirlitz includes stirlitz library (can be found ind stirlitz directory). To use stirlitz library independently you can build it by same commands as noted in `Installation`. You need [libgcrypt](https://www.gnupg.org/software/libgcrypt/), [libgpg-error](https://www.gnupg.org/software/libgpg-error/) and [СMake](https://cmake.org/).

# License
GPLv3 (see `COPYING`).

## Donation
If you want to help to develop this project, you can assist it by [donation](https://yoomoney.ru/to/4100117795409573).

## Contacts
You can contact author by email \
bobilev_yury@mail.ru