#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uuid/uuid.h>

//centos: yum install libuuid-devel
//gcc xxx -luuid
//cat /proc/sys/kernel/random/uuid 

char *random_uuid( char buf[37] )
{
    uuid_t uuid;
    char str[36];
    char str2[33];
    int i=0,j=0;
    
    uuid_generate_time_safe(uuid);
    uuid_unparse(uuid, str);
    
    for(i=0; i<strlen(str);i++){
        if(str[i]!='-'){
           buf[j]=str[i];
           j++;
        }
    }
    return buf;
}