/* HelloWorld sysbus device
 * Copyright (C) 2015 Kevin Grandemange
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <hw/sysbus.h>
#include <hw/qdev.h>
#include <hw/irq.h>
#define TYPE_DUMMY "test_tic"
#define DUMMY_SIZE (1 << 10)
#define DUMMY(obj)\
    OBJECT_CHECK(DummyState, (obj), TYPE_DUMMY)


typedef struct {
    SysBusDevice parent_obj;
    MemoryRegion iomem;
    qemu_irq irq;

} DummyState;

static void dummy_reset(DeviceState *d)
{
    return;
}

static void dummy_write(void *opaque, hwaddr addr,
                         uint64_t val, unsigned size)
{

    DummyState *dummy = DUMMY(opaque);
    printf("DUMMY. MMIO write Ordered, addr=%x, value=%lu, size=%d\n",(unsigned)  addr, val, size);
    /* let's say writing somewhere trigger a pulse irq */
    printf("pulsing irq\n");
    qemu_irq_pulse(dummy->irq);
    printf("irq up\n");
    qemu_irq_raise(dummy->irq);

}

static uint64_t dummy_read(void *opaque, hwaddr addr,
                           unsigned size)
{
    DummyState *dummy = DUMMY(opaque);
    printf("DUMMY. MMIO Read Ordered, addr =%x, size=%d\n",(unsigned)  addr, size);
    printf("irq down\n");
    qemu_irq_lower(dummy->irq);
    return 0;
}


static const MemoryRegionOps dummy_mem_ops = {
    .read = dummy_read,
    .write = dummy_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

/* for migration */
static const VMStateDescription dummy_vmstate = {
    .name = "test_tic",
    .version_id = 1,
    .minimum_version_id = 1,
    .fields = (VMStateField[]) {
        VMSTATE_END_OF_LIST(),
    }
};

static void dummy_init(Object *obj)
{
    SysBusDevice *dev = SYS_BUS_DEVICE(obj);
    DummyState *s = DUMMY(obj);
    fprintf(stderr, "INIT\n");
    memory_region_init_io(&s->iomem, OBJECT(s), &dummy_mem_ops, s, "dummy",
            DUMMY_SIZE);
    sysbus_init_mmio(dev, &s->iomem);
    sysbus_init_irq(dev, &s->irq);

}


static Property dummy_properties[] = {
    DEFINE_PROP_END_OF_LIST(),
};


static void dummy_realize(DeviceState *dev, Error **errp)
{  
    printf("Do you realize ?\n");
}

static void dummy_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->realize = dummy_realize;
    dc->reset = dummy_reset;
    dc->vmsd = &dummy_vmstate;
    dc->props = dummy_properties;

}   



static const TypeInfo dummy_info = {
    .name          = TYPE_DUMMY,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(DummyState),
    .instance_init = dummy_init,
    .class_init    = dummy_class_init,

};


static void dummy_register_types(void)
{
    type_register_static(&dummy_info);
}


type_init(dummy_register_types);
