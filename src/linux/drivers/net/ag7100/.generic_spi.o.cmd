cmd_/home/raul/work/2010/11/wap4410n/2010.11.08/SDK_73/build/../linux/drivers/net/ag7100/generic_spi.o := mips-linux-uclibc-gcc -Wp,-MD,/home/raul/work/2010/11/wap4410n/2010.11.08/SDK_73/build/../linux/drivers/net/ag7100/.generic_spi.o.d  -nostdinc -isystem /home/raul/work/2010/11/wap4410n/2010.11.08/SDK_73/build/gcc-3.4.4-2.16.1/build_mips_nofpu/bin-ccache/../lib/gcc/mips-linux-uclibc/3.4.4/include -D__KERNEL__ -Iinclude  -include include/linux/autoconf.h -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -ffreestanding -O2     -fomit-frame-pointer  -I /home/raul/work/2010/11/wap4410n/2010.11.08/SDK_73/linux/kernels/mips-linux-2.6.15/include/asm/gcc -G 0 -mno-abicalls -fno-pic -pipe  -mabi=32 -march=mips32r2 -Wa,-32 -Wa,-march=mips32r2 -Wa,-mips32r2 -Wa,--trap -Iinclude/asm-mips/mach-ar7100 -Iinclude/asm-mips/mach-generic    -DMODULE -mlong-calls -DKBUILD_BASENAME=generic_spi -DKBUILD_MODNAME=ag7100_mod -c -o /home/raul/work/2010/11/wap4410n/2010.11.08/SDK_73/build/../linux/drivers/net/ag7100/.tmp_generic_spi.o /home/raul/work/2010/11/wap4410n/2010.11.08/SDK_73/build/../linux/drivers/net/ag7100/generic_spi.c

