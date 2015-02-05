
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/init.h>
#include <linux/irqdomain.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/slab.h>




struct dev{
    void __iomem *regs;
};

static struct dev test_dev;

static irqreturn_t test_interrupt(int irq, void *data)
{
    pr_alert("SYSBUS IRQ\n");
    /* do irq stuff */
    return IRQ_HANDLED;
}

static int test_probe(struct platform_device *pdev)
{
    struct dev *tdev = &test_dev;
    int err, irq;
    struct resource *res;
    pr_alert("SYSBUS PROBE\n");
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    tdev->regs = devm_ioremap_resource(&pdev->dev, res);

    if (IS_ERR(tdev->regs))
        return PTR_ERR(tdev->regs);
    irq = platform_get_irq(pdev, 0);
    pr_alert("irq=%d\n", irq);
    if (irq < 0) {
        dev_err(&pdev->dev, "missing irq\n");
        err = irq;
        return err;
    }
    pr_alert("requesting irq\n");
    err = devm_request_irq(&pdev->dev, irq, test_interrupt, IRQF_SHARED, dev_name(&pdev->dev),tdev);
    if (err) {
        dev_err(&pdev->dev, "failed to request IRQ #%d -> :%d\n",
                irq, err);
        return err;
    }
    pr_alert("requested irq\n");

    iowrite32(34, tdev->regs);
    pr_alert("probe read: %d\n", ioread32(tdev->regs));


    return 0;
}

static int test_remove(struct platform_device *pdev)
{
    pr_alert("SYSBUS REMOVE\n");
    return 0;
}

static const struct of_device_id test_of_match[] = {
    { .compatible = "tic_test"},
    {}
};
MODULE_DEVICE_TABLE(of, test_of_match);

static struct platform_driver test_driver = {
    .remove = test_remove,
    .probe = test_probe,
    .driver = {
        .name = "test",
        .of_match_table = test_of_match,
    },
};

module_platform_driver(test_driver);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Kevin Grandemange");
MODULE_DESCRIPTION("Test driver");
