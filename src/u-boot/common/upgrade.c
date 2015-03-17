//==========================================================================
//
//      upgrade.c
//
//      upgrade support for Sernet
//
//==========================================================================

#include <common.h>
#include <command.h>
#include <watchdog.h>
#include <malloc.h>
#include <net.h>
#include <flash.h>
#include <exports.h>
#include <upgrade.h>
#include <asm/addrspace.h>

int sc_util = 0;
int upgrade_action = 0;

extern flash_info_t	flash_info[CFG_MAX_FLASH_BANKS];
void delay_1s(void);
static unsigned char eall_flag;
static VCI_TABLE vci;
static DCB_BUF dcb;
//static unsigned short led_value;

unsigned short sc_xchg( unsigned short dwData)
{
    unsigned short temp;
#if 0 
    temp = dwData;
#else
    temp = ((dwData & 0xff) << 8);
    temp |= ((dwData & 0xff00) >> 8);
#endif
    return temp;
}

void dumpData(unsigned char *data, int len)
{
    int i;
    for (i = 0; i < len; i++)
    {
        printf("%02x ", data[i]);
        if ((i+1)%16 == 0)
        {
            printf("\n");
        }
    }
    printf("\n");
}

int PushButton(void)
{
    return (rst_button_pushed());
}

int ProgramChip(unsigned long dlAddress,unsigned char * dbData,unsigned long dlLength)
{
    return flash_write(dbData,dlAddress,dlLength);
}

int FlashDriver(unsigned long dlAddress,unsigned char *dbData,unsigned long dlLength,unsigned long dlFlag)
{
	int dwReturnCode=0;
	//  int i;
	unsigned long addr;
	addr = (FLASH_ADDR_BASE|dlAddress);
	switch (dlFlag) 
	{
		case 0:
			if (eall_flag == 0) 
			{
				if ((dlAddress >=  UPGRADE_START_OFFSET)&&(dlAddress < UPGRADE_END_OFFSET)) 
				{
					dwReturnCode = ProgramChip(addr,dbData, dlLength);
				} 
				else 
				{
					dwReturnCode = 0;
				} /* endif */
			} 
			else 
			{
				if(dlAddress < RF_ADDRESS)
					dwReturnCode = ProgramChip(addr,dbData, dlLength);
				else
					dwReturnCode = 0;
			}
			break;
		case 1:
			break;
		case 2:
			break;
		case 3: //erase all  -- should keep calibration
#ifdef _KEEP_CALIBRATION_
			printf("Erase All : %08x to %08x .\n", FLASH_ADDR_BASE, FLASH_ADDR_BASE + RF_ADDRESS - 1);

			flash_sect_protect(0, FLASH_ADDR_BASE, FLASH_ADDR_BASE + RF_ADDRESS - 1);
			flash_sect_erase(FLASH_ADDR_BASE, FLASH_ADDR_BASE + RF_ADDRESS - 1 );
			

			if ((dlAddress >=  0)&&(dlAddress < RF_ADDRESS))
			{
            	dwReturnCode = ProgramChip(addr,dbData, dlLength);
        	}
        	else
        	{
            	dwReturnCode = 0;
        	} /* endif */
#else 
			printf("Erase All : %08x to %08x .\n", FLASH_ADDR_BASE, FLASH_ADDR_BASE + FLASH_SIZE - 1);

        	flash_sect_protect(0, FLASH_ADDR_BASE, FLASH_ADDR_BASE + FLASH_SIZE - 1);
			flash_sect_erase(FLASH_ADDR_BASE, FLASH_ADDR_BASE + FLASH_SIZE - 1 );


			if ((dlAddress >=  0)&&(dlAddress < FLASH_SIZE))
			{
				dwReturnCode = ProgramChip(addr,dbData, dlLength);
			}
			else
			{
				dwReturnCode = 0;
			} /* endif */
#endif
			eall_flag = 1;
			break;
		case 4://normal erase
			printf("Erase Normal : %08x to %08x .\n", FLASH_ADDR_BASE + UPGRADE_START_OFFSET, FLASH_ADDR_BASE + UPGRADE_END_OFFSET - 1);

			flash_sect_protect(0, FLASH_ADDR_BASE + UPGRADE_START_OFFSET, FLASH_ADDR_BASE + UPGRADE_END_OFFSET - 1);
			flash_sect_erase(FLASH_ADDR_BASE + UPGRADE_START_OFFSET, FLASH_ADDR_BASE + UPGRADE_END_OFFSET - 1);

			
			if ((dlAddress >=  UPGRADE_START_OFFSET)&&(dlAddress < UPGRADE_END_OFFSET)) 
			{
				dwReturnCode = ProgramChip(addr,dbData, dlLength);
			} 
			else 
			{
				dwReturnCode = 0;
			} /* endif */
			eall_flag = 0;
			break;
		case 5:
			if ((dlAddress >=  UPGRADE_START_OFFSET)&&(dlAddress < UPGRADE_END_OFFSET)) 
			{
				dwReturnCode = ProgramChip(addr,dbData, dlLength);
			} 
			else 
			{
				dwReturnCode = 0;
			} /* endif */
			break;
	} /* endswitch */
	return dwReturnCode;
}