deps_/home/raul/work/2010/11/wap4410n/2010.11.08/SDK_73/build/../linux/drivers/net/ag7100/generic_spi.o := \
  /home/raul/work/2010/11/wap4410n/2010.11.08/SDK_73/build/../linux/drivers/net/ag7100/generic_spi.c \
    $(wildcard include/config/ar9100.h) \
  include/linux/kernel.h \
    $(wildcard include/config/preempt/voluntary.h) \
    $(wildcard include/config/debug/spinlock/sleep.h) \
    $(wildcard include/config/printk.h) \
    $(wildcard include/config/sysctl.h) \
  /home/raul/work/2010/11/wap4410n/2010.11.08/SDK_73/build/gcc-3.4.4-2.16.1/build_mips_nofpu/bin-ccache/../lib/gcc/mips-linux-uclibc/3.4.4/include/stdarg.h \
  include/linux/linkage.h \
  include/linux/config.h \
    $(wildcard include/config/h.h) \
  include/asm/linkage.h \
  include/linux/stddef.h \
  include/linux/compiler.h \
  include/linux/compiler-gcc3.h \
  include/linux/compiler-gcc.h \
  include/linux/types.h \
    $(wildcard include/config/uid16.h) \
  include/linux/posix_types.h \
  include/asm/posix_types.h \
  include/asm/sgidefs.h \
  include/asm/types.h \
    $(wildcard include/config/highmem.h) \
    $(wildcard include/config/64bit/phys/addr.h) \
    $(wildcard include/config/64bit.h) \
    $(wildcard include/config/lbd.h) \
  include/linux/bitops.h \
  include/asm/bitops.h \
    $(wildcard include/config/smp.h) \
    $(wildcard include/config/cpu/mips32.h) \
    $(wildcard include/config/cpu/mips64.h) \
    $(wildcard include/config/32bit.h) \
  include/asm/bug.h \
    $(wildcard include/config/bug.h) \
  include/asm/break.h \
  include/asm-generic/bug.h \
  include/asm/byteorder.h \
  include/linux/byteorder/big_endian.h \
  include/linux/byteorder/swab.h \
  include/linux/byteorder/generic.h \
  include/asm/cpu-features.h \
    $(wildcard include/config/mips/mt.h) \
    $(wildcard include/config/cpu/mipsr2.h) \
    $(wildcard include/config/cpu/mipsr2/irq/vi.h) \
    $(wildcard include/config/cpu/mipsr2/irq/ei.h) \
  include/asm/cpu.h \
  include/asm/cpu-info.h \
    $(wildcard include/config/sgi/ip27.h) \
  include/asm/cache.h \
    $(wildcard include/config/mips/l1/cache/shift.h) \
  include/asm-mips/mach-generic/kmalloc.h \
    $(wildcard include/config/dma/coherent.h) \
  include/asm-mips/mach-ar7100/cpu-feature-overrides.h \
  include/asm/interrupt.h \
    $(wildcard include/config/irq/cpu.h) \
  include/asm/hazards.h \
    $(wildcard include/config/cpu/rm9000.h) \
    $(wildcard include/config/cpu/r10000.h) \
    $(wildcard include/config/cpu/sb1.h) \
  include/asm/war.h \
    $(wildcard include/config/sgi/ip22.h) \
    $(wildcard include/config/sni/rm200/pci.h) \
    $(wildcard include/config/cpu/r5432.h) \
    $(wildcard include/config/sb1/pass/1/workarounds.h) \
    $(wildcard include/config/sb1/pass/2/workarounds.h) \
    $(wildcard include/config/mips/malta.h) \
    $(wildcard include/config/mips/atlas.h) \
    $(wildcard include/config/mips/sead.h) \
    $(wildcard include/config/cpu/tx49xx.h) \
    $(wildcard include/config/momenco/jaguar/atx.h) \
    $(wildcard include/config/pmc/yosemite.h) \
    $(wildcard include/config/momenco/ocelot/3.h) \
  include/linux/spinlock.h \
    $(wildcard include/config/debug/spinlock.h) \
    $(wildcard include/config/preempt.h) \
  include/linux/preempt.h \
    $(wildcard include/config/debug/preempt.h) \
  include/linux/thread_info.h \
  include/asm/thread_info.h \
    $(wildcard include/config/page/size/4kb.h) \
    $(wildcard include/config/page/size/8kb.h) \
    $(wildcard include/config/page/size/16kb.h) \
    $(wildcard include/config/page/size/64kb.h) \
    $(wildcard include/config/debug/stack/usage.h) \
  include/asm/processor.h \
    $(wildcard include/config/cpu/has/prefetch.h) \
  include/linux/threads.h \
    $(wildcard include/config/nr/cpus.h) \
    $(wildcard include/config/base/small.h) \
  include/asm/cachectl.h \
  include/asm/mipsregs.h \
    $(wildcard include/config/cpu/vr41xx.h) \
  include/asm/prefetch.h \
  include/asm/system.h \
    $(wildcard include/config/cpu/has/sync.h) \
    $(wildcard include/config/cpu/has/wb.h) \
  include/asm/addrspace.h \
    $(wildcard include/config/cpu/r4300.h) \
    $(wildcard include/config/cpu/r4x00.h) \
    $(wildcard include/config/cpu/r5000.h) \
    $(wildcard include/config/cpu/nevada.h) \
    $(wildcard include/config/cpu/r8000.h) \
    $(wildcard include/config/cpu/sb1a.h) \
  include/asm-mips/mach-generic/spaces.h \
    $(wildcard include/config/dma/noncoherent.h) \
  include/asm/dsp.h \
  include/asm/ptrace.h \
  include/asm/isadep.h \
    $(wildcard include/config/cpu/r3000.h) \
    $(wildcard include/config/cpu/tx39xx.h) \
  include/linux/stringify.h \
  include/linux/spinlock_types.h \
  include/linux/spinlock_types_up.h \
  include/linux/spinlock_up.h \
  include/linux/spinlock_api_up.h \
  include/asm/atomic.h \
  include/asm/delay.h \
  include/linux/param.h \
  include/asm/param.h \
  include/asm-mips/mach-generic/param.h \
  include/linux/smp.h \
  include/asm/compiler.h \
  include/asm/io.h \
    $(wildcard include/config/swap/io/space.h) \
  include/asm/page.h \
    $(wildcard include/config/need/multiple/nodes.h) \
    $(wildcard include/config/limited/dma.h) \
  include/asm-generic/page.h \
  include/asm/pgtable-bits.h \
    $(wildcard include/config/cpu/mips32/r1.h) \
    $(wildcard include/config/mips/uncached.h) \
  include/asm/string.h \
  include/asm-mips/mach-generic/ioremap.h \
  include/asm-mips/mach-ar7100/mangle-port.h \
  include/asm-mips/mach-ar7100/ar7100.h \
    $(wildcard include/config/base.h) \
    $(wildcard include/config/16bit.h) \
    $(wildcard include/config/page/open.h) \
    $(wildcard include/config/cas/lat/shift.h) \
    $(wildcard include/config/tmrd/shift.h) \
    $(wildcard include/config/trfc/shift.h) \
    $(wildcard include/config/trrd/shift.h) \
    $(wildcard include/config/trp/shift.h) \
    $(wildcard include/config/trcd/shift.h) \
    $(wildcard include/config/tras/shift.h) \
    $(wildcard include/config/sec/pll.h) \
    $(wildcard include/config/eth/int0/clock.h) \
    $(wildcard include/config/eth/int1/clock.h) \
    $(wildcard include/config/eth/ext/clock.h) \
    $(wildcard include/config/pci/clock.h) \
    $(wildcard include/config/pll/power/down/mask.h) \
    $(wildcard include/config/pll/bypass/mask.h) \
    $(wildcard include/config/pll/fb/shift.h) \
    $(wildcard include/config/pll/fb/mask.h) \
    $(wildcard include/config/pll/loop/bw/shift.h) \
    $(wildcard include/config/pll/loop/bw/mask.h) \
    $(wildcard include/config/cpu/div/shift.h) \
    $(wildcard include/config/cpu/div/mask.h) \
    $(wildcard include/config/ddr/div/shift.h) \
    $(wildcard include/config/ddr/div/mask.h) \
    $(wildcard include/config/ahb/div/shift.h) \
    $(wildcard include/config/ahb/div/mask.h) \
    $(wildcard include/config/locked/shift.h) \
    $(wildcard include/config/locked/mask.h) \
    $(wildcard include/config/sw/update/shift.h) \
    $(wildcard include/config/sw/update/mask.h) \
    $(wildcard include/config/enable.h) \
    $(wildcard include/config/reset.h) \
    $(wildcard include/config/delay.h) \
    $(wildcard include/config/mic/word/size.h) \
    $(wildcard include/config/mode.h) \
    $(wildcard include/config/data/word/size.h) \
    $(wildcard include/config/i2s/32b/word.h) \
    $(wildcard include/config/master.h) \
    $(wildcard include/config/psedge.h) \
  /home/raul/work/2010/11/wap4410n/2010.11.08/SDK_73/build/../linux/drivers/net/ag7100/generic_spi.h \
  /home/raul/work/2010/11/wap4410n/2010.11.08/SDK_73/build/../linux/drivers/net/ag7100/generic_i2c.h \

/home/raul/work/2010/11/wap4410n/2010.11.08/SDK_73/build/../linux/drivers/net/ag7100/generic_spi.o: $(deps_/home/raul/work/2010/11/wap4410n/2010.11.08/SDK_73/build/../linux/drivers/net/ag7100/generic_spi.o)

$(deps_/home/raul/work/2010/11/wap4410n/2010.11.08/SDK_73/build/../linux/drivers/net/ag7100/generic_spi.o):
