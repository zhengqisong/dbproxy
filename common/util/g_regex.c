#include<stdio.h>
#include<sys/types.h>
#include<regex.h>
#include<memory.h>
#include<stdlib.h>
#include "trace.h"
#include "g_regex.h"

int g_regexec_get_value(const char *string, const char *regex, char *dstlist, int dstlen, int maxnum)
{
    int status, i, num = 0;
    int cflags=REG_EXTENDED|REG_NEWLINE;  
    regmatch_t *pmatch;  
    const size_t nmatch = maxnum+1;  
    regex_t reg;
    int p = 0;
    char *list_p = dstlist;
    memset(dstlist,'\0',sizeof(dstlen));
    pmatch = (regmatch_t *)malloc(sizeof(regmatch_t) * nmatch);
    do{
        string = string + p;
        regcomp(&reg,  regex,  cflags);  
        status = regexec(&reg, string, nmatch, pmatch, REG_NOTEOL); 
        if(status == 0){
            i =  0;
            for(i = 1; pmatch[i].rm_so != -1; i++){
              int len = pmatch[i].rm_eo - pmatch[i].rm_so;
              if(len > 0){
                  if(dstlist + dstlen < list_p + len +1){
                     goto error;
                  }
                  memcpy(list_p, string + pmatch[i].rm_so, len);
                  *(list_p + len) = '\0';
                  
                  list_p = list_p + len + 1;
                  p = pmatch[i].rm_so;
                  num++;
                  
                  if(maxnum == num){
                    goto ret;
                  }
              }
            }
        }
    }while(status == 0);
ret:
    regfree(&reg);
    free(pmatch);    
    return num;
error:
    
    regfree(&reg);
    free(pmatch);
    return -1;    
}