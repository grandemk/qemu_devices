#include "hw/hw.h"
#include "hw/pci/pci.h"
#include "qemu/event_notifier.h"
#include <time.h>
#include "qemu/osdep.h"


typedef struct PCIHelloDevState {
    PCIDevice parent_obj;

    /* for PIO */
    MemoryRegion io;
    /* for MMIO */
    MemoryRegion mmio;
    qemu_irq irq;

} PCIHelloDevState;

#define TYPE_PCI_HELLO_DEV "pci-hellodev"
#define PCI_HELLO_DEV(obj)     OBJECT_CHECK(PCIHelloDevState, (obj), TYPE_PCI_HELLO_DEV)

static void hello_iowrite(void *opaque, hwaddr addr, uint64_t value, unsigned size)
{
    int i;
    printf("Write Ordered, addr=%x, value=%lu, size=%d\n", (unsigned) addr, value, size);
    /* trigger a dma in vga adress space */
    char *buf = malloc(0x1ffff * sizeof(char));
    for ( i = 0; i < 0x1ffff; ++i)
        buf[i] = rand();
    cpu_physical_memory_write(0xa0000, (void *) buf, 0x1ffff);

}

static uint64_t hello_ioread(void *opaque, hwaddr addr, unsigned size)
{
    PCIDevice *pci_dev = (PCIDevice *) opaque;
    /* trigger irq */
    /* just playing around with this
     * we need a pci driver to
     * really test this
     */
    pci_set_irq(pci_dev, 1);
    printf("Read Ordered, addr =%x, size=%d\n", (unsigned) addr, size);
    if (addr == 0) {
        printf("irq assert\n");
        pci_irq_assert(pci_dev);
    } else {
        printf("irq deassert\n");
        pci_irq_deassert(pci_dev);
    }
    return 0xabcdef;
}

static uint64_t hello_mmioread(void *opaque, hwaddr addr, unsigned size)
{
    printf("MMIO Read Ordered, addr =%x, size=%d\n",(unsigned)  addr, size);
    return 0xabcdef;
}

static void hello_mmiowrite(void *opaque, hwaddr addr, uint64_t value, unsigned size)
{
    printf("MMIO write Ordered, addr=%x, value=%lu, size=%d\n",(unsigned)  addr, value, size);
}



/*
 * Callback called when the Memory Region
 * representing the MMIO space is
 * accessed.
 */ 
static const MemoryRegionOps hello_mmio_ops = {
    .read = hello_mmioread,
    .write = hello_mmiowrite,
    .endianness = DEVICE_NATIVE_ENDIAN,
    .valid = {
        .min_access_size = 4,
        .max_access_size = 4,
    },
};

/*
 * Callback called when the Memory Region
 * representing the PIO space is
 * accessed. 
 */ 
static const MemoryRegionOps hello_io_ops = {
    .read = hello_ioread,
    .write = hello_iowrite,
    .endianness = DEVICE_NATIVE_ENDIAN,
    .valid = {
        .min_access_size = 4,
        .max_access_size = 4,
    },
};

/* Callback for MMIO and PIO regions are registered here */
static void hello_io_setup(PCIHelloDevState *d) 
{
    memory_region_init_io(&d->mmio, OBJECT(d), &hello_mmio_ops, d, "hello_mmio", 1<<6);
    memory_region_init_io(&d->io, OBJECT(d), &hello_io_ops, d, "hello_io", R_MAX*4);
}

static int pci_hellodev_init(PCIDevice *pci_dev)
{
    PCIHelloDevState *d = PCI_HELLO_DEV(pci_dev);
    printf("d=%lu\n", (unsigned long) &d);
    uint8_t *pci_conf;

    hello_io_setup(d);
    /*
     * See linux device driver (Edition 3) for the definition of a bar
     * in the PCI bus.
     */
    pci_register_bar(pci_dev, 0, PCI_BASE_ADDRESS_SPACE_IO, &d->io);
    pci_register_bar(pci_dev, 1, PCI_BASE_ADDRESS_SPACE_MEMORY, &d->mmio);

    pci_conf = pci_dev->config;
    /* also in ldd, a pci device has 4 pin for interrupt
     * here we use pin B.
     */
    pci_conf[PCI_INTERRUPT_PIN] = 0x02; 
    d->irq = pci_allocate_irq(pci_dev);

    printf("Hello World loaded\n");
    return 0;
}

/* When device is loaded */
static void pci_hellodev_uninit(PCIDevice *dev)
{
    printf("Good bye World unloaded\n");
}

/* When device is unloaded 
 * Can be useful for hot(un)plugging
 */
static void qdev_pci_hellodev_reset(DeviceState *dev)
{
    printf("Reset World\n");
}

/*
 * TODO
 */
static Property hello_properties[] = {

    DEFINE_PROP_END_OF_LIST(),
};

/* Called when the device is defined
 * PCI configuration is defined here
 * We inherit from PCIDeviceClass
 * Also see ldd for the meaning of the different args
 */
static void pci_hellodev_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    PCIDeviceClass *k = PCI_DEVICE_CLASS(klass);
    k->init = pci_hellodev_init;
    k->exit = pci_hellodev_uninit;
    k->vendor_id = 0x1337;
    k->device_id = 0x0001;
    k->revision  = 0x00;
    k->class_id = PCI_CLASS_OTHERS;
    dc->desc = "PCI Hello World";
    dc->props = hello_properties;
    set_bit(DEVICE_CATEGORY_MISC, dc->categories);
    dc->reset = qdev_pci_hellodev_reset;
}

/* Contains all the informations of the device
 * we are creating.
 * class_init will be called when we are defining
 * our device.
 */
static const TypeInfo pci_hello_info = {
    .name           = TYPE_PCI_HELLO_DEV,
    .parent         = TYPE_PCI_DEVICE,
    .instance_size  = sizeof(PCIHelloDevState),
    .class_init     = pci_hellodev_class_init,
};

/* function called before the qemu main
 * it will define our device
 */
static void pci_hello_register_types(void) 
{
    type_register_static(&pci_hello_info);
}

/* macro actually defining our device */
type_init(pci_hello_register_types);