/*
 * Handle incoming upgrade packets.
 */
void AssignStart(void)
{
    NetSetTimeout (0, NULL);
    NetSetHandler ((rxhand_f *)(_CODE_RAM_ADDR));

    printf("\nIn Assign...\n");
    return;
}

void DownloadStart(void)
{
    NetSetTimeout (0, NULL);
    NetSetHandler ((rxhand_f *)(_CODE_RAM_ADDR));
    printf("\nIn Download...\n");
    return;
}


int dl_Initialize(void)
{
    DECLARE_GLOBAL_DATA_PTR;
    bd_t *bd = gd->bd;

    eth_initialize (bd);
    /* delay for MIPS LE stuck issue - probably unstable board. ??? */
    printf("\n");
    return 0;
}

void dl_GetAddr(unsigned char *node)
{
    static struct eth_device *dev;
    dev = eth_get_dev();
    memcpy(node,dev->enetaddr,6);
}

int dl_Transmit(char *buf,int len)
{
    return eth_send(buf, len);
}

void reset(void)
{
    /* Reinit board to run initialization code again */
    do_reset();

}

#ifdef _MULTIPLE_ASSIGN_
unsigned char Serial[18];  	 // 0x03ff70		[16 bytes]
unsigned char Domain[3];	   // 0x03ff80		[1 byte]
unsigned char Country[3];	   // 0x03ff81		[1 byte]
unsigned char HW_version[3]; // 0x03ff82		[1 byte]
unsigned char WPS_PIN[10];	 // 0x03ff88		[8 bytes]
#endif

void do_boot(void)
{

    unsigned char *dbSign,*ptr;
    unsigned char temp_str[20];
    unsigned char * env_value;

#if 1
	printf("%s\n", BOOT_VERSION_STRING);
    gpio_init();
#endif

#if 1 /* close now, OPEN this code until h/w ready. */
	if(cold_start())
	{
		printf("cold start!!!\n");
		remove_cold_flag();
		
		if(!cold_start())
			printf("cold_start flag removed.\n");
		else
			printf("cold_start flag remove failed.\n");
	}
	else
		printf("warm start!!!\n");		

#endif

#ifdef _FOR_DDR_ISSUE_
    void rd_wr_ddr(void);
    printf("Entering deadloop for read & write ddr ram...\n");
    while(1)
    {
        rd_wr_ddr();
    }
#endif
    dbSign	= (unsigned char *)(FLASH_ADDR_BASE + SIGN_OFFSET);
    ptr 	= (unsigned char *)(BOOT_ADDR_BASE + NODE_ADDRESS);
    printf("mac in flash:	%02x:%02x:%02x:%02x:%02x:%02x\r\n",ptr[0],ptr[1],ptr[2],ptr[3],ptr[4],ptr[5]);
#if 1 /* mac check & assign */
    if ( (memcmp(ptr,"\x00\x00\x00\x00\x00\x00",6) == 0) ||
            (memcmp(ptr,"\xff\xff\xff\xff\xff\xff",6) == 0) )
    {
        printf("no mac address \r\n");
        do_assign();
    }
    else
    {
        /* update MAC in env */
        sprintf(temp_str,"%02x:%02x:%02x:%02x:%02x:%02x",ptr[0],ptr[1],ptr[2],ptr[3],ptr[4],ptr[5]);
        env_value = getenv("ethaddr");
        printf("mac in env :	%s\n", env_value);
        if (memcmp(temp_str, env_value, strlen(env_value)) != 0)
        {
            setenv("ethaddr",temp_str);
            saveenv();
        }
    }
#endif

    if (memcmp (dbSign,"eRcOmM",6) == 0)
    {
        //printf("have eRcOmM Sign\r\n");
#if 0 
        if(PushButton()==1)
        {
            unsigned long timer;
            printf("if release button,enter download\n");

            timer = get_timer(0);
            while((get_timer(0)- timer) <= 2*CFG_HZ)
            {
                if(PushButton()==0)
                {
                    do_upgrade();
                }
            }
        }//end pushbutton=1
#else
        if(PushButton())
        {
        	printf("button is pushed, enter download mode...\n");
            do_upgrade();
        }
#endif
        return ;
    }
    else
    {
        printf("not have eRcOmM \r\n");
        do_upgrade();
    } /* endif */
}

