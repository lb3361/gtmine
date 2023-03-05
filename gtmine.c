#include <stdlib.h>
#include <string.h>
#include <gigatron/sys.h>
#include <gigatron/libc.h>
#include <stdarg.h>

#define FGBG 0x3f20

#define MAXX 26
#define MAXY 17

#define SFREE 0
#define S1 1
#define S2 2
#define S3 3
#define S4 4
#define S5 5
#define S6 6
#define S7 7
#define S8 8
#define SBOMB 9
#define SBOMBHIDDEN 10
#define SCURSOR 11
#define SHIDDEN 12
#define SMARKER 13

#define BHIDDEN 0x10
#define BMARKER 0x20

static char sfree[]={44,44,44,44,44,46,44,44,44,44,44,46,44,44,44,44,44,46,44,44,44,44,44,46,44,44,44,44,44,46,46,46,46,46,46,46,250};        // 0
static char s1[]={44,44,48,48,44,46,44,44,44,48,44,46,44,44,44,48,44,46,44,44,44,48,44,46,44,44,44,48,44,46,46,46,46,46,46,46,250};           // 1
static char s2[]={44,8,8,8,44,46,44,44,44,44,8,46,44,44,8,8,44,46,44,8,44,44,44,46,44,8,8,8,8,46,46,46,46,46,46,46,250};                      // 2
static char s3[]={44,35,35,35,44,46,44,44,44,44,35,46,44,44,35,35,44,46,44,44,44,44,35,46,44,35,35,35,44,46,46,46,46,46,46,46,250};           // 3
static char s4[]={44,33,44,44,44,46,44,33,44,44,44,46,44,33,44,33,44,46,44,33,33,33,33,46,44,44,44,33,44,46,46,46,46,46,46,46,250};           // 4
static char s5[]={44,6,6,6,6,46,44,6,44,44,44,46,44,44,6,6,44,46,44,44,44,44,6,46,44,6,6,6,44,46,46,46,46,46,46,46,250};                      // 5
static char s6[]={44,44,57,57,44,46,44,57,44,44,44,46,44,57,57,57,44,46,44,57,44,44,57,46,44,44,57,57,44,46,46,46,46,46,46,46,250};           // 6
static char s7[]={44,16,16,16,16,46,44,44,44,44,16,46,44,44,44,16,44,46,44,44,16,44,44,46,44,44,16,44,44,46,46,46,46,46,46,46,250};           // 7
static char s8[]={44,44,37,37,44,46,44,37,44,44,37,46,44,44,37,37,44,46,44,37,44,44,37,46,44,44,37,37,44,46,46,46,46,46,46,46,250};           // 8
static char sbomb[]={16,19,16,19,16,19,19,62,16,16,19,19,16,16,16,16,16,19,19,16,16,16,19,19,16,19,16,19,16,19,19,19,19,19,19,19,250};        // 9
static char sbombhidden[]={16,44,16,44,16,46,44,61,16,16,44,46,16,16,16,16,16,46,44,16,16,16,44,46,16,44,16,44,16,46,46,46,46,46,46,46,250};  // 10
static char scursor[]={35,35,35,35,35,35,35,0,0,0,0,35,35,0,0,0,0,35,35,0,0,0,0,35,35,0,0,0,0,35,35,35,35,35,35,35,250};                      // 11
static char shidden[]={58,58,58,58,58,50,58,58,58,58,58,50,58,58,58,58,58,50,58,58,58,58,58,50,58,58,58,58,58,50,50,50,50,50,50,50,250};      // 12
static char smarker[]={58,58,19,19,58,50,58,19,19,19,58,50,58,58,19,19,58,50,58,58,58,1,58,50,58,58,1,1,1,50,50,50,50,50,50,50,250};          // 13

/*
void clear_lines(int l1, int l2)
{
    int i;
    for (i=l1; i<l2; i++) {
        char *row = (char*)(videoTable[i+i]<<8);
        memset(row, FGBG & 0xff, 160);
    }
}

void clear_screen()
{
    int i;
    for (i=0; i<120; i++) {
        videoTable[i+i] = 8 + i;
        videoTable[i+i+1] = 0;
    }
    clear_lines(0,120);
}
*/
// #############################################################################
typedef struct {
	char *addr;
	char x;
	char y;
} screenpos_t;

