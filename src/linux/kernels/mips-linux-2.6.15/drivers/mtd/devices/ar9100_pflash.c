/*
 * This file contains glue for Atheros ar9100 pf flash interface
 * Primitives are ar9100_pf_*
 * mtd flash implements are ar9100_flash_*
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <asm/delay.h>
#include <asm/io.h>
#include <asm/semaphore.h>

#include "ar7100.h"
#include "ar9100_pflash.h"
#define  _SC_CODE_

/* this is passed in as a boot parameter by bootloader */
extern int __ath_flash_size;
extern void ar9100_write(uint32_t, CFG_FLASH_WORD_SIZE);
extern CFG_FLASH_WORD_SIZE ar9100_read(uint32_t offset);

/*
 * bank geometry
 */
typedef struct ar9100_flash_geom {
    uint16_t vendor_id;
    uint16_t device_id;
    char *name;
    uint32_t sector_size;
    uint32_t size;
} ar9100_flash_geom_t;


/*
 * statics

static int ar9100_pflash_erase(ar9100_flash_geom_t *geom,int s_first, int s_last);
static int ar9100_pflash_write_word(ar9100_flash_geom_t *geom,unsigned long dest, unsigned long data);
static int ar9100_pflash_write_buff(ar9100_flash_geom_t *geom,u_char * src, unsigned long addr, unsigned long cnt);
static int ar9100_flash_probe(void);
 */
static const char *part_probes[] __initdata =
    { "cmdlinepart", "RedBoot", NULL };

static DECLARE_MUTEX(ar9100_flash_sem);
static DECLARE_MUTEX(ar7100_flash_sem);

/* GLOBAL FUNCTIONS */
void ar9100_pflash_down(void)
{
    down(&ar9100_flash_sem);
}

void ar9100_pflash_up(void)
{
    up(&ar9100_flash_sem);
}

/* For spi flash controller */
void ar7100_flash_spi_down(void)
{
    down(&ar7100_flash_sem);
}

void ar7100_flash_spi_up(void)
{
    up(&ar7100_flash_sem);
}

EXPORT_SYMBOL(ar7100_flash_spi_down);
EXPORT_SYMBOL(ar7100_flash_spi_up);

#ifdef _SC_CODE_
/* ID's read from the supported FLASH chips in the system */
#define UNKNOWN_CHIP_ID         (0xffff)    /* Anything but one of the following */
#define AM29LV320DB_CHIP_ID     (0x01f9)
#define AM29DS323D_CHIP_ID      (0x0150)
#define AM29LV320MB_CHIP_ID     (0x017e)    
#define AT29C040_CHIP_ID        (0x1f5b)
#define AT29C040A_CHIP_ID       (0x1fa4)
#define AT49BV1614_CHIP_ID      (0x1fc0)
#define AM29F040_CHIP_ID        (0x01a4)
#define AM29LV081B_CHIP_ID      (0x0138)
#define AM29LV040B_CHIP_ID      (0x014F)
#define AM29LV017B_CHIP_ID      (0x01c8)
#define AM29LV116BB_CHIP_ID     (0x014c)
#define AM29LV116BT_CHIP_ID     (0x01c7)
#define AM29LV800BB_CHIP_ID     (0x015b)
#define AM29LV800BT_CHIP_ID     (0x01da)
#define AM29LV160BB_CHIP_ID     (0x0149)
#define AM29LV160BT_CHIP_ID     (0x01c4)
#define HY29LV160BB_CHIP_ID     (0xad49)
#define HY29LV160BT_CHIP_ID     (0xadc4)
#define MT28F008B3B_CHIP_ID     (0x8999)
#define MT28F008B3T_CHIP_ID     (0x8998)
#define INT28F800B5B_CHIP_ID    (0x899d)
#define INT28F320J5_CHIP_ID     (0x8914)
#define INT28F320J3A_CHIP_ID    (0x8916)
#define INT28F640J3A_CHIP_ID    (0x8917)
#define INT28F128J3A_CHIP_ID    (0x8918)
#define INT28F320C3B_CHIP_ID    (0x89c5)
#define W29C040_CHIP_ID         (0xda46)
#define MX29L1611B_CHIP_ID      (0xc2f8)
#define M29W008T_CHIP_ID        (0x20D2)
#define M29W008B_CHIP_ID        (0x20DC)
#define M29W040B_CHIP_ID        (0x20E3)
#define M29W800AT_CHIP_ID       (0x20D7)
#define M29W800AB_CHIP_ID       (0x205B)
#define M29W160DT_CHIP_ID       (0x20C4)
#define BM29F040_CHIP_ID        (0xAD40)
#define MX29F040_CHIP_ID        (0xC2A4)
#define EN29F040_CHIP_ID        (0x7F7F)
#define AC29F040_CHIP_ID        (0x3786)
#define MX29F800B_CHIP_ID       (0xC258)
#define SST39VF080Q_CHIP_ID     (0xBFD8)
#define SST39VF016Q_CHIP_ID     (0xBFD9)
#define SST28SF040A_CHIP_ID     (0xBF04)
#define LH28F160BVE_CHIP_ID     (0xb049)
#define LH28F160BJE_CHIP_ID     (0xb0e9)
#define AT49LV321T_CHIP_ID      (0x1fc9)
#define AM29LV008BB_CHIP_ID     (0x0137)
#define S29AL032D_CHIP_ID       (0x01f6)

