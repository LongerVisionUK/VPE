#include <sys/types.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <assert.h>

#include "transcoder.h"

typedef unsigned char		u8;
typedef unsigned short		u16;
typedef unsigned int		u32;

char * ip_name[8] = {
	"L2CACH_VCD",
	"L2CACH_VCE",
	"F1",
	"F2",
	"F3",
	"F4_TCACH",
	"F4_DTRC",
	"F4_L2CACHE",
};

int main(int argc, char **argv)
{
	int dev_fd = -1;
	int ret, i;
	u32 ip_id;
	u32 core_id, reg_id, val[10], ip_size;
	struct ip_desc ip_desc;

	if(argc<6)
	{
		printf("\ncommand line:\n");
		printf("  %s dev ip_id r core reg_id\n",argv[0]);
		printf("  %s dev ip_id w core reg_id val \n",argv[0]);
		printf("  %s dev ip_id pull core reg_id size \n",argv[0]);
		printf("  %s dev ip_id push core reg_id size val..val \n\n",argv[0]);
		return -1;
	}
	dev_fd = open(argv[1], O_RDWR);
	if(dev_fd == -1)
	{
		printf("Failed to open dev: %s\n", argv[1]);
		return -1;
	}
	else
		printf("open /dev/trans_mem succeed.\n");

	ip_id = strtoul(argv[2], 0, 0);
	if (ip_id > 7) {
		printf("ip_id = %d error.\n", ip_id);
		goto end;
	}
	printf("will operate ip_id: %d - %s\n", ip_id, ip_name[ip_id]);

	if(strcmp(argv[3],"r") == 0)
	{
		core_id = strtoul(argv[4], 0, 0);
		reg_id = strtoul(argv[5], 0, 0);

		ip_desc.ip_id = ip_id;
		ip_desc.core.id = core_id;
		ip_desc.core.regs = val;
		ip_desc.core.reg_id = reg_id;

		ret = ioctl(dev_fd,CB_TRANX_MISC_RD_REG,&ip_desc);
		if (ret)
			printf("r failed: core_id:%d reg_id:%d.\n", core_id, reg_id);
		else
			printf("r core_id:%d reg_id:%d val=0x%x.\n", core_id, reg_id, val[0]);
	}
	else if(strcmp(argv[3],"w") == 0)
	{
		core_id = strtoul(argv[4], 0, 0);
		reg_id = strtoul(argv[5], 0, 0);
		val[0] = strtoul(argv[6], 0, 0);

		ip_desc.ip_id = ip_id;
		ip_desc.core.id = core_id;
		ip_desc.core.regs = val;
		ip_desc.core.reg_id = reg_id;

		ret = ioctl(dev_fd,CB_TRANX_MISC_WR_REG,&ip_desc);
		if (ret)
			printf("w failed: core_id:%d reg_id:%d val=0x%x.\n", core_id, reg_id, val[0]);
		else
			printf("w core_id:%d reg_id:%d val=0x%x.\n", core_id, reg_id, val[0]);
	}
	else if(strcmp(argv[3],"pull") == 0)
	{
		core_id = strtoul(argv[4], 0, 0);
		reg_id = strtoul(argv[5], 0, 0);
		ip_size = strtoul(argv[6], 0, 0);

		ip_desc.ip_id = ip_id;
		ip_desc.core.id = core_id;
		ip_desc.core.regs = val;
		ip_desc.core.reg_id = reg_id;
		ip_desc.core.size = ip_size * 4;

		ret = ioctl(dev_fd,CB_TRANX_MISC_PULL_REGS,&ip_desc);
		if (ret)
			printf("pull failed: core_id:%d reg_id:%d.\n", core_id, reg_id);
		else
		{
			printf("pull core_id:%d reg_id:%d ip_size:%d:\n", core_id,reg_id,ip_size);
			for (i=0;i<ip_size;i++)
				printf("  0x%x", val[i]);
			printf("\n");
		}
	}
	else if(strcmp(argv[3],"push") == 0)
	{
		core_id = strtoul(argv[4], 0, 0);
		reg_id = strtoul(argv[5], 0, 0);
		ip_size = strtoul(argv[6], 0, 0);

		for (i=0;i<ip_size;i++)
			val[i] = strtoul(argv[7+i], 0, 0);

		ip_desc.ip_id = ip_id;
		ip_desc.core.id = core_id;
		ip_desc.core.regs = val;
		ip_desc.core.reg_id = reg_id;
		ip_desc.core.size = ip_size * 4;

		ret = ioctl(dev_fd,CB_TRANX_MISC_PUSH_REGS,&ip_desc);
		if (ret)
			printf("push failed: core_id:%d reg_id:%d.\n", core_id, reg_id);
		else
		{
			printf("push core_id:%d reg_id:%d ip_size:%d:\n", core_id,reg_id,ip_size);
			for (i=0;i<ip_size;i++)
				printf("  0x%x", val[i]);
			printf("\n");
		}
	}
	else
		printf("operate name is error.\n");



end:
	close(dev_fd);

	return 0;
}

