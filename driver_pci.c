#include <linux/module.h>
#include <linux/device.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <asm/io.h>

#define PCI_TIC_VENDOR 0x1337
#define PCI_TIC_DEVICE 0x0001
#define PCI_TIC_SUBVENDOR 0x1af4
#define PCI_TIC_SUBDEVICE 0x1100

#define MAX_TIC_MAPS 1
#define MAX_TIC_PORT_REGIONS 1
static struct pci_driver hello_tic;
struct tic_mem {
    const char *name;
    void __iomem *start;
    unsigned long size;
};

struct tic_io {
    const char *name;
    unsigned long start;
    unsigned long size;
};

struct tic_info {
    struct tic_mem mem[MAX_TIC_MAPS];
    struct tic_io port[MAX_TIC_PORT_REGIONS];
    u8 irq;
};

static irqreturn_t hello_tic_handler(int irq, void *dev_info)
{
    struct tic_info *info = (struct tic_info *) dev_info;
    pr_alert("IRQ %d handled\n", irq);
    /* is it our device throwing an interrupt ? */
    if (inl(info->port[0].start)) {
        /* deassert it */
        outl(0, info->port[0].start );
        return IRQ_HANDLED;
    } else {
        return IRQ_NONE;
    }
}


static int hello_tic_probe(struct pci_dev *dev, const struct pci_device_id *id)
{
    struct tic_info *info;
    info = kzalloc(sizeof(struct tic_info), GFP_KERNEL);
    if (!info)
        return -ENOMEM;

    if (pci_enable_device(dev))
        goto out_free;
    pr_alert("enabled device\n");

    if (pci_request_regions(dev, "hello_tic"))
        goto out_disable;

    pr_alert("requested regions\n");

    /* BAR 0 has IO */
    info->port[0].name = "tic-io";
    info->port[0].start = pci_resource_start(dev, 0);
    info->port[0].size = pci_resource_len(dev, 0);

    /* BAR 1 has MMIO */
    info->mem[0].name = "tic-mmio";
    info->mem[0].start = pci_ioremap_bar(dev, 1);
    info->mem[0].size = pci_resource_len(dev, 1);
    if (!info->mem[0].start)
        goto out_unrequest;
    pr_alert("remaped addr for kernel uses\n");



    /* get pci irq */
    if (pci_read_config_byte(dev, PCI_INTERRUPT_LINE, &info->irq))
        goto out_iounmap;

    /* request irq */
    if (devm_request_irq(&dev->dev, info->irq, hello_tic_handler, IRQF_SHARED, hello_tic.name, (void *) info))
        goto out_iounmap;

    /* test io memory operations */
    pr_alert("device id=%x\n", ioread32(info->mem[0].start + 4));
    iowrite32(0x4567, info->mem[0].start + 4);
    pr_alert("modified device id=%x\n", ioread32(info->mem[0].start + 4));
    /* try writing io ports */
    /* assert an irq */
    outb(1, info->port[0].start);
    /* try dma without iommu */
    outl(1, info->port[0].start + 4);

    pci_set_drvdata(dev, info);
    return 0;

out_iounmap:
    pr_alert("tic:probe_out:iounmap");
    iounmap(info->mem[0].start);
out_unrequest:
    pr_alert("tic:probe_out_unrequest\n");
    pci_release_regions(dev);
out_disable:
    pr_alert("tic:probe_out_disable\n");
    pci_disable_device(dev);
out_free:
    pr_alert("tic:probe_out_free\n");
    kfree(info);
    return -ENODEV;
}

static void hello_tic_remove(struct pci_dev *dev)
{
    struct tic_info *info = pci_get_drvdata(dev);

    pci_release_regions(dev);
    pci_disable_device(dev);
    iounmap(info->mem[0].start);

    kfree(info);

}


static struct pci_device_id hello_tic_ids[] = {
    
    { PCI_DEVICE(PCI_TIC_VENDOR, PCI_TIC_DEVICE) },
    { 0, },
};

static struct pci_driver hello_tic = {
    .name = "hello_tic",
    .id_table = hello_tic_ids,
    .probe = hello_tic_probe,
    .remove = hello_tic_remove,
};

module_pci_driver(hello_tic);
MODULE_DEVICE_TABLE(pci, hello_tic_ids);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kevin Grandemange");
