#include <stdio.h>
#include <time.h>

#define KEYSIZE 16	 
typedef unsigned int WORD; 

typedef enum { ShiftLeft, ShiftRight } ShiftDir;


typedef enum { KeyWords = KEYSIZE / 4,
	       NumRounds = 15,  
	       TableSize = 32  
} bogus;


WORD Table[ TableSize ];
WORD L[KeyWords];



WORD ROT(const WORD x, const WORD y, const ShiftDir dir)
{

  const unsigned int ShiftAmt = (y & (unsigned int)0x1f);
  const unsigned int ShiftBack = 0x20 - ShiftAmt;
  unsigned int result;

  if (dir == ShiftLeft)
    result = (x << ShiftAmt) | (x >> ShiftBack);
  else
    result = (x >> ShiftAmt) | (x << ShiftBack);
  return result;
} 



void SetKey( unsigned char KeyChar )
{
    static unsigned int KeyCntr;
    static unsigned int Shift;

    int ix = KeyCntr >> 2;

    L[ ix ] = (L[ ix ] & (~(0xff << Shift))) | (KeyChar << Shift);

    Shift = (Shift + 8) & 0x1f;
    KeyCntr = (KeyCntr + 1) & (KEYSIZE - 1);  
} 




void encrypt(WORD *PlainText, WORD *CryptoText)
{

  WORD i, temp;
  WORD A;
  WORD B;

  A = PlainText[0] + Table[0];
  B = PlainText[1] + Table[1];

  for (i = 1; i <= NumRounds; i++) {
    temp = i << 1;
    A = ROT(A^B, B, ShiftLeft) + Table[temp];
    B = ROT(A^B, A, ShiftLeft) + Table[temp+1];
  }
  CryptoText[0] = A;
  CryptoText[1] = B;
}  



void decrypt(WORD *CryptoText, WORD *PlainText)
{
  WORD i, temp;
  WORD B;
  WORD A;

  B = CryptoText[1];
  A = CryptoText[0];

  for (i=NumRounds; i > 0; i--) {
    temp = i << 1;
    B = ROT(B - Table[temp+1],A, ShiftRight)^A;
    A = ROT(A - Table[temp],  B, ShiftRight)^B;
  }
  PlainText[1] = B-Table[1];
  PlainText[0] = A-Table[0];
} 	




void setup() 
{
  static const WORD ROM[ TableSize ] = { 0xb7e15163, 0x5618cb1c, 0xf45044d5,
                                         0x9287be8e, 0x30bf3847, 0xcef6b200,
                                         0x6d2e2bb9, 0x0b65a572, 0xa99d1f2b,
                                         0x47d498e4, 0xe60c129d, 0x84438c56,
                                         0x227b060f, 0xc0b27fc8, 0x5ee9f981,
                                         0xfd21733a, 0x9b58ecf3, 0x399066ac,
                                         0xd7c7e065, 0x75ff5a1e, 0x1436d3d7,
                                         0xb26e4d90, 0x50a5c749, 0xeedd4102,
                                         0x8d14babb, 0x2b4c3474, 0xc983ae2d,
                                         0x67bb27e6, 0x05f2a19f, 0xa42a1b58,
                                         0x42619511, 0xe0990eca };
  WORD i;
  WORD A;
  WORD B;
  WORD j;
  WORD k;

  for (i=0; i < TableSize; i++)
    Table[i] = ROM[i];


  A = 0;
  B = 0;
  i = 0;
  j = 0;

  for (k=0; k < 3*TableSize; k++) {
    Table[i] = ROT(Table[i]+(A+B),3, ShiftLeft);
    A = Table[i];
    L[j] = ROT(L[j]+(A+B),(A+B), ShiftLeft);
    B = L[j];
    i= (i+1) & (TableSize-1);  
    j= (j+1) & (KeyWords-1);
  }
} 	



/*
   Testbench
 */
void main()
{
  WORD i, j;
  WORD PlainText1[2], PlainText2[2];
  WORD CryptoText[2] = {0,0};
  time_t t0, t1;
  char *keystr = "ThisIsATestString";

  if (sizeof(WORD)!=4)
    printf("RC5 error: WORD has %d bytes.\n",sizeof(WORD));

  printf("RC5-32/12/16 examples:\n");
  for (i = 1; i<6; i++)
    {
      PlainText1[0] = CryptoText[0];
      PlainText1[1] = CryptoText[1];
      for (j=0; j < KEYSIZE; j++)
         SetKey((unsigned char)keystr[j]);
      /* Setup, encrypt, and decrypt */
      setup();
      encrypt(PlainText1,CryptoText);
      decrypt(CryptoText,PlainText2);

      /* Print out results, checking for decryption failure */
      printf("\n   plaintext %.8lX %.8lX  --->  ciphertext %.8lX %.8lX  \n",
             PlainText1[0], PlainText1[1], CryptoText[0], CryptoText[1]);
      if (PlainText1[0]!=PlainText2[0] || PlainText1[1]!=PlainText2[1])
        printf("Decryption Error!");
    }
  time (&t0);
  for (i=1;i<100000;i++)
     encrypt(CryptoText,CryptoText);
  time (&t1);
  printf ("\n   Time_t for 100000 blocks:  %ld \n", t1-t0);
}