unsigned char block_buffer[0x10000];

void AssignHWAddress(unsigned char *psBuffer)
{
#ifdef _MULTIPLE_ASSIGN_
/* 
    belkin_assign *pba = (multiple_assign *)psBuffer;
    long sect_addr;
    long sect_size;

#if ASSIGN_TEST
    dump_aspack(pba);
#endif

    sect_addr = flash_get_sector_addr(FLASH_ADDR_BASE + WPS_PIN_ADDRESS);
    sect_size = flash_get_sector_size(FLASH_ADDR_BASE + WPS_PIN_ADDRESS);

    memcpy(block_buffer, (unsigned char *)sect_addr, sect_size);
    memcpy(block_buffer + (NODE_ADDRESS + FLASH_ADDR_BASE - sect_addr), pba->mac, sizeof(pba->mac));
    memcpy(block_buffer + (WPS_PIN_ADDRESS + FLASH_ADDR_BASE - sect_addr), pba->wps_pin, sizeof(pba->wps_pin));
    memcpy(block_buffer + (DOMAIN_ADDRESS + FLASH_ADDR_BASE - sect_addr), &pba->domain, sizeof(pba->domain));
    memcpy(block_buffer + (COUNTRY_ADDRESS + FLASH_ADDR_BASE - sect_addr), &pba->country_code, sizeof(pba->country_code));
    memcpy(block_buffer + (SN_ADDRESS + FLASH_ADDR_BASE - sect_addr), pba->serial, sizeof(pba->serial));

    flash_sect_protect(0, sect_addr, (sect_addr + sect_size - 1));
    flash_sect_erase(sect_addr, (sect_addr + sect_size - 1));
    if (flash_write(block_buffer, sect_addr, sect_size))
    {
        printf("%s %d flash write err\n", __FUNCTION__, __LINE__);
    }
    flash_sect_protect(1, sect_addr, (sect_addr + sect_size - 1));
*/
    long sect_addr;
    long sect_size;

    sect_addr = flash_get_sector_addr(FLASH_ADDR_BASE + NODE_ADDRESS);
    sect_size = flash_get_sector_size(FLASH_ADDR_BASE + NODE_ADDRESS);

    /* copy mac resident sector content into buffer */
    memcpy(block_buffer, (unsigned char *)sect_addr, sect_size);
    /* copy mac into the buffer and position should be ok */
    memcpy(block_buffer + (NODE_ADDRESS + FLASH_ADDR_BASE - sect_addr), psBuffer, 6);
    memcpy(block_buffer + (NODE_ADDRESS + FLASH_ADDR_BASE - sect_addr - 0x30 ), Serial, 16);
    memcpy(block_buffer + (NODE_ADDRESS + FLASH_ADDR_BASE - sect_addr - 0x20 ), Domain, 1);
    memcpy(block_buffer + (NODE_ADDRESS + FLASH_ADDR_BASE - sect_addr - 0x20 +1 ), Country, 1);
    memcpy(block_buffer + (NODE_ADDRESS + FLASH_ADDR_BASE - sect_addr - 0x20 +2 ), HW_version, 1);
    memcpy(block_buffer + (NODE_ADDRESS + FLASH_ADDR_BASE - sect_addr - 0x20 +8 ), WPS_PIN, 8);

    flash_sect_protect(0, sect_addr, (sect_addr + sect_size - 1));
    flash_sect_erase(sect_addr, (sect_addr + sect_size - 1));
#if 0 /* dump some mem to see if it is real empty */
    dumpData(sect_addr, 100);
    dumpData(sect_addr + 0x10000 - 0x90, 0x100);
#endif
    if (flash_write(block_buffer, sect_addr, sect_size))
    {
        printf("%s %d flash write err\n", __FUNCTION__, __LINE__);
    }
#if 0 /* dump some mem to see if it is real empty */
    dumpData(sect_addr, 100);
    dumpData(sect_addr + 0x10000 - 0x90, 0x100);
#endif
    flash_sect_protect(1, sect_addr, (sect_addr + sect_size - 1));
#else
    long sect_addr;
    long sect_size;

    sect_addr = flash_get_sector_addr(FLASH_ADDR_BASE + NODE_ADDRESS);
    sect_size = flash_get_sector_size(FLASH_ADDR_BASE + NODE_ADDRESS);

    /* copy mac resident sector content into buffer */
    memcpy(block_buffer, (unsigned char *)sect_addr, sect_size);
    /* copy mac into the buffer and position should be ok */
    memcpy(block_buffer + (NODE_ADDRESS + FLASH_ADDR_BASE - sect_addr), psBuffer, 6);

    flash_sect_protect(0, sect_addr, (sect_addr + sect_size - 1));
    flash_sect_erase(sect_addr, (sect_addr + sect_size - 1));
#if 0 /* dump some mem to see if it is real empty */
    dumpData(sect_addr, 100);
    dumpData(sect_addr + 0x10000 - 0x90, 0x100);
#endif
    if (flash_write(block_buffer, sect_addr, sect_size))
    {
        printf("%s %d flash write err\n", __FUNCTION__, __LINE__);
    }
#if 0 /* dump some mem to see if it is real empty */
    dumpData(sect_addr, 100);
    dumpData(sect_addr + 0x10000 - 0x90, 0x100);
#endif
    flash_sect_protect(1, sect_addr, (sect_addr + sect_size - 1));
#endif
}

