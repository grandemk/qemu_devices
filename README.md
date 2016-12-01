# qemu_devices

## Hello World PCI device for qemu

### List of supported features the device emulate:

   - MMIO
   - PIO
   - IRQ
   - DMA without IOMMU (in vga space, just for the visual effect ^^)

Hello World PCI driver for it. (linux 3.2, with little tweek it should work with 2.6 too (I use devm framework in some place))

### install

As this repo just contains the files for the device, you need to integrate it in Qemu.
To do that:
    copy hello_tic.c in hw/char for example and add it to the Makefile too then make

For the driver, you need to compile it with you kernel of choice 
insmoding the driver will test the device

### Qemu cmdline:

i386-softmmu/qemu-system-i386  yourimg.qcow2 -device pci-hellodev --enable-kvm -monitor stdio

## Hello World Sysbus device

Notice: Use this only as inspiration to create your own device.
You should use the device tree way in qemu-arm and not the verstatilepb platform

(for those who don't fear atrocious install process)

- copy versatilepb in qemu/hw/arm
- get buildroot 
- use dts and buildroot configs from this git repository
- build the arm system
- Buildroot will create a cross compilation toolchain in output/host/usr/bin
- set ARCH=arm and CROSS_COMPILE=path-to-toolchain-prefix
- cross compile the kernel module driver_platform for arm
- cp it inside output/target (which will be shipped as the rootfs at the end of the build)
- make again 
- in order to get the device tree blob you can do dtc -O dtb -I dts -o tic.dtb tic2.dts 
- /path_to_your_qemu/arm-softmmu/qemu-system-arm -M versatilepb -kernel output/images/zImage  -append "root=/dev/ram console=ttyAMA0,115200 earlyprintk" -nographic -dtb tic.dtb

versatilepb seems to use a deprecated api for the registration of the device, and we can't use the -device switch, this isn't good, as we have to add specific code inside the board to wire our device inside instead of dynamic mapping

