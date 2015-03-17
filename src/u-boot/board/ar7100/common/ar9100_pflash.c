#include <common.h>
#include <jffs2/jffs2.h>
#include <asm/addrspace.h>
#include <asm/types.h>
#include "ar7100_soc.h"

#include "sc_config.h"
//----------------------------------------------------------------------------
// Common device details.
#define FLASHWORD(_x)			((CFG_FLASH_WORD_SIZE)(_x))

#define FLASH_Read_ID                   FLASHWORD( 0x90 )
#define FLASH_Read_ID_Exit              FLASHWORD( 0xF0 )
#define FLASH_Reset                     FLASHWORD( 0xFF )
#define FLASH_Program                   FLASHWORD( 0xA0 )
#define FLASH_Block_Erase               FLASHWORD( 0x30 )

#define FLASH_Data                      FLASHWORD( 0x80 )	// Data complement
#define FLASH_Busy                      FLASHWORD( 0x44 )	// "Toggle" bit
#define FLASH_Err                       FLASHWORD( 0x20 )
#define FLASH_Sector_Erase_Timer        FLASHWORD( 0x08 )

#define FLASH_Setup_Code1               FLASHWORD( 0xAA )
#define FLASH_Setup_Code2               FLASHWORD( 0x55 )
#define FLASH_Setup_Erase               FLASHWORD( 0x80 )
#define AMD_SPANSION			0x227e
#define VENDOR_ID(x)			(ushort) ((x & 0x00ff0000) >> 16)
#define DEVICE_ID(x)			(ushort) (x & 0x0000ffff)

/*
 * globals
 */
flash_info_t flash_info[CFG_MAX_FLASH_BANKS];

#ifdef _SC_CODE_
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
    FlashSector     sectors [NUMBER_DIFFERENT_SECTOR_SIZES];
} FlashType;
static const FlashType   flashTypes[] =
    {
        { S29AL032D_CHIP_ID,  	4096UL * 1024UL, { {65536UL, 64UL},  {0UL, 0UL},      {0UL, 0UL}, {0UL, 0UL}}},
        { AM29LV320DB_CHIP_ID, 4096UL * 1024UL, { {8192UL, 8UL},    {65536UL, 63UL}, {0UL, 0UL}, {0UL, 0UL}}},
        { MX29LV320B_CHIP_ID, 	4096UL * 1024UL, { {8192UL, 8UL},    {65536UL, 63UL}, {0UL, 0UL}, {0UL, 0UL}}},
        { MX29LV640BTT_CHIP_ID,8192UL * 1024UL, { {8192UL, 8UL},    {65536UL, 127UL},{0UL, 0UL}, {0UL, 0UL}}},
        { MX29LV640DBT_CHIP_ID,8192UL * 1024UL, { {8192UL, 8UL},    {65536UL, 127UL},{0UL, 0UL}, {0UL, 0UL}}},
        { UNKNOWN_CHIP_ID,     0UL,             { {0UL, 0UL},       {0UL, 0UL},      {0UL, 0UL}, {0UL, 0UL}}}
    };
#define NUMBER_FLASH_TYPES  ((sizeof (flashTypes) / sizeof (flashTypes [0])) - 1)
#else
typedef struct ar9100_flash_geom {
    ushort vendor_id;
    ushort device_id;
    char *name;
    ulong nsectors;
    ulong sector_size;
    ulong size;
} ar9100_flash_geom_t;

ar9100_flash_geom_t flash_geom_tbl[] = {
                                           {0x00bf, 0x2780, "SST-39VF400", 8, 0x01000, 0x080000},	/* 512 KB */
                                           {0x00bf, 0x2782, "SST-39VF160", 32, 0x01000, 0x200000},	/* 2 MB   */
                                           {0x00bf, 0x236b, "SST-39VF6401", 128, 0x01000, 0x800000},	/* 8 MB   */
                                           {0x00bf, 0x236a, "SST-39VF6402", 128, 0x01000, 0x800000},	/* 8 MB   */
                                           {0x00bf, 0x236d, "SST-39VF6402", 128, 0x01000, 0x800000},	/* 8 MB   */
                                           {0x0001, 0x227e, "AMD-SPANSION", 128, 0x02000, 0x1000000},	/* 16 MB  */
#if 1 /* anonymous add support for mxic flash */
                                           {0x00c2, 0x22a8, "MXIC-29LV320CBTC", 64, 0x01000, 0x400000},	/* 4 MB - hack it has no small block  */
                                           {0x00c2, 0x22c9, "MXIC-29LV640BTTC", 128, 0x01000, 0x800000},	/* 8 MB   */
#endif
                                           {0xffff, 0xffff, NULL, 0, 0}	/* end list */
                                       };
