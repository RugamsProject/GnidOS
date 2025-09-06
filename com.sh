# Создаем пустой образ диска (256MB)
dd if=/dev/zero of=output/images/gnidos.img bs=1M count=256

# Форматируем образ как MBR с одним разделом
echo -e "o\nn\np\n1\n\n\nt\nc\na\n1\nw" | fdisk output/images/gnidos.img

# Создаем loop-устройство для образа
sudo losetup -fP output/images/gnidos.img
LOOP_DEVICE=$(sudo losetup -a | grep gnidos.img | awk -F: '{print $1}' | tail -n1)
sudo partprobe $LOOP_DEVICE

# Даем системе время на обработку
sleep 2

# Форматируем раздел как ext4
sudo mkfs.ext4 ${LOOP_DEVICE}p1

# Монтируем раздел
mkdir -p /mnt/gnidos-img
sudo mount ${LOOP_DEVICE}p1 /mnt/gnidos-img

# Копируем файлы из rootfs.ext2
sudo mkdir -p /mnt/gnidos-tmp
sudo mount -o loop output/images/rootfs.ext2 /mnt/gnidos-tmp
sudo cp -a /mnt/gnidos-tmp/* /mnt/gnidos-img/
sudo umount /mnt/gnidos-tmp
rmdir /mnt/gnidos-tmp

# Устанавливаем GRUB
sudo grub-install --target=i386-pc --boot-directory=/mnt/gnidos-img/boot --modules="part_msdos ext2" $LOOP_DEVICE

# Создаем конфиг GRUB
sudo mkdir -p /mnt/gnidos-img/boot/grub
cat << EOF | sudo tee /mnt/gnidos-img/boot/grub/grub.cfg
menuentry "GnidOS" {
    linux /boot/bzImage root=/dev/sda1 console=tty0 console=ttyS0
    initrd /boot/initrd.gz
}
EOF

# Копируем ядро и initramfs
sudo cp buildroot/output/images/bzImage /mnt/gnidos-img/boot/ 2>/dev/null || \
sudo cp output/images/bzImage /mnt/gnidos-img/boot/ 2>/dev/null || \
echo "Warning: bzImage not found, you may need to copy it manually"

sudo cp output/images/rootfs.cpio.gz /mnt/gnidos-img/boot/initrd.gz

# Размонтируем и очищаем
sudo umount /mnt/gnidos-img
sudo losetup -d $LOOP_DEVICE
rmdir /mnt/gnidos-img

echo "IMG image created: output/images/gnidos.img"