#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>

#define MAX_PATH 1024

void run_command(const char *cmd) {
    printf("Executing: %s\n", cmd);
    int result = system(cmd);
    if (result != 0) {
        printf("Command failed with code %d\n", result);
    }
}

void show_error(const char *msg) {
    printf("ERROR: %s\n", msg);
}

int main(int argc, char *argv[]) {
    printf("=== GnidOS Installer ===\n\n");
    
    // Проверяем права root
    if (geteuid() != 0) {
        show_error("This installer must be run as root");
        return 1;
    }
    
    // Получаем список дисков
    printf("Available disks:\n");
    run_command("lsblk -d -n -o NAME,SIZE,MODEL | grep -v \"loop\"");
    
    char disk[MAX_PATH];
    printf("\nEnter disk to install to (e.g. sda): ");
    if (scanf("%s", disk) != 1) {
        show_error("Invalid disk name");
        return 1;
    }
    
    // Подтверждение
    printf("\nWARNING: All data on /dev/%s will be destroyed!\n", disk);
    printf("Do you want to continue? (yes/no): ");
    
    char confirm[10];
    if (scanf("%s", confirm) != 1 || strcmp(confirm, "yes") != 0) {
        printf("Installation cancelled\n");
        return 0;
    }
    
    // Создаем раздел
    printf("\nCreating partition...\n");
    char cmd[MAX_PATH];
    snprintf(cmd, sizeof(cmd), "echo -e \"o\\nn\\np\\n1\\n\\n\\nw\" | fdisk /dev/%s", disk);
    run_command(cmd);
    
    // Форматируем раздел
    printf("\nSelect filesystem:\n");
    printf("1) FAT32\n");
    printf("2) NTFS\n");
    printf("Enter choice (1-2): ");
    
    int choice;
    if (scanf("%d", &choice) != 1 || choice < 1 || choice > 2) {
        show_error("Invalid choice");
        return 1;
    }
    
    char partition[MAX_PATH];
    snprintf(partition, sizeof(partition), "/dev/%s1", disk);
    
    if (choice == 1) {
        printf("Formatting as FAT32...\n");
        snprintf(cmd, sizeof(cmd), "mkfs.vfat -F32 %s", partition);
        run_command(cmd);
    } else {
        printf("Formatting as NTFS...\n");
        snprintf(cmd, sizeof(cmd), "mkfs.ntfs -F %s", partition);
        run_command(cmd);
    }
    
    // Монтируем раздел
    printf("Mounting partition...\n");
    run_command("mkdir -p /mnt/gnidos");
    snprintf(cmd, sizeof(cmd), "mount %s /mnt/gnidos", partition);
    run_command(cmd);
    
    // Копируем систему
    printf("Copying system files...\n");
    run_command("cp -ax / /mnt/gnidos");
    
    // Устанавливаем загрузчик
    printf("Installing bootloader...\n");
    snprintf(cmd, sizeof(cmd), "grub-install --target=i386-pc --root-directory=/mnt/gnidos /dev/%s", disk);
    run_command(cmd);
    
    // Копируем конфиг GRUB
    run_command("cp /boot/grub/grub.cfg /mnt/gnidos/boot/grub/");
    
    // Отмонтируем раздел
    printf("Unmounting partition...\n");
    run_command("umount /mnt/gnidos");
    
    printf("\nInstallation complete!\n");
    printf("You can now reboot and boot from the hard disk.\n");
    
    return 0;
}