#endif


/*
 * statics
 */

static int write_word(flash_info_t * info, ulong dest, ulong data);
static ulong read_id(void);

#ifdef _SC_CODE_
unsigned long flash_init(void)
{
    ushort venid, devid;
    ushort chip_id;

    FlashType *fl_type;
    int nsectors = 0;

    int i;
    int r1,r2,r3,r4;

    flash_info->flash_id = read_id();
    venid = VENDOR_ID(flash_info->flash_id);
    devid = DEVICE_ID(flash_info->flash_id);
    chip_id = ((venid<<8)&0xff00)|(devid&0xff);

#if 0 /* anonymous debug */
    debug("VENDOR_ID(%08x)\n", venid);
    debug("DEVICE_ID(%08x)\n", devid);
    debug("CHIP_ID(%08x)\n", chip_id);
#endif	

    for (i = 0; i < NUMBER_FLASH_TYPES; i++)
    {
        if (chip_id == flashTypes[i].id) {
            break;
        }
    }
    {
        int ii = 0;
        for(;ii<NUMBER_DIFFERENT_SECTOR_SIZES; ii++)
        {
            nsectors += flashTypes[i].sectors[ii].numberSectors;
        }
    }
    fl_type = &(flashTypes[i]);
    if (fl_type->id == UNKNOWN_CHIP_ID) {
        debug("Unknown flash device\n");
        return -1;
    }
    flash_info->size = fl_type->size;	/* bytes */
    flash_info->sector_count = nsectors;


    for (i = 0; i < flash_info->sector_count; i++)
    {
        if(i==0)
            flash_info->start[i] = CFG_FLASH_BASE;
        else
        {
            uint increase;
            if(i<=fl_type->sectors[0].numberSectors)
                increase = fl_type->sectors[0].size;
            else
                increase = fl_type->sectors[1].size;
            flash_info->start[i] = flash_info->start[i-1] + increase;
        }
#if 0 	
        printf("%d : start(%08x)\n", i, flash_info->start[i]);
#endif		
        flash_info->protect[i] = 0;
    }
    flash_print_info(flash_info);
    return (flash_info->size);
}
#else
unsigned long flash_init(void)
{
    ulong sector_size;
    ar9100_flash_geom_t *geom;
    ushort venid, devid;

    int i;

    flash_info->flash_id = read_id();
    venid = VENDOR_ID(flash_info->flash_id);
    devid = DEVICE_ID(flash_info->flash_id);

    for (i = 0; flash_geom_tbl[i].name != NULL; i++) {
        if (venid == flash_geom_tbl[i].vendor_id &&
                devid == flash_geom_tbl[i].device_id) {
            break;
        }
    }

    geom = &flash_geom_tbl[i];
    if (geom->name == NULL) {
        printf("Unknown flash device\n");
        return -1;
    }
    flash_info->size = geom->size;	/* bytes */
    flash_info->sector_count = geom->nsectors;
    sector_size = (geom->sector_size * 16); /* One Erase block size */

    for (i = 0; i < flash_info->sector_count; i++) {
        flash_info->start[i] = CFG_FLASH_BASE + (i * sector_size);
        flash_info->protect[i] = 0;
    }
    flash_print_info(flash_info);

    return (flash_info->size);
}
#endif