void clear_lines(int l1, int l2)
{
	int i;
	for (i=l1; i<l2; i++) {
		char *row = (char*)(videoTable[i+i]<<8);
		memset(row, FGBG & 0xff, 160);
	}
}

void clear_screen(screenpos_t *pos)
{
	int i;
	for (i=0; i<120; i++) {
		videoTable[i+i] = 8 + i;
		videoTable[i+i+1] = 0;
	}
	clear_lines(0,120);
	pos->x = pos->y = 0;
	pos->addr = (char*)(videoTable[0]<<8);
}

void scroll(void)
{
	char pages[8];
	int i;
	for (i=0; i<8; i++)
		pages[i] = videoTable[i+i];
	for (i=0; i<112; i++)
		videoTable[i+i] = videoTable[i+i+16];
	for (i=112; i<120; i++)
		videoTable[i+i] = pages[i-112];
}

void newline(screenpos_t *pos)
{
	pos->x = 0;
	pos->y += 1;
	if (pos->y >  14) {
		scroll();
		clear_lines(112,120);
		pos->y = 14;
	}
	pos->addr = (char*)(videoTable[16*pos->y]<<8);
}

void print_char(screenpos_t *pos, int ch)
{
	unsigned int fntp;
	char *addr;
	int i;
	if (ch < 32) {
		if (ch == '\n') 
			newline(pos);
		return;
	} else if (ch < 82) {
		fntp = font32up + 5 * (ch - 32);
	} else if (ch < 132) {
		fntp = font82up + 5 * (ch - 82);
	} else {
		return;
	}
	addr = pos->addr;
	for (i=0; i<5; i++) {
		SYS_VDrawBits(FGBG, SYS_Lup(fntp), addr);
		addr += 1;
		fntp += 1;
	}
	pos->x += 1;
	pos->addr = addr + 1;
	if (pos->x > 24)
		newline(pos);
}

screenpos_t pos;

void print_unsigned(unsigned int n, int radix)
{
	static char digit[] = "0123456789abcdef";
	char buffer[8];
	char *s = buffer;
	do {
		*s++ = digit[n % radix];
		n = n / radix;
	} while (n);
	while (s > buffer)
		print_char(&pos, *--s);
}

void print_int(int n, int radix)
{
	if (n < 0) {
		print_char(&pos, '-');
		n = -n;
	}
	print_unsigned(n, radix);
}

int myprintf(const char *fmt, ...)
{
	char c;
	va_list ap;
	va_start(ap, fmt);
	while (c = *fmt++) {
		if (c != '%') {
			print_char(&pos, c);
			continue;
		}
		if (c = *fmt++) {
			if (c == 'd')
				print_int(va_arg(ap, int), 10);
			else if (c == 'u')
				print_unsigned(va_arg(ap, unsigned), 10);
			else if (c == 'x')
				print_unsigned(va_arg(ap, unsigned), 16);
			else
				print_char(&pos, c);
		}
	}
	va_end(ap);
	return 0;
}
// #############################################################################
void mySprite(char *addr, char *dest){
    int i,z,v;
    z = 0;
    v = 0;
    i = 0;
    while(addr[v]<128){
        dest[z + i] = addr[v];
        v++; i++;
        if(i > 5){
            i = 0;
            z += 256;
        }
    }

}

void mySpritet(char *addr, char *dest){
    int i,z,v;
    z = 0;
    v = 0;
    i = 0;
    while(addr[v]<128){
        if(addr[v]>0) dest[z + i] = addr[v];
        v++; i++;
        if(i > 5){
            i = 0;
            z += 256;
        }
    }

}

void printSprite(int val, int xx, int yy)
{
	char* ptrChar;
	int sprnum;
	sprnum = val;
	ptrChar = (char*)scursor;		
	switch(sprnum){
        case SFREE:
            ptrChar = (char*)sfree;
            break;
        case S1:
            ptrChar = (char*)s1;
            break;
        case S2:
            ptrChar = (char*)s2;
            break;
        case S3:
            ptrChar = (char*)s3;
            break;
        case S4:
            ptrChar = (char*)s4;
            break;
        case S5:
            ptrChar = (char*)s5;
            break;
        case S6:
            ptrChar = (char*)s6;
            break;
        case S7:
            ptrChar = (char*)s7;
            break;
        case S8:
            ptrChar = (char*)s8;
            break;
        case SBOMB:
            ptrChar = (char*)sbomb;
            break;
        case SBOMBHIDDEN:
            ptrChar = (char*)sbombhidden;
            break;
        case SCURSOR:
            ptrChar = (char*)scursor;
            break;
        case SHIDDEN:
            ptrChar = (char*)shidden;
            break;
        case SMARKER:
            ptrChar = (char*)smarker;
            break;
	}
	mySprite(ptrChar, (char*)(yy*6+24<<8)+6*xx+2);
}

