qemu_devices
============

Hello World PCI device for qemu
Emulate:
    MMIO
    PIO
    IRQ
    DMA without IOMMU (in vga space, just for the visual effect ^^)

Hello World PCI driver for it. (linux 3.2, with little tweek it should work with 2.6 too (I use devm framework in some place))

copy hello_tic.c in hw/char for example and add it to the Makefile too then make

make the driver as a module and load it, it will test the device (there is no framework interface for now)

Qemu cmdline:
i386-softmmu/qemu-system-i386  yourimg.qcow2 -device pci-hellodev --enable-kvm -monitor stdio
