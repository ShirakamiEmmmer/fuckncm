#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "otherFun/base64.h"
#include "myfun.h"

int main() {
    const unsigned char firstKey[] = {0x68, 0x7A, 0x48, 0x52, 0x41, 0x6D, 0x73, 0x6F,
                                      0x35, 0x6B, 0x49, 0x6E, 0x62, 0x61, 0x78, 0x57};
    const unsigned char secondKey[] = {0x23, 0x31, 0x34, 0x6C, 0x6A, 0x6B, 0x5F, 0x21,
                                       0x5C, 0x5D, 0x26, 0x30, 0x55, 0x3C, 0x27, 0x28};
    //两段从天上掉下来的密钥
    FILE *aSong = NULL;
    FILE *newSong = NULL;
    FILE *newJson = NULL;
    FILE *cover = NULL;
    FILE *rc4 = NULL;


    aSong = fopen("tmp/new.ncm", "rb");

    remove("tmp/rc4");
    remove("tmp/cover.jpeg");
    remove("tmp/tmp.out");
    remove("tmp/tmp.json");

    newSong = fopen("tmp/tmp.out", "wb");

    newJson = fopen("tmp/tmp.json", "wb");

    cover = fopen("tmp/cover.jpeg", "wb");

    rc4 = fopen("tmp/rc4", "wb");

    unsigned char buffer[9];
    fread(buffer, 8, 1, aSong);
    buffer[8] = '\0';
    if (memcmp(buffer, "CTENFDAM", 8) != 0) { exit(1); }//不是ncm

    //读取key的长度和数据
    int keyLength = 0;
    fseek(aSong, 2, SEEK_CUR);
    fread(&keyLength, sizeof(int), 1, aSong);
    unsigned char *keyData = (unsigned char *) malloc(keyLength + 1);
    keyData[keyLength] = '\0';
    fread(keyData, keyLength, 1, aSong);
    for (int i = 0; i < keyLength; ++i) { keyData[i] ^= 0x64; }
    //在解密前对key做异或运算，原理未知

    unsigned char messageOfKey[keyLength];
    unsigned char rc4Key[keyLength];
    //开始第一部分的解密，还原key之后用于还原媒体文件
    AES_Decryption(keyData, firstKey, keyLength, messageOfKey);
    for (int k = 17; messageOfKey[k - 2] != 'L' || messageOfKey[k - 1] != 'b'; ++k) {
        fwrite(messageOfKey + k, 1, 1, rc4);
        rc4Key[k - 17] = messageOfKey[k];
    }
    fseek(rc4, 0, SEEK_END);


    //第二部分的解密，用的是RC4的算法，通过rc4Key计算keyBox
    unsigned char keyBox[256];
    RC4_Decryption(rc4Key, ftell(rc4), keyBox);

    //回到文件继续读取meta，其中包含了歌曲的主要信息
    int metaLength = 0;
    fread(&metaLength, sizeof(int), 1, aSong);
    unsigned char *metaData = (unsigned char *) malloc(metaLength);
    fread(metaData, metaLength, 1, aSong);
    for (int i = 0; i < metaLength; ++i) { metaData[i] ^= 0x63; }
    //解密前进行异或运算，原理未知

    unsigned char metaData2[metaLength - 22];
    unsigned char messageOfMeta[metaLength];
    base64_decode(metaData + 22, metaLength - 22, metaData2);
    AES_Decryption(metaData2, secondKey, metaLength, messageOfMeta);
    //解密并且进行输出，之后有精力会考虑加入json字典
    for (int j = 6;; ++j) {
        fwrite(messageOfMeta + j, 1, 1, newJson);
        if (messageOfMeta[j] == 0x7D) { break; }
    }

    //获得歌曲的封面信息
    fseek(aSong, 9, SEEK_CUR);
    int imageSize = 0;
    fread(&imageSize, sizeof(int), 1, aSong);
    unsigned char imageData[imageSize];
    fread(&imageData, imageSize, 1, aSong);
    fwrite(&imageData, imageSize, 1, cover);
    //最重要的部分，对文件进行解密还原出mp3文件

    audioDecoding(aSong, newSong, keyBox);
    changeName(newSong, newJson);

    free(keyData);
    free(metaData);
    keyData = NULL;
    metaData = NULL;
    return 0;
}