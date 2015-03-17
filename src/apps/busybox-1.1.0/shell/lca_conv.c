#include "../../libiconv-1.8/include/iconv.h"
#include "lca_conv.h"
#include <stdarg.h>
#include <stdlib.h>
typedef struct CONV_TABLE_S
{
    int country;
    char charset[64];
}CONV_TABLE;

static CONV_TABLE convTable[] = 
{
{840,"CP1252"},
{36,"CP1252"},
{76,"CP1252"},
{156,"CP936"},
{410,"CP949"},
{1702,"CP1252"},
{208,"CP1252"},
{1276,"CP1252"},
{246,"CP1252"},
{250,"CP1252"},
{276,"CP1252"},
{372,"CP1252"},
{380,"CP1252"},
{392,"CP932"},
{528,"CP1252"},
{554,"CP1252"},
{578,"CP1252"},
{724,"CP1252"},
{752,"CP1252"},
{756,"CP1252"},
{826,"CP1252"},
{484,"CP1252"},
{630,"CP1252"},
{1076,"CP1252"}
};



/* type=0:from UTF8 to LAN ;type=1:from LAN to UTF8 ;*/
size_t do_convert( int type,
		char *inbuf,  size_t inbytesleft,
		char *outbuf, size_t outbytesleft)
{
	char* to_ces=NULL;
	char* from_ces=NULL;
	iconv_t cd;
	size_t  ret;
	int tableSize=0,CountryCode=0;
	int i=0;
	CountryCode=apCfgCountryCodeGet();
	tableSize = sizeof(convTable)/sizeof(CONV_TABLE);
	if(type==UTF2LAN){
		from_ces="UTF-8";
	
		for(i=0; i<tableSize; i++)
   		 {
       			 if(CountryCode==convTable[i].country)
        		{
        			to_ces=convTable[i].charset;
        			break;
        		}
   		 }
   		 if(i>=tableSize)
    			return -1;
	}
	else if(type==LAN2UTF){
		to_ces="UTF-8";
		
		for(i=0; i<tableSize; i++)
   		 {
       			 if(CountryCode==convTable[i].country)
        		{
        			from_ces=convTable[i].charset;
        			break;
        		}
   		 }
   		 if(i>=tableSize)
    			return -1;
	}
	cd  = libiconv_open(to_ces, from_ces);
	ret = libiconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
	
	libiconv_close(cd);
	return ret;
}