int main()
{
    
    unsigned int ticks = _clock();
    
    
    int i,x,y;
    int offset = 0;
    int fieldsx = 26;
    int fieldsy = 17;
    int numberbomb;
    int cx, cy;
    char field[MAXY][MAXX]; 
    char *selectspr;
    
    clear_screen(&pos);
    

    numberbomb = fieldsx * fieldsy * 15 / 100; // 15% bombs

    for( y=0; y<fieldsy; y++ ){
        for( x=0; x<fieldsx; x++ ){
            field[y][x] = SFREE;
            printSprite(field[y][x], x, y);
        }
    }

    i = 0; // bomb counter temp
    while(i < numberbomb){
        x = rand() % (fieldsx-1);
        y = rand() % (fieldsy-1);
        if(field[y][x] != SBOMB){ // field is not a bomb, bomb set
            i++;                  // add bomb
            field[y][x] = SBOMB;  // set marker for bomb
			// increase neighbor fields by one if no bomb
            if(x < fieldsx+1 ){ // Observe margins
                if(field[y][x+1] != SBOMB) field[y][x+1]++;                        // right
                if(y < fieldsy+1 ) if(field[y+1][x+1] != SBOMB) field[y+1][x+1]++; // bottom right
                if(y > 0 ) if(field[y-1][x+1] != SBOMB) field[y-1][x+1]++;         // top right
            }
            if(x > 0 ){
                if(field[y][x-1] != SBOMB) field[y][x-1]++;                        // left
                if(y < fieldsy+1 ) if(field[y+1][x-1] != SBOMB) field[y+1][x-1]++; // bottom left
                if(y > 0 ) if(field[y-1][x-1] != SBOMB) field[y-1][x-1]++;         // top left
            }
            if(y < fieldsy+1 ){
                if(field[y+1][x] != SBOMB) field[y+1][x]++;                        // bottom
            }
            if(y > 0 ) if(field[y-1][x] != SBOMB) field[y-1][x]++;                 // top
        }
    }

    cx = 0;
    cy = 0;
    
    mySpritet((char*)scursor, (char*)(cy*6+24<<8)+6*cx+2 );

    while(1){
        
        switch(buttonState) {
            case 0xFB: // down
                if(cy < fieldsy-1){
                    mySprite((char*)shidden, (char*)(cy*6+24<<8)+6*cx+2 );
                    cy++;
                    mySpritet((char*)scursor, (char*)(cy*6+24<<8)+6*cx+2 );
                }
                break;
            case 0xF7: // up
                if(cy > 0){
                    mySprite((char*)shidden, (char*)(cy*6+24<<8)+6*cx+2 );
                    cy--;
                    mySpritet((char*)scursor, (char*)(cy*6+24<<8)+6*cx+2 );
                }
            break;
            case 0xFD: // left
                if(cx > 0){
                    mySprite((char*)shidden, (char*)(cy*6+24<<8)+6*cx+2 );
                    cx--;
                    mySpritet((char*)scursor, (char*)(cy*6+24<<8)+6*cx+2 );
                }
            break;
            case 0xFE: // right
                if(cx < fieldsx-1){
                    mySprite((char*)shidden, (char*)(cy*6+24<<8)+6*cx+2 );
                    cx++;
                    mySpritet((char*)scursor, (char*)(cy*6+24<<8)+6*cx+2 );
                }
            break;
            case 0x20: // space
                for( y=0; y<fieldsy; y++ ){
                    for( x=0; x<fieldsx; x++ ){
                        printSprite(field[y][x], x, y);
                    }
                }

            break;
            case 0x0A: // enter
            break;
        }
		
   		pos.x = 0;
		pos.y = 0;
		pos.addr = (char*)(videoTable[16*pos.y]<<8)+6*pos.x;
		myprintf("X=%d Y=%d V=%d  ", cx, cy, field[cy][cx]);
		
        while(serialRaw != 0xFF) {}
    }
    
    return 0;

}
