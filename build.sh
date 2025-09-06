#!/bin/bash

set -e

# Конфигурация
BUILDROOT_VERSION="2023.02"
TARGET_ARCH="x86_64"
CROSS_COMPILE="x86_64-linux-"

echo "=== Building GnidOS ==="

# Проверяем, что путь не содержит пробелов
if [[ "$PWD" == *" "* ]]; then
    echo "Error: Buildroot does not support paths with spaces."
    echo "Please move the project to a path without spaces and try again."
    exit 1
fi

# Создаем директории
mkdir -p output/images output/rootfs output/toolchain
mkdir -p overlay/{bin,sbin,usr/bin,usr/share/gnidos,etc/{init.d,profile.d}}

# Проверяем наличие Buildroot
if [ ! -d "buildroot" ]; then
    echo "Downloading Buildroot..."
    git clone --depth 1 --branch $BUILDROOT_VERSION https://github.com/buildroot/buildroot.git
fi

# Копируем конфигурацию Buildroot
cp config/buildroot.config buildroot/.config

# Компилируем компоненты
echo "Building components..."
make -C src

# Копируем скомпилированные компоненты в overlay
cp src/dos overlay/bin/
cp src/gnidos-installer overlay/sbin/
cp scripts/asm overlay/usr/bin/
cp scripts/runc overlay/usr/bin/
cp config/help.txt overlay/usr/share/gnidos/
cp config/profile overlay/etc/
cp config/rcS overlay/etc/init.d/

# Переходим в Buildroot и собираем систему
cd buildroot

# Настраиваем Buildroot с нашей конфигурацией
make olddefconfig

# Собираем систему
echo "Building system with Buildroot..."
make -j$(nproc)

cd ..

echo "=== Build complete ==="
echo "Images available in output/images/"
echo "Test with: qemu-system-x86_64 -kernel output/images/bzImage -initrd output/images/rootfs.cpio -nographic -append \"console=ttyS0\""