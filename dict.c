  /*
  * =====================================================================================
  *       Filename:  dict.c
  *    Description:  Little C program which can be used to look up words in linux console
  *				  :  now only support Collins.bgl, NewOxford.bgl, it does not depend only 
  *				  :  on standard C libary, it's shamble more or less, but small is beautiful
  *				  :  fulfilment is a matter of time.
  *        Version:  0.1
  *        Created:  11/30/2015 01:35:37 AM
  *       Revision:  0.2
  *       Compiler:  gcc (Ubuntu 4.8.4-2ubuntu1~14.04) 4.8.4
  *         Author:  Copyright (C) 2015 Sui Jingfeng (moon), Jingfengsui@gmail.com    
  *        Company:  CASIA£¨2014 ~ 2017£©
  *
  *        ALL RIGHT RESERVED
  * =====================================================================================
  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include "dict.h"

#define PERMS	0666
//#define DEBUG	1
static long DictIndex = 0;
struct Dict{
	char* word;
	unsigned int defStarPos;
	unsigned int defLenth;
};

#define COLLINS_V3
#define NEW_OXFORD	1
#define COLLINS_V3_SIZE	33330
#define NEW_OXFORD_SIZE	91330
#define DICT_HEADWORD_SIZE	NEW_OXFORD_SIZE	

struct Dict *pTerm[NEW_OXFORD_SIZE];

int WriteToFile(char *pFilename, char *pData) // WRITE the Data pointed by pData in the RAM to FILE who's name stored at pFilename.  
{
	FILE* fd;
	if((fd = fopen(pFilename, "w")) == NULL)
	{
		perror("open HeadWord:");
		return -1;
	}
	if(fwrite(pData, 1 ,strlen(pData), fd) != strlen(pData))
	{
		perror("write to HeadWord");
		return -2;
	}		
	fclose(fd);
	return 0;
}

int WriteHeadWordToFile(char *pFile)
{
	FILE* fdHeadWord;
	int i;
	if((fdHeadWord = fopen(pFile, "w")) == NULL)
	{
		perror("open HeadWord:");
		return -1;
	}
	for(i=0; i<DictIndex; i++)
		if(fwrite(pTerm[i]->word,1,strlen(pTerm[i]->word),fdHeadWord) != strlen(pTerm[i]->word))
		{
			perror("write to HeadWord");
			return -1;
		}		
		else
			fwrite((char *)"\n", 1, 1, fdHeadWord);
	fclose(fdHeadWord);
	return 0;
}

int main(int argc, char *argv[])
{
	char hex2char(unsigned char dat);
	void PrintTerm();
	void lookup(FILE *fDict);
	void quicksort(struct Dict *v[], int left, int right, int (*cmp)(const char*, const char *));
	int WriteHeadWordToFile(char *pFile);

	char resname[64];
	char pngBuf[BUFSIZ];
	unsigned char hdr, high_nibble,low_nibble,lenbyte;
	char specifier[1024];
	int i, n, record_length;
	int Id3;
	int Charset;
	FILE *fpin;

	unsigned int curpos, datapos;
	if (argc != 2)
	{
		printf("usage: %s uncompressed_filename\n", argv[0]);
		return -1;
	}

	if((fpin = fopen(argv[1], "r")) == NULL) 
	{
		printf("exit due to error %d: %s\n", errno, strerror(errno));
		return -1;
	}
/*
    if((Id3 = open("IdSpecifier3", O_RDWR, 0) < 0))
	{
		printf("open Id3 error!");
		return -1;
	}
    	close(Id3);	
*/		
	while((low_nibble != 5) && (low_nibble != 4)) // a record per loop
	{
		curpos = ftell(fpin); 
		fread(&hdr, 1, 1, fpin);	// get the record size, record in bytes.
		
		high_nibble = hdr >> 4;
		low_nibble = hdr & 0x0F;
		
		if(high_nibble >= 4) 
			record_length = high_nibble - 4; //that is length
		else								 // the lenght stored in the following bytes
			for(i = record_length = 0; i<high_nibble+1; i++) 
			{
				record_length <<= 8;
				fread(&lenbyte, 1, 1, fpin);
				record_length |= lenbyte;
			}

		datapos = ftell(fpin);
	
		switch(low_nibble) // low nibble
		{  
			case 0:// one-byte specifier follows
			{
				fread(specifier, 1, record_length, fpin);
				specifier[record_length]='\0';
          		if(specifier[0] == 7)
          		{
					WriteToFile("Specifier0_7", &specifier[1]);// save those data for analyse
          		}
          		else if(specifier[0] == 8)// source char set
          		{
            		Charset = (unsigned int)specifier[1] - 65;
            		printf("specifier[0]=%d => Source char set: %s", specifier[0], bgl_charset[Charset]);
        		}
        		else
        		{
   					printf("specifier[0]=%d,", specifier[0]); // specifier[0]:specifier type
   					printf("data: ");
          			for(i = 1; i < record_length;i++) //   	 specifier[1:length-1]: data 
          				printf("%x ", specifier[i]);
        		}
        		printf("\n");							
				break;
			}	
/*			case 3:  // two-bytes specifier follows
			{
				fread(specifier, 1, record_length, fpin);// specifier[0]
				specifier[record_length] = '\0';			 
				
				switch(specifier[1])
				{
					case 1://title
					{
						if(write(Id3, bgl_info[0], strlen(bgl_info[0])) != strlen(bgl_info[0]))
							printf("error write Title to IdSpecifier");
						if((n = write(Id3, &specifier[2], record_length-2)) != record_length-2)
							printf("case1: Write to IdSpecifier error!");
						write(Id3, (char *)("\n"), 1);	
					}break;
					
					case 2://author
					{
						if(write(Id3, bgl_info[1], strlen(bgl_info[1])) != strlen(bgl_info[1]))
							printf("Error write Author to IdSpecifie");
						if((n = write(Id3, &specifier[2], record_length-2)) != record_length-2)
							printf("case2: Write to Idspecifier error!");
						write(Id3, (char *)("\n"), 1);	
					}break;
					
					case 3://email
					{
						write(Id3, bgl_info[2], strlen(bgl_info[2]));
						if((n = write(Id3, &specifier[2], record_length-2)) != record_length-2)
							printf("case3: Write to Idspecifier error!");
						write(Id3, (char *)("\n"), 1);	
					}break;

					case 4://copyright
					{
						write(Id3, bgl_info[3], strlen(bgl_info[3]));
						if((n = write(Id3, &specifier[2], record_length-2)) != record_length-2)
							printf("case4: Write to Idspecifier error!");
						write(Id3, (char *)("\n"), 1);	
					}break;

					case 7:// source language
					{
						write(Id3, bgl_info[9], strlen(bgl_info[9]));
						write(Id3, bgl_language[(unsigned int)specifier[5]], strlen(bgl_language[(unsigned int)specifier[5]]));	
						write(Id3, (char *)("\n"), 1);		
					}break;
					
					case 8:// target language
					{
						write(Id3, bgl_info[10], strlen(bgl_info[10]));
						write(Id3, bgl_language[(unsigned int)specifier[5]], strlen(bgl_language[(unsigned int)specifier[5]]));
						write(Id3, (char *)("\n"), 1);	
					}break;
					
					case 9://description
					{
						write(Id3, bgl_info[6], strlen(bgl_info[6]));
						if((n = write(Id3, &specifier[2], record_length-2)) != record_length-2)
							printf("case9: Write to Idspecifier error!");
						write(Id3, (char *)("\n"), 1);	
					}break;

					case 11://window icon 
					{
						resname[0]='s';	resname[1]='p';
						resname[2]='e';	resname[3]='c';	
						resname[4]=hex2char(specifier[0] >> 4);
						resname[5]=hex2char(specifier[0] & 0x0f);
						resname[6]=hex2char(specifier[1] >> 4);
						resname[7]=hex2char(specifier[1] & 0x0f);
						resname[8]='.';
						resname[9]='i';
						resname[10]='c';
						resname[11]='o';
						resname[12]='\0';
						
						WriteToFile(resname, &specifier[2]);
					}break;
					
					case 17:// is UTF-8 encoding?
					{
						n = write(Id3, bgl_info[11], strlen(bgl_info[11]));
						if(n != strlen(bgl_info[11]))
						{
							printf("case17:");
						}
						if(record_length >= 5)
						{
							if(specifier[4] == 0x80) 
								write(Id3, (char *)("YES \n"), 5);
							else
								write(Id3, (char *)("No \n"), 4);	
						}
					}break;

					case 26:// Source charset
					{
						if(write(Id3, bgl_info[7], strlen(bgl_info[7])) != strlen(bgl_info[7]))
						{
							perror("case26:write");
						}
						i = (int)(specifier[2] - 'A');
						if((n = write(Id3, bgl_charset[i], strlen(bgl_charset[i]))) 
														!= strlen(bgl_charset[i]))
							printf("case26: Write to Idspecifier error!");
						write(Id3, (char *)("\n"), 1);			
					}break;
					
					case 27:// Target charset
					{
						write(Id3, bgl_info[8], strlen(bgl_info[8]));
						i = (int)(specifier[2] - 'A');
						if((n = write(Id3, bgl_charset[i],strlen(bgl_charset[i]))) != strlen(bgl_charset[i]))
							printf("case27:Write to Idspecifier error!");
						write(Id3, (char *)("\n"), 1);	
					}break;
				}	
			}break;
*/
			case 4:  break;// no specifier
			case 6:  break;
			case 2:  // named resource
			{
				fread(&lenbyte, 1, 1, fpin);
				fread(resname, 1, lenbyte, fpin);
				resname[lenbyte] = '\0';
				fread(pngBuf ,1, (record_length-lenbyte), fpin);
				pngBuf[record_length-lenbyte] = '\0';
				WriteToFile(resname, pngBuf);
			}break;

			case 1:  // entry
			case 10:
			{	
				fread(&lenbyte, 1, 1, fpin);
				pTerm[DictIndex] = (struct Dict *)malloc(sizeof(struct Dict *));
				
				pTerm[DictIndex]->word = (char *)malloc(lenbyte+1);// headword
				fread(pTerm[DictIndex]->word, sizeof(char), lenbyte, fpin);
				*(pTerm[DictIndex]->word + lenbyte) = '\0';
					
				fread(&lenbyte, sizeof(char), 1, fpin);// read definition length high bytes
				pTerm[DictIndex]->defLenth = lenbyte << 8;
				fread(&lenbyte, sizeof(char), 1, fpin);// read defination low bytes
				pTerm[DictIndex]->defLenth += lenbyte;// defination length
/*				
				alternative; // not easy handle, and useless, just forget it temporarily ... 
						     //  fulfile it later.
*/
				pTerm[DictIndex]->defStarPos = ftell(fpin);
				DictIndex++;						
			}break;
			default:
				printf("@curpos=%d, datapos=%d,unexpected low_nibble %d\n", curpos, datapos, hdr & 0xF);
			break;
		}
		fseek(fpin, datapos+record_length, SEEK_SET);
	}
#ifdef DEBUG	
	WriteHeadWordToFile("HeadWordUnsorted");
#endif
	quicksort(pTerm, 0, DictIndex-1, strcmp);
#ifdef DEBUG
	WriteHeadWordToFile("HeadWordSorted");//Write function to headword
#endif	
	while(1)
	{
		lookup(fpin);
	}

	fclose(fpin);
	return 0;
}

int binsearch(char *word, struct Dict *tab[], int n)
{
	int low = 0;
	int high = n;
	int mid;
	int comp;
	while(low <= high)
	{
		mid = (low + high) >> 1;
		if((comp = strcmp(word, tab[mid]->word)) > 0)
			low = mid + 1;
		else if(comp < 0)
			high = mid - 1;
		else 
			return mid;			
	}
	return -1; // no match
}

void quicksort(struct Dict *v[], int left, int right, int (*cmp)(const char *, const char *))
{
	int i, last;
	void swap(struct Dict *v[], int i, int j);
	if(left >= right)
		return;
	swap(v, left, (left+right)/2);
	
	last = left;
	for(i = left+1; i<=right; i++)
		if((*cmp)(v[i]->word, v[left]->word) < 0)
			swap(v, ++last, i);

	swap(v,left,last);
	quicksort(v, left, last-1, cmp);
	quicksort(v, last+1, right, cmp);	
}

void swap(struct Dict *v[], int i, int j)
{
	struct Dict *temp;

	temp = v[i];
	v[i] = v[j];
	v[j] = temp;
}

#define IN		0
#define OUT		1

void lookup(FILE *fDict)
{
	char word[32]={0,};
	char *pDef;
	int pos;
	void HtmlTotxt(const char *Dat,const unsigned int len);

	printf("Please input a word: ");
//	gets(word);
	scanf("%s", word);
	printf("--- *_* ---------------------------------------------\n");
	if((pos = binsearch(word, pTerm, DictIndex-1)) != -1)
	{
		fseek(fDict, pTerm[pos]->defStarPos, SEEK_SET);
		pDef = (char *)malloc(pTerm[pos]->defLenth+1);
		fread(pDef, 1, pTerm[pos]->defLenth, fDict);
		*(pDef + pTerm[pos]->defLenth) = '\0';
		HtmlTotxt(pDef,strlen(pDef));
		free(pDef);
	}
	else
	{
		printf("can't find %s\n", word);
	}	
}

void HtmlTotxt(const char *Dat, const unsigned int len)
{
	int i=0;
	char *pOutput = (char *)malloc(len+1);
	char *pOutputBase = pOutput;
	char tagtable[32];
	
	static int state = IN;
	static int tagIndex = 0;
#ifdef COLLINS_V3
	const char *pFontColorBlue = "669900"; //34 blue
	const char *pFontColorGreen = "004080"; // 
#endif
	const char *pFontColorNavy = "font color=navy"; 
	const char *pFontColorTeal = "font color=teal";// font color=teal, 36 dark green
	const char *pFontColorBrown = "font color=brown size=+1";// 33 yellow
	const char *pFontColorEnd = "/font";//
	
	for(i=0; i<len; i++, Dat++)
	{
		if(*Dat == '<')
		{
			state = IN;//in <...>
			tagIndex = 0;
			continue;
		}	
		else if(*Dat == '>')
		{
			state = OUT;// out of <...>	//	judge the HTML tags
			tagtable[tagIndex] = '\0';
//			Parse Html Tags ...
			if(strcmp(tagtable, "br") == 0 || strcmp(tagtable, "p") == 0 /* || strcmp(tagtable, "ul") == 0*/
				|| strcmp(tagtable, "/ul") == 0 ) //<br> => '\n',p => paragraph 
			{	*pOutput++ = '\n'; *pOutput++ = ' ';	}
			else if(!strcmp(tagtable, "li")) //<li>
			{
				*pOutput++ = '\n'; *pOutput++ = '-'; *pOutput++ = '-'; *pOutput++ = '>'; *pOutput++ = ' ';	
			}
#ifdef COLLINS_V3 			
			else if(strstr(tagtable,pFontColorBlue) != NULL)
			{
				*pOutput++ = '\033'; *pOutput++ = '['; *pOutput++ = '3'; *pOutput++ = '2'; *pOutput++ = 'm';
			}
			else if(strstr(tagtable,pFontColorGreen) != NULL)
			{
				*pOutput++ = '\033'; *pOutput++ = '['; *pOutput++ = '3'; *pOutput++ = '4'; *pOutput++ = 'm';
			}
#endif		
#ifdef NEW_OXFORD
			else if(strcmp(tagtable, pFontColorNavy) == 0)
			{
				*pOutput++ = '\033'; *pOutput++ = '['; *pOutput++ = '3'; *pOutput++ = '4'; *pOutput++ = 'm';
			}
			else if(strcmp(tagtable, pFontColorTeal) == 0)
			{
				*pOutput++ = '\033'; *pOutput++ = '['; *pOutput++ = '3'; *pOutput++ = '2'; *pOutput++ = 'm';
			}
			else if(strcmp(tagtable, pFontColorBrown) == 0)
			{
				*pOutput++ = '\033'; *pOutput++ = '['; *pOutput++ = '3'; *pOutput++ = '3'; *pOutput++ = 'm';
			}
#endif			

			else if(strcmp(tagtable, "b")==0 || (strcmp(tagtable, "i") == 0))
			{
				*pOutput++ = '\033'; *pOutput++ = '['; *pOutput++ = '1'; *pOutput++ = 'm';
			}
			else if(strcmp(tagtable, pFontColorEnd)==0 || strcmp(tagtable, "/b") == 0 
													   || strcmp(tagtable, "/i") == 0)
			{
				*pOutput++ = '\033'; *pOutput++ = '['; *pOutput++ = '0'; *pOutput++ = 'm'; 
			}
			else
			{
#ifdef DEBUG		
		 		printf("%s", tagtable);		
#endif			
			}		
			continue;
		}
		else
		{
			if(state == OUT)                
				*pOutput++ = *Dat;
			else // state = IN
			{ 
				tagtable[tagIndex] = *Dat;
				tagIndex++;
			}
		}
	}
	*pOutput = '\0';// stop the string. 
	
	pOutput = pOutputBase;
	printf("%s\n", pOutput);
	printf("--------------------------------------------- +_+ ---\n");
#ifdef DEBUG
	WriteToFile("Output", pOutput);//for Debug, not need when program run normally
#endif
	free(pOutput);
}

void PrintTerm()
{
	int i=0;
	for(i=0;i<DictIndex;i++)
		printf("%s\n", pTerm[i]->word);	
}
/*
char *CharsetConvert(char *s, char *from, char *to)
{	
 	unsigned int iconv(iconv_t cd, char **inbuf, size_t *inbytesleft, char **outbuf, size_t *outbytesleft); 
 	//iconv_t iconv_open(const char *tocode, const char *fromcode);
 	iconv_t cd = iconv_open(to, from);
 	if(cd == (iconv_t)(-1))
	{
		printf("Error openning iconv library\n" );
		exit(1);
	}

	char *inbuf, *outbuf, *defbuf;
	unsigned int inbufbytes, outbufbytes;

	inbufbytes = strlen(s);
	outbufbytes = inbufbytes * 6;// why?
	inbuf = (char*)s;
	outbuf = (char*)malloc(outbufbytes + 1);
	memset(outbuf, '\0', outbufbytes + 1 );
	defbuf = outbuf;// reserve the address of outbuf,
	while(inbufbytes)
	{
		if(iconv(cd, &inbuf, &inbufbytes, &outbuf, &outbufbytes) == -1)
		{
			printf("%s\n",outbuf );
			printf("Error in iconv conversion\n");
			exit(1);
    	}
	}
	iconv_close(cd);
	return defbuf;
}*/

char hex2char(unsigned char dat)
{
    if(dat > 15)
    	printf("erro convert char to hex");
	return (dat > 9 ? (dat - 10 + 'A'):(dat+'0'));	
}

