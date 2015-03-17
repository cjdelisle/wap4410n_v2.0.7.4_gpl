#include <common.h>
#include <exports.h>

DECLARE_GLOBAL_DATA_PTR;

static void dummy(void)
{
}

unsigned long get_version(void)
{
	return XF_VERSION;
}

void jumptable_init (void)
{
	int i;

	gd->jt = (void **) malloc (XF_MAX * sizeof (void *));
	for (i = 0; i < XF_MAX; i++)
		gd->jt[i] = (void *) dummy;

	gd->jt[XF_get_version] = (void *) get_version;
	gd->jt[XF_malloc] = (void *) malloc;
	gd->jt[XF_free] = (void *) free;
	gd->jt[XF_get_timer] = (void *)get_timer;
	gd->jt[XF_udelay] = (void *)udelay;
#if defined(CONFIG_I386) || defined(CONFIG_PPC)
	gd->jt[XF_install_hdlr] = (void *) irq_install_handler;
	gd->jt[XF_free_hdlr] = (void *) irq_free_handler;
#endif	/* I386 || PPC */
#if (CONFIG_COMMANDS & CFG_CMD_I2C)
	gd->jt[XF_i2c_write] = (void *) i2c_write;
	gd->jt[XF_i2c_read] = (void *) i2c_read;
#endif	/* CFG_CMD_I2C */

#if 1  /*standalone download*/
	gd->jt[XF_dl_GetAddr] = (void *) dl_GetAddr;
	gd->jt[XF_dl_Initialize] = (void *) dl_Initialize;
	gd->jt[XF_ctrlc] = (void *) ctrlc;
	gd->jt[XF_AssignHWAddress] = (void *) AssignHWAddress;
	gd->jt[XF_sprintf] = (void *) sprintf;
	gd->jt[XF_setenv] = (void *) setenv;
	gd->jt[XF_dl_Transmit] = (void *) dl_Transmit;
	gd->jt[XF_reset] = (void *) reset;
	gd->jt[XF_memcpy] = (void *) memcpy;
	gd->jt[XF_memset] = (void *) memset;
	gd->jt[XF_sc_xchg] = (void *) sc_xchg;
	gd->jt[XF_dumpData] = (void *) dumpData;
	gd->jt[XF_FlashDriver] = (void *) FlashDriver;
	gd->jt[XF_memcmp] = (void *) memcmp;
	gd->jt[XF_saveenv] = (void *) saveenv;
	gd->jt[XF_NetLoop] = (void *) NetLoop;
	gd->jt[XF_delay_1s] = (void *) delay_1s;
	gd->jt[XF_ret_NetTxPacket] = (void *) ret_NetTxPacket;
#endif
}
