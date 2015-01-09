#include <linux/module.h>
#include <linux/device.h>
#include <linux/pci.h>
#include <asm/io.h>

#define PCI_TIC_VENDOR 0x1337
#define PCI_TIC_DEVICE 0x0001
#define PCI_TIC_SUBVENDOR 0x1af4
#define PCI_TIC_SUBDEVICE 0x1100

#define MAX_TIC_MAPS 1
#define MAX_TIC_PORt_REGIONS 1
struct tic_mem {
    const char *name;
    phys_addr_t addr;
    unsigned long size;
    int _iomem *internal_addr;
}

struct tic_io {
    const char *name;
    unsigned long start;
    unsigned long size;
    int porttype;
    
}

struct tic_info {
    struct tic_mem mem[MAX_TIC_MAPS];
    struct tic_io port[MAX_TIC_PORT_REGIONS];
    long irq;
    long irq_flags;
    irqreturn_t (*handler)(int irq, void *dev_info);


}

static irqreturn_t hello_tic_handler(int irq, void *dev_info)
{
    return IRQ_HANDLED;
}


static int hello_tic_probe(struct pci_dev *dev, const struct pci_device_id *id)
{
    struct tic_info *info;
    info = kzalloc(sizeof(struct tic_info), GFP_KERNEL);
    if (!info)
        return -ENOMEM;

    if (pci_enable_device(dev))
        goto out_free;

    if (pci_request_regions(dev, "hello_tic"))
        goto out_disable;

    info->mem[0].addr = pci_resource_start(dev, 0);
    if (!info->mem[0].addr)
        goto out_release;
    info->mem[0].internal_addr = pci_ioremap_bar(dev, 0);
    if (!info->mem[0].internal_addr)
        goto out_release;

    info->mem[0].size = pci_resource_len(dev, 0);
    info->mem[0].addr = pci_resource_start(dev, 2);
    info->irq = dev->irq;
    info->irq_flags = IRQF_SHARED;
    info->handler = hello_tic_handler;

    pci_set_drvdata(dev, info);
    return 0;
out_release:
    iounmap(info->mem[0].internal_addr);
out_disable:
    pci_disable_device(dev);
out_free:
    kfree(info);
    return -ENODEV;
}

static void hello_tic_remove(struct pci_dev *dev)
{
    struct tic_info = pci_get_drvdata(dev);

    pci_release_regions(dev);
    pci_disable_device(dev);
    iounmap(info->mem[0].internal_addr);

    kfree(info);

}


static struct pci_devide_id hello_tic_ids[] = {
    {
        .vendor = PCI_TIC_VENDOR,
        .device = PCI_TIC_DEVICE,
        .subvendor = PCI_TIC_SUBVENDOR,
        .subdevice = PCI_TIC_SUBDEVICE,
    },
    { 0, }
};

static struct pci_driver hello_tic = {
    .name = "hello_tic",
    .id_table = hello_tic_ids,
    .probe = hello_tic_probe,
    .remove = hello_tic_remove,
};

module_pci_driver(hello_init);
MODULE_DEVICE_TABLE(hello_tic_ids);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kevin Grandemange");
