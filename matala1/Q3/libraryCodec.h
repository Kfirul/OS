#ifndef LIBRARYCODEC_H

#define LIBRARYCODEC_H

extern char globalKey[62];

extern char keyEncode[62];

void *createCodec(char key[62]);

int encode(char *textin, char *textout, int len, void *codec);

int decode(char *textin, char *textout, int len, void *codec);

void freeCodec(void *codec);

int findCharacterIndex(char textChar, char *key);


#endif