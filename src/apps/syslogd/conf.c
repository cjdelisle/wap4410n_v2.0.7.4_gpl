
void strccpy(char *dst,char *src,char c)
{
	if(src==NULL){
		*dst='\0';
		return;
	}
	
	while(*src!=c && *src!='\0')	*dst++=*src++;
	
	*dst='\0';
}

void main()
{
	char tmp[]="test1\ntest2\ntest3\n";
	char tmp[16];
	strccpy(tmp,tmp2,'\n');
	printf("%s\n",tmp);
}