void flash_print_info(flash_info_t * info)
{
    printf("Name: ");
    switch (DEVICE_ID(info->flash_id))
    {
    case 0x2780: printf ("SST-39VF400");            break;
    case 0x2782: printf ("SST-39VF160");            break;
    case 0x236b: printf ("SST-39VF6401");           break;
    case 0x236a: printf ("SST-39VF6402");           break;
    case 0x236d: printf ("SST-39VF6402");           break;
    case 0x227e: printf ("AMD-SPANSION");           break;
#ifdef _SC_CODE_ /* anonymous add */
    case 0x22a8: printf ("MXIC-29LV320CBTC");		break;
    case 0x22cb: printf ("MXIC-29LV640DBTC");		break;
#endif
    default:     printf ("Unknown Vendor ");        break;
    }

    printf(" Flash id: 0x%X, Size: %lu bytes.\n", info->flash_id, info->size);
}
#ifdef _SC_LED_BLINK_SUPPOER_
static int time_compare = 0;
static int flag = 0;
static int power_led_status = LED_ON;
#endif
int flash_erase(flash_info_t * info, int s_first, int s_last)
{
    int i = s_first;
    volatile CFG_FLASH_WORD_SIZE *ROM =
        (volatile CFG_FLASH_WORD_SIZE *) (info->start[0]);
    int timeout;
#ifdef _SC_LED_BLINK_SUPPOER_
	int time_now;
	int freq = CFG_HZ/2;
#endif

    printf("First %#x last %#x\n", s_first, s_last);

    for (i = s_first; i <= s_last; i++) 
    {
#ifdef _SC_LED_BLINK_SUPPOER_	
	if(!flag)
	{
		time_compare = get_ticks();
		flag = 1;
	}
	else
	{
		time_now = get_ticks();
		
		if((time_now - time_compare)/freq >= 1)
		{
			if(power_led_status == LED_ON)
			{
				Led_Power(LED_OFF);
				Led_Wlan(LED_ON);
				power_led_status = LED_OFF;
			}
			else
			{
				Led_Power(LED_ON);
				Led_Wlan(LED_OFF);
				power_led_status = LED_ON;
			}
			time_compare = time_now;			
		}
	}
#endif  
        CFG_FLASH_WORD_SIZE state, prev_state, rd_data;
        volatile CFG_FLASH_WORD_SIZE *addr_ptr =
            (volatile CFG_FLASH_WORD_SIZE *) (info->start[i]);

        // Program data [byte] - 6 step sequence
        ROM[CFG_FLASH_ADDR0] = FLASH_Setup_Code1;
        ROM[CFG_FLASH_ADDR1] = FLASH_Setup_Code2;
        ROM[CFG_FLASH_ADDR0] = FLASH_Setup_Erase;
        ROM[CFG_FLASH_ADDR0] = FLASH_Setup_Code1;
        ROM[CFG_FLASH_ADDR1] = FLASH_Setup_Code2;
        *addr_ptr = FLASH_Block_Erase;

        printf("\b\b\b\b\b %3d\%",(( ((i-s_first) + 1) * 100) / ((s_last-s_first) + 1 )) );

        if (DEVICE_ID(flash_info->flash_id) == AMD_SPANSION) {
            // Wait for erase completion.
            timeout = 10000000;
            while (timeout) {
                state = *addr_ptr;
                if (FLASHWORD(0xffff) == state) {
                    break;
                }
                timeout--;
            }
        } else {
            // Wait for completion (bit 6 stops toggling)
            timeout = 5000000;
            prev_state = *addr_ptr & FLASH_Busy;

            while (timeout) {
                rd_data = *addr_ptr;
                state = rd_data & FLASH_Busy;
                if ((prev_state == state) && (rd_data == FLASHWORD(0xffff))) {
                    break;
                }
                timeout--;
                prev_state = state;
            }
        }
        if (!timeout){
            printf("Error erasing flash...\n");
            return -1;
        }
    }
    printf("\n");

    return 0;
}

/*
 * Copy memory to flash, returns:
 * Assumption: Caller has already erased the appropriate sectors.
 * 0 - OK
 * 1 - write timeout
 */
