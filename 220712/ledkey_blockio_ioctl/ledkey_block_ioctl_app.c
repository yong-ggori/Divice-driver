#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include "ioctl_block.h"

#define DEVICE_FILENAME  "/dev/ledkey_block"

int main()
{
	ioctl_block_info info={0};
    int dev;
    char buff = 15;
    int ret;
    int key_old = 0;

//  dev = open( DEVICE_FILENAME, O_RDWR|O_NDELAY );
//    dev = open( DEVICE_FILENAME, O_RDWR|O_NONBLOCK );
    dev = open( DEVICE_FILENAME, O_RDWR ); // 옵션이 없으면 default는 block mode이다.
	if(dev<0)
	{
		perror("open()");
		return 1;
	}
    ret = write(dev,&buff,sizeof(buff));
	if(ret < 0)
		perror("write()");
	buff = 0;
	do {
		// blocking 은 대부분 read 함수에서 구현한다.
    	ret = read(dev,&buff,sizeof(buff));
		ioctl(dev, IOCTLBLOCK_INTVAL, &info );
		if(ret < 0)
		{
			perror("read()");
			return 1;
		}
//		if(ret == 0)
//			printf("time out\n");
		ret = ioctl(dev, IOCTLBLOCK_INTVAL, &info);
		printf("intval : %lu %.2f\n", info.intval, info.intval/100.0);
//  		if(buff == 0)	// 스위를 누르지 않으면
//				continue;
		/*폴링 방식 아니기 때문에 위 경우 필요 없음*/
//		if(buff != key_old)
		printf("key_no : %d\n",buff);
		write(dev,&buff,sizeof(buff));
		if(buff == 8)
			break;
//		key_old = buff;
	} while(1);
    close(dev);
    return 0;
}