/* 16-bit device IDs */
#define AT29C1024_CHIP_ID       (0x001f0025)
#define AT49BV320_CHIP_ID       (0x001f00c8)
#define INT28F640J3A16_CHIP_ID  (0x00890017)
#define LH28F160BVE16_CHIP_ID   (0x00b00049)
#define LH28F160BJE16_CHIP_ID   (0x00b000e9)

//dyno
#define MX29LV320B_CHIP_ID                      (0xC2A8)
#define AT49LV321B_CHIP_ID                      (0x1FC8)
#define MX29LV640BTT_CHIP_ID					(0xC2C9)
#define MX29LV640DBT_CHIP_ID					(0xC2CB)

/* The following device IDs have the same device code and 
   need to distinguish between themselves using additional
   device code. The additional device code is put into the MS byte */
#define AT49BV161_FAMILY_ID     (0x00001fc2)
#define AT49BV16116_FAMILY_ID   (0x001f00c2)
#define AT49BV1614T_CHIP_ID     (0x00001fc2)
#define AT49BV1614T16_CHIP_ID   (0x001f00c2)
#define AT49BV1614AT_CHIP_ID    (0xc8001fc2)
#define AT49BV1614AT16_CHIP_ID  (0xc81f00c2)
#define AT49BV161T_CHIP_ID      (0x08001fc2)
#define AT49BV161T16_CHIP_ID    (0x081f00c2)

/* Spansion */
#define S29GL128N_CHIP_ID       (0x0001227e)

#endif
#ifdef _SC_CODE_

static uint type_1_num = 0;
static uint type_2_num = 0;
static uint type_1_size = 0;
static uint type_2_size = 0;
static uint index_num = 0;

#define NUMBER_DIFFERENT_SECTOR_SIZES	4
typedef struct flash_sector_tag
{
    uint             size;
    uint             numberSectors;
} FlashSector;
typedef struct flash_type_tag
{
    ulong            id;
    uint             size;
    const char *    partname;
    FlashSector     sectors [NUMBER_DIFFERENT_SECTOR_SIZES];
} FlashType;
static const FlashType   flashTypes[] =
{
     { S29AL032D_CHIP_ID,  	4096UL * 1024UL, "S29AL032D",   { {65536UL, 64UL},  {0UL, 0UL},      {0UL, 0UL}, {0UL, 0UL}}},
     { AM29LV320DB_CHIP_ID, 4096UL * 1024UL, "29LV320DB",   { {8192UL, 8UL},    {65536UL, 63UL}, {0UL, 0UL}, {0UL, 0UL}}},
     { MX29LV320B_CHIP_ID, 	4096UL * 1024UL, "MX29LV320B",  { {8192UL, 8UL},    {65536UL, 63UL}, {0UL, 0UL}, {0UL, 0UL}}},
     { MX29LV640BTT_CHIP_ID,8192UL * 1024UL, "MX29LV640BTT",{ {8192UL, 8UL},    {65536UL, 127UL},{0UL, 0UL}, {0UL, 0UL}}},
     { MX29LV640DBT_CHIP_ID,8192UL * 1024UL, "MX29LV640DBT",{ {8192UL, 8UL},    {65536UL, 127UL},{0UL, 0UL}, {0UL, 0UL}}},
     { UNKNOWN_CHIP_ID,     0UL,             "UNKNOWN",		{ {0UL, 0UL},       {0UL, 0UL},      {0UL, 0UL}, {0UL, 0UL}}}
};                                           
#define NUMBER_FLASH_TYPES  ((sizeof (flashTypes) / sizeof (flashTypes [0])) - 1)
#else
ar9100_flash_geom_t flash_geom_tbl[] = {
    {0x00bf, 0x2780, "SST-39VF400", 0x01000, 0x080000},	/* 512KB */
    {0x00bf, 0x2782, "SST-39VF160", 0x01000, 0x200000},	/* 2MB */
    {0x00bf, 0x236b, "SST-39VF6401", 0x01000, 0x800000},
    {0x00bf, 0x236a, "SST-39VF6402", 0x01000, 0x800000},
    {0x00bf, 0x236d, "SST-39VF6402", 0x01000, 0x800000},
    {0x0001, 0x227e, "AMD-SPANSION", 0x02000, 0x1000000},  /* 16 MB  */
    {0xffff, 0xffff, NULL, 0, 0}	/* end list */
};
#endif


/*
 * statics
 */
static int ar9100_pflash_erase(FlashType *geom,int s_first, int s_last);
static int ar9100_pflash_write_word(FlashType *geom,unsigned long dest, unsigned long data);
static int ar9100_pflash_write_buff(FlashType *geom,u_char * src, unsigned long addr, unsigned long cnt);
static int ar9100_flash_probe(void);


static int ar9100_flash_probe()
{
    uint16_t venid, devid, i;

#ifdef _SC_CODE_
    ushort chip_id;
#endif

    /* issue JEDEC query */

    ar9100_write(CFG_FLASH_ADDR0, FLASH_Setup_Code1);
    ar9100_write(CFG_FLASH_ADDR1, FLASH_Setup_Code2);
    ar9100_write(CFG_FLASH_ADDR0, FLASH_Jedec_Query);

    udelay(10000);

    venid = ar9100_read(0);
    devid = ar9100_read(1);

    /* issue software exit */
    ar9100_write(CFG_FLASH_ADDR0, FLASH_Setup_Code1);
    ar9100_write(CFG_FLASH_ADDR1, FLASH_Setup_Code2);
    ar9100_write(CFG_FLASH_ADDR0, FLASH_Soft_Exit);

    udelay(10000);

#ifdef _SC_CODE_
	printk("venid(%08x) , devid(%08x)\n", venid, devid);
	chip_id = ((venid<<8)&0xff00)|(devid&0xff);
	
	for (i = 0; i < NUMBER_FLASH_TYPES; i++) 
    {
        if (chip_id == flashTypes[i].id) {
            break;
        }
    }	
    printk("FLASH ID: %s ", flashTypes[i].partname);
    if (flashTypes[i].size >= 0x100000)
        printk("SIZE: (%d MB)\n", flashTypes[i].size >> 20);
    else
        printk("SIZE: (%d KB)\n", flashTypes[i].size >> 10);  
#else
    for (i = 0; flash_geom_tbl[i].name != NULL; i++) {
        if ((venid == flash_geom_tbl[i].vendor_id) &&
            (devid == flash_geom_tbl[i].device_id)) {
            break;
        }
    }

    printk("FLASH ID: %s ", flash_geom_tbl[i].name);

    if (flash_geom_tbl[i].size >= 0x100000)
        printk("SIZE: (%d MB)\n", flash_geom_tbl[i].size >> 20);
    else
        printk("SIZE: (%d KB)\n", flash_geom_tbl[i].size >> 10);
#endif
    return i;
}
#ifdef _SC_CODE_
/* helpful FUNCTION	*/
uint get_sNum_by_sAddr(u_int32_t addr)
{
	u_int32_t i = 0;

	if(addr < type_1_num*type_1_size)
	{
		for(;i<type_1_num;i++)
		{
			if((addr>=i*type_1_size)&&
				(addr<(i+1)*type_1_size))
			{
				return i;
			}
		}
	}
	else
	{
		u_int32_t x = addr - type_1_num*type_1_size;

		for(;i<type_2_num;i++)
		{
			if((x>=i*type_2_size)&&
				(x<(i+1)*type_2_size))
			{
				return (i + type_1_num);
			}
		}		
		
	}
	return 0;
}
#endif
static int ar9100_flash_erase(struct mtd_info *mtd, struct erase_info *instr)
{
    int nsect, s_curr, s_last;
 
    if (instr->addr + instr->len > mtd->size) return (-EINVAL);

    ar9100_pflash_down();



#ifdef _SC_CODE_
	s_curr = get_sNum_by_sAddr(instr->addr);
	s_last = get_sNum_by_sAddr(instr->addr + instr->len - 1);
	if(s_curr == s_last)
		s_last ++;
	
	printk("Going to erase from sector %08x to %08x..\n", s_curr, s_last);
#else
    nsect = instr->len/mtd->erasesize;
    if (instr->len % mtd->erasesize)
        nsect ++;

    s_curr = instr->addr/mtd->erasesize;
    s_last  = s_curr + nsect;
#endif

#ifdef _SC_CODE_
    ar9100_pflash_erase((FlashType *)mtd->priv,s_curr, s_last);
#else
    ar9100_pflash_erase((ar9100_flash_geom_t *)mtd->priv,s_curr, s_last);
#endif
    ar9100_pflash_up();

    if (instr->callback) {
        instr->state |= MTD_ERASE_DONE;
        instr->callback(instr);
    }

    return 0;
}

static int
ar9100_flash_read(struct mtd_info *mtd, loff_t from, size_t len,
	  size_t * retlen, u_char * buf)
{
    uint32_t addr = from | AR9100_PFLASH_CTRLR;

    if (!len)
        return (0);
    if (from + len > mtd->size)
        return (-EINVAL);

    ar9100_pflash_down();

    memcpy(buf, (uint8_t *) (addr), len);
    *retlen = len;

    ar9100_pflash_up();

    return 0;
}

static int
ar9100_flash_write(struct mtd_info *mtd, loff_t to, size_t len,
	   size_t * retlen, const u_char * buf)
{


    ar9100_pflash_down();

    if (mtd->size < to + len)
        return ENOSPC;

    to += AR9100_PFLASH_CTRLR;
#ifdef _SC_CODE_
    ar9100_pflash_write_buff((FlashType *)mtd->priv, (u_char *) buf, to, len);
#else
    ar9100_pflash_write_buff((ar9100_flash_geom_t *)mtd->priv, (u_char *) buf, to, len);
#endif
    ar9100_pflash_up();

    *retlen = len;

    return 0;
}


/*
 * sets up flash_info and returns size of FLASH (bytes)
 */
#ifdef _SC_CODE_
static int
__init ar9100_flash_init(void)
{
    int np;

    FlashType *fl_type;

    struct mtd_info *mtd;
    struct mtd_partition *mtd_parts;
    uint8_t index;

    init_MUTEX(&ar9100_flash_sem);

    index = ar9100_flash_probe();
    

    fl_type = &(flashTypes[index]);
    
    /* record flash info for self-use */
    index_num = index;
    type_1_num = fl_type->sectors[0].numberSectors;
    type_2_num = fl_type->sectors[1].numberSectors;
    type_1_size = fl_type->sectors[0].size;
    type_2_size = fl_type->sectors[1].size;    
	/* record end */
	
    /* set flash size to value from bootloader if it passed valid value */
    /* otherwise use the default 4MB.                                   */
    if (__ath_flash_size >= 4 && __ath_flash_size <= 16)
        fl_type->size = __ath_flash_size * 1024 * 1024;

    mtd = kmalloc(sizeof(struct mtd_info), GFP_KERNEL);
    if (!mtd) {
        printk("Cant allocate mtd stuff\n");
        return -1;
    }
    memset(mtd, 0, sizeof(struct mtd_info));

    mtd->name = AR9100_FLASH_NAME;
    mtd->type = MTD_NORFLASH;
    mtd->flags = (MTD_CAP_NORFLASH | MTD_WRITEABLE);
    mtd->size = fl_type->size;
    mtd->erasesize = fl_type->sectors[1].size;	/* Erase block size */ /* thinking we will only erase non-boot area -- tony*/
    mtd->numeraseregions = 0;
    mtd->eraseregions = NULL;
    mtd->owner = THIS_MODULE;
    mtd->erase = ar9100_flash_erase;
    mtd->read = ar9100_flash_read;
    mtd->write = ar9100_flash_write;
    mtd->priv = (void *)(&flashTypes[index]);

    np = parse_mtd_partitions(mtd, part_probes, &mtd_parts, 0);
    if (np > 0) {
        add_mtd_partitions(mtd, mtd_parts, np);
    } else
        printk("No partitions found on flash\n");
    return 0;
}
#else
static int
__init ar9100_flash_init(void)
{
    int np;

    ar9100_flash_geom_t *geom;
    struct mtd_info *mtd;
    struct mtd_partition *mtd_parts;
    uint8_t index;

    init_MUTEX(&ar9100_flash_sem);

    index = ar9100_flash_probe();
    geom = &flash_geom_tbl[index];

    /* set flash size to value from bootloader if it passed valid value */
    /* otherwise use the default 4MB.                                   */
    if (__ath_flash_size >= 4 && __ath_flash_size <= 16)
        geom->size = __ath_flash_size * 1024 * 1024;

    mtd = kmalloc(sizeof(struct mtd_info), GFP_KERNEL);
    if (!mtd) {
        printk("Cant allocate mtd stuff\n");
        return -1;
    }
    memset(mtd, 0, sizeof(struct mtd_info));

    mtd->name = AR9100_FLASH_NAME;
    mtd->type = MTD_NORFLASH;
    mtd->flags = (MTD_CAP_NORFLASH | MTD_WRITEABLE);
    mtd->size = geom->size;
    mtd->erasesize = (geom->sector_size * 16);	/* Erase block size */
    mtd->numeraseregions = 0;
    mtd->eraseregions = NULL;
    mtd->owner = THIS_MODULE;
    mtd->erase = ar9100_flash_erase;
    mtd->read = ar9100_flash_read;
    mtd->write = ar9100_flash_write;
    mtd->priv = (void *)(&flash_geom_tbl[index]);

    np = parse_mtd_partitions(mtd, part_probes, &mtd_parts, 0);
    if (np > 0) {
        add_mtd_partitions(mtd, mtd_parts, np);
    } else
        printk("No partitions found on flash\n");
    return 0;
}
#endif

static void
__exit ar9100_flash_exit(void)
{
    /*
     * nothing to do
     */
}

/*
 * Primitives to implement flash operations
 */

#ifdef _SC_CODE_
static int
ar9100_pflash_write_buff(FlashType *geom, u_char * src, unsigned long addr, unsigned long cnt)
#else
static int
ar9100_pflash_write_buff(ar9100_flash_geom_t *geom, u_char * src, unsigned long addr, unsigned long cnt)
#endif
{
    unsigned long cp, wp, data;
    int i, l, rc;


    wp = (addr & ~3);   /* get lower word aligned address */

    /*
     * handle unaligned start bytes
     */
    if ((l = addr - wp) != 0) {
        data = 0;
        for (i = 0, cp = wp; i < l; ++i, ++cp) {
            data = (data << 8) | (*(unsigned char *) cp);
        }
        for (; i < 4 && cnt > 0; ++i) {
            data = (data << 8) | *src++;
            --cnt;
            ++cp;
        }
        for (; cnt == 0 && i < 4; ++i, ++cp) {
            data = (data << 8) | (*(unsigned char *) cp);
        }

        if ((rc = ar9100_pflash_write_word(geom, wp, data)) != 0) {
            return (rc);
        }
        wp += 4;
    }
    /*
     * handle word aligned part
     *
     */
    while (cnt >= 4) {
        data = 0;
        for (i = 0; i < 4; ++i) {
            data = (data << 8) | *src++;
        }
        if ((rc = ar9100_pflash_write_word(geom, wp, data)) != 0) {
            return (rc);
        }
        wp += 4;
        cnt -= 4;
    }

    if (cnt == 0) {
        return (0);
    }

    /*
     * handle unaligned tail bytes
     */
    data = 0;
    for (i = 0, cp = wp; i < 4 && cnt > 0; ++i, ++cp) {
        data = (data << 8) | *src++;
        --cnt;
    }
    for (; i < 4; ++i, ++cp) {
        data = (data << 8) | (*(unsigned char *) cp);
    }
    return (ar9100_pflash_write_word(geom, wp, data));
}
#ifdef _SC_CODE_
static int ar9100_pflash_write_word(FlashType *geom, unsigned long dest, unsigned long data)
{


    volatile CFG_FLASH_WORD_SIZE *dest2 = (CFG_FLASH_WORD_SIZE *) dest;
    unsigned long data_addr;

    data_addr = (unsigned long) &data;
    CFG_FLASH_WORD_SIZE *data2 = (CFG_FLASH_WORD_SIZE *) data_addr;
    int i;

    /* Check if Flash is (sufficiently) erased */
    if ((*((unsigned long *) dest) & data) != data) {
        return (2);
    }

    for (i = 0; i < 4 / sizeof(CFG_FLASH_WORD_SIZE); i++) {
        CFG_FLASH_WORD_SIZE state, prev_state;
        int timeout;

        ar9100_write(CFG_FLASH_ADDR0, FLASH_Setup_Code1);
        ar9100_write(CFG_FLASH_ADDR1, FLASH_Setup_Code2);
        ar9100_write(CFG_FLASH_ADDR0, FLASH_Program);
        dest2[i] = data2[i];

        if (!strcmp(geom->partname,"MX29LV640DBT")) {
            timeout = 10000000;
            while (timeout) {
                if (dest2[i] == data2[i]) {
                         break;
                }
                timeout--;
            }
        }
        else {
        /*  Wait for completion (bit 6 stops toggling) */
        	timeout = 5000000;
        	prev_state = (dest2[i] & FLASH_Busy);
        	while (timeout) {
            		state = (dest2[i] & FLASH_Busy);
            		if (prev_state == state) {
				break;
            		}
            	timeout--;
            	prev_state = state;
        	}

        	if (!timeout)
            		return -1;
       	    }
     }

    return (0);
}
#else
static int ar9100_pflash_write_word(ar9100_flash_geom_t *geom, unsigned long dest, unsigned long data)
{


    volatile CFG_FLASH_WORD_SIZE *dest2 = (CFG_FLASH_WORD_SIZE *) dest;
    unsigned long data_addr;

    data_addr = (unsigned long) &data;
    CFG_FLASH_WORD_SIZE *data2 = (CFG_FLASH_WORD_SIZE *) data_addr;
    int i;

    /* Check if Flash is (sufficiently) erased */
    if ((*((unsigned long *) dest) & data) != data) {
        return (2);
    }

    for (i = 0; i < 4 / sizeof(CFG_FLASH_WORD_SIZE); i++) {
        CFG_FLASH_WORD_SIZE state, prev_state;
        int timeout;

        ar9100_write(CFG_FLASH_ADDR0, FLASH_Setup_Code1);
        ar9100_write(CFG_FLASH_ADDR1, FLASH_Setup_Code2);
        ar9100_write(CFG_FLASH_ADDR0, FLASH_Program);
        dest2[i] = data2[i];

#if 1
            timeout = 10000000;
            while (timeout) {
                if (dest2[i] == data2[i]) {
                         break;
                }
                timeout--;
            }
#else
        /*  Wait for completion (bit 6 stops toggling) */
        	timeout = 5000000;
        	prev_state = (dest2[i] & FLASH_Busy);
        	while (timeout) {
            		state = (dest2[i] & FLASH_Busy);
            		if (prev_state == state) {
				break;
            		}
            	timeout--;
            	prev_state = state;
        	}
#endif
        	if (!timeout)
            		return -1;
     }

    return (0);
}
#endif
#ifdef _SC_CODE_
static int ar9100_pflash_erase(FlashType *p_type, int s_first, int s_last)
{

    int i;
    int timeout;
    u_int32_t increment = 0;

    for (i = s_first; i < s_last; i++) 
   	{
        CFG_FLASH_WORD_SIZE state, prev_state;

		if(i<type_1_num)
			increment = i*type_1_size;
		else
		{
			increment = type_1_num*type_1_size + (i-type_1_num)*type_2_size;
		}
	 	f_ptr addr_ptr = (f_ptr) (AR9100_PFLASH_CTRLR + increment);

		printk("erasing %08x...\n", addr_ptr);
		
        /* Program data [byte] - 6 step sequence */
        ar9100_write(CFG_FLASH_ADDR0, FLASH_Setup_Code1);
        ar9100_write(CFG_FLASH_ADDR1, FLASH_Setup_Code2);
        ar9100_write(CFG_FLASH_ADDR0, FLASH_Setup_Erase);
        ar9100_write(CFG_FLASH_ADDR0, FLASH_Setup_Code1);
        ar9100_write(CFG_FLASH_ADDR1, FLASH_Setup_Code2);

        *addr_ptr = FLASH_Block_Erase;

  	if (!strcmp(p_type->partname,"MX29LV640DBT")) {
        // Wait for erase completion.
        timeout = 10000000;
        while (timeout) {
        	state = *addr_ptr;
                if (FLASHWORD(0xffff) == state) {
                        break;
                }
                timeout--;
            }
	}
	else {
        /*  Wait for completion (bit 6 stops toggling) */
       		timeout = 5000000;
        	prev_state = (*addr_ptr & FLASH_Busy);
        	while (timeout) {
            		state = (*addr_ptr & FLASH_Busy);
            		if (prev_state == state) {
				break;
            		}
            		timeout--;
            		prev_state = state;
        	}

    	}
        if (!timeout) {
		printk("Erase operation failed\n");
        	return -1;
	}
     }
   	 return 0;
}
#else
static int ar9100_pflash_erase(ar9100_flash_geom_t *geom, int s_first, int s_last)
{

    int i;
    int timeout;

    for (i = s_first; i < s_last; i++) {
        CFG_FLASH_WORD_SIZE state, prev_state,rd_data;

	 f_ptr addr_ptr = (f_ptr) (AR9100_PFLASH_CTRLR + (i * geom->sector_size * 16));

        /* Program data [byte] - 6 step sequence */
        ar9100_write(CFG_FLASH_ADDR0, FLASH_Setup_Code1);
        ar9100_write(CFG_FLASH_ADDR1, FLASH_Setup_Code2);
        ar9100_write(CFG_FLASH_ADDR0, FLASH_Setup_Erase);
        ar9100_write(CFG_FLASH_ADDR0, FLASH_Setup_Code1);
        ar9100_write(CFG_FLASH_ADDR1, FLASH_Setup_Code2);

        *addr_ptr = FLASH_Block_Erase;
#if 1
        // Wait for erase completion.
        timeout = 10000000;
        while (timeout) {
        	state = *addr_ptr;
                if (FLASHWORD(0xffff) == state) {
                        break;
                }
                timeout--;
            }
#else
        /*  Wait for completion (bit 6 stops toggling) */
       		timeout = 5000000;
        	prev_state = (*addr_ptr & FLASH_Busy);
		while (timeout) {
                	rd_data = *addr_ptr;
                	state = rd_data & FLASH_Busy;
                	if ((prev_state == state) && (rd_data == FLASHWORD(0xffff))) {
                        	break;
			}
			timeout--;
                        prev_state = state;
                }
#endif
        if (!timeout) {
		printk("Erase operation failed\n");
        	return -1;
	}
     }
   	 return 0;
}
#endif
module_init(ar9100_flash_init);
module_exit(ar9100_flash_exit);