int write_buff(flash_info_t * info, uchar * src, ulong addr, ulong cnt)
{
    ulong cp, wp, data;
    int i, l, rc, j=0, count;

    wp = (addr & ~3);   /* get lower word aligned address */
    count = cnt;
#ifndef _SC_CODE_
    printf("\n");
#else
#endif
    /*
     * handle unaligned start bytes
     */
    if ((l = addr - wp) != 0)
    {
        data = 0;
        for (i = 0, cp = wp; i < l; ++i, ++cp)
        {
            data = (data << 8) | (*(uchar *) cp);
        }
        for (; i < 4 && cnt > 0; ++i)
        {
            data = (data << 8) | *src++;
            --cnt;
            ++cp;
        }
        for (; cnt == 0 && i < 4; ++i, ++cp)
        {
            data = (data << 8) | (*(uchar *) cp);
        }

        if ((rc = write_word(info, wp, data)) != 0)
        {
            return (rc);
        }
        j = j + 4;
        wp += 4;
    }

    /*
     * handle word aligned part
     */
    while (cnt >= 4)
    {
        data = 0;
        for (i = 0; i < 4; i++)
        {
            data = (data << 8) | (*src);
            src++;
        }
#if 0 
        printf("b(%08x)\t", data);
#endif
#ifndef _SC_CODE_
        if(!(j%1024))
            printf("\b\b\b\b\b %3d\%",((j * 100) / count));
#endif
        if ((rc = write_word(info, wp, data)) != 0)
        {
            return (rc);
        }
        wp += 4;
        cnt -= 4;
        j += 4;
    }
#ifndef _SC_CODE_
    printf("\b\b\b\b\b %3d\%\n",(((j + 4) * 100 ) / count));
#endif

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
        data = (data << 8) | (*(uchar *) cp);
    }
    return (write_word(info, wp, data));
}

static int write_word(flash_info_t * info, ulong dest, ulong data)
{
    volatile CFG_FLASH_WORD_SIZE *ROM = (CFG_FLASH_WORD_SIZE *) (info->start[0]);
    volatile CFG_FLASH_WORD_SIZE *dest2 = (CFG_FLASH_WORD_SIZE *) dest;
    CFG_FLASH_WORD_SIZE *data2 = (CFG_FLASH_WORD_SIZE *) & data;
    int i;


    /* Check if Flash is (sufficiently) erased */
    if ((*((vu_long *) dest) & data) != data) {
        return (2);
    }

    for (i = 0; i < 4 / sizeof(CFG_FLASH_WORD_SIZE); i++)
    {
        CFG_FLASH_WORD_SIZE state, prev_state;
        int timeout;

        ROM[CFG_FLASH_ADDR0] = FLASH_Setup_Code1;
        ROM[CFG_FLASH_ADDR1] = FLASH_Setup_Code2;
        ROM[CFG_FLASH_ADDR0] = FLASH_Program;
        dest2[i] = data2[i];

#if 0 
        printf("\t(%08x)\n.", data2[i]);
#endif
#if 0
		if (DEVICE_ID(flash_info->flash_id) == AMD_SPANSION)
#else 
        if (1)
#endif
        {
            timeout = 10000000;
            while (timeout)
            {
                if (dest2[i] == data2[i])
                {
                    break;
                }
                timeout--;
            }
        }
        else
        {
            // Wait for completion (bit 6 stops toggling)
            timeout = 5000000;
            prev_state = dest2[i] & FLASH_Busy;
            while (timeout)
            {
                state = dest2[i] & FLASH_Busy;
                if (prev_state == state)
                {
                    break;
                }
                timeout--;
                prev_state = state;
            }
        }
        if (!timeout)
        {
            printf("Error while Writing into flash...\n");
            return -1;
        }
    }

    return (0);
}

static ulong read_id()
{
    volatile CFG_FLASH_WORD_SIZE *ROM;
    ulong id;
#if 0 /* anonymous debug */
    debug("reading flash id...\n");
#endif

    ROM = (volatile CFG_FLASH_WORD_SIZE *) CFG_FLASH_BASE;


    ROM[CFG_FLASH_ADDR0] = FLASH_Setup_Code1;
    ROM[CFG_FLASH_ADDR1] = FLASH_Setup_Code2;
    ROM[CFG_FLASH_ADDR0] = FLASH_Read_ID;

    udelay(10000);

    id = ROM[0];
    id = (id << 16) | (ROM[1] & 0xffff);

    ROM[CFG_FLASH_ADDR0] = FLASH_Setup_Code1;
    ROM[CFG_FLASH_ADDR1] = FLASH_Setup_Code2;
    ROM[CFG_FLASH_ADDR0] = FLASH_Read_ID_Exit;

#if 0/* anonymous debug */
    debug("id = %08x \n", id);
#endif
    udelay(10000);
    return id;
}
