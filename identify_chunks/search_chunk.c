#include<stdio.h>
#include<zlib.h>
#include<openssl/sha.h>
#include<string.h>
int main(void){
    uint8_t SHA1_digest[20];
    SHA1("hello", 5 ,SHA1_digest);
    
    printf("the sha1 calue is %x\n", SHA1_digest);
    return 0;
}