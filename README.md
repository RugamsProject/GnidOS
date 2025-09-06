# GnidOS - DOS-like Linux Distribution

Minimalist Linux distribution with DOS-like command line interface.

## Features
- DOS-like command interface
- Support for ASM and C compilation
- Text-based installer
- Minimal Linux kernel with BusyBox

## Building
Run ./build.sh to build the system

## Testing
Use QEMU to test the images:
```
qemu-system-x86_64 -kernel output/images/bzImage -initrd output/images/rootfs.cpio.gz -nographic -append "console=ttyS0"
```

# GnidOS