unsigned long flash_get_sector_addr(unsigned long addr)
{
    flash_info_t *info;
    int sect;

    info=&flash_info[0];
    for (sect=0; sect < info->sector_count; ++sect)
    {
        if ((addr >= info->start[sect]) && (addr < info->start[sect+1]))
        {
            return info->start[sect];
        }
    }
    return -1;
}

unsigned long flash_get_sector_size(unsigned long addr)
{
    flash_info_t *info;
    int sect;

    info=&flash_info[0];
    for (sect=0; sect < info->sector_count; ++sect)
    {
        if ((addr >= info->start[sect]) && (addr < info->start[sect+1]))
        {
            return (info->start[sect+1] - info->start[sect]);
        }
    }
    return -1;
}
int do_upgrade (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
    int ret = 0;
    int cnt = 0;
	
	char cmd[200];

    sc_util = 1;

	upgrade_action = 1;

    sprintf(cmd,"cp.b 0x%x 0x%x 0x%x", FLASH_ADDR_BASE + _STANDALONE_OFFSET,
                                                _CODE_RAM_ADDR,
                                                _MAX_STANDALONE_SIZE);

	printf("cmd: %s\n", cmd);
    run_command(cmd,0);

    dl_Initialize();
    do
    {
        ret = NetLoop(DOWNLOAD);
        if(ret == -1 && (!cnt))
        {
            printf("please plug in cable...\n");
            cnt ++;
        }
        delay_1s();
    }
    while(ret == -1);

    sc_util = 0;

    return 0;
}
int do_assign (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
    int ret = 0;
    int cnt = 0;
	char cmd[200];

    sc_util = 1;

	upgrade_action = 0;

    sprintf(cmd,"cp.b 0x%x 0x%x 0x%x", FLASH_ADDR_BASE + _STANDALONE_OFFSET,
                                                _CODE_RAM_ADDR,
                                                _MAX_STANDALONE_SIZE);

	printf("cmd: %s\n", cmd);
    run_command(cmd,0);

    dl_Initialize();
	
    do
    {
        ret = NetLoop(ASSIGN);
        if(ret == -1 && (!cnt))
        {
            printf("please plug in cable...\n");
            cnt ++;
        }
        delay_1s();
    }
    while(ret == -1);

    sc_util = 0;

    return 0;
}
int do_button(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
    int ret = 0 ;

    while(1)
    {
        udelay(10000);
      
        ret = rst_button_pushed();
        if(ret)
            printf("reset button is pushed.\n");
    }
}


#ifdef _FOR_DDR_ISSUE_
void rd_wr_ddr(void)
{
    ulong i = 0;
    ulong size = 0x200000;
    uchar data1 = 0x12;
    uchar data2 = 0x34;
    ulong start_addr = (ulong)(0x80060000);

    while(i<(size/2))
    {
        uchar data;
        if(size%2)
            data = data1;
        else
            data = data2;
        *((uchar*)(start_addr + i)) = data;
        data = *((uchar*)(start_addr + i));
        *((uchar*)(start_addr + size - i)) = data;

        i++;
    }
}
#endif

void delay_1s(void)
{
    int i = 0;
    for(;i<10;i++)
        udelay(100000);
}
