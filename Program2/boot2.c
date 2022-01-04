//structs
struct idt_entry
{
  unsigned short baselow_16;
  unsigned short selector;
  unsigned char always0;
  unsigned char access;
  unsigned short basehi_16;
}__attribute__((packed));
typedef struct idt_entry idt_entry_t;

struct gdt_r_s 
{
  unsigned short limit;
  idt_entry_t* base;
}__attribute__((packed));
typedef struct gdt_r_s gdt_r_t;

enum CSET_1 
{
  Q_PRESSED = 0x10, W_PRESSED = 0x11, E_PRESSED = 0x12, R_PRESSED = 0x13,
  T_PRESSED = 0x14, Y_PRESSED = 0x15, U_PRESSED = 0x16, I_PRESSED = 0x17,
  O_PRESSED = 0x18, P_PRESSED = 0x19
};
static char* cset_1_chars = "qwertyuiop";

enum CSET_2 
{
  A_PRESSED = 0x1E, S_PRESSED = 0x1F, D_PRESSED = 0x20, F_PRESSED = 0x21,
  G_PRESSED = 0x22, H_PRESSED = 0x23, J_PRESSED = 0x24, K_PRESSED = 0x25,
  L_PRESSED = 0x26
};
static char *cset_2_chars = "asdfghjkl";

enum CSET_3 
{
  Z_PRESSED = 0x2C, X_PRESSED = 0x2D, C_PRESSED = 0x2E, V_PRESSED = 0x2F,
  B_PRESSED = 0x30, N_PRESSED = 0x31, M_PRESSED = 0x32,
};
static char *cset_3_chars = "zxcvbnm";

enum CSET_NUMBERS
{
   ONE_PRESSED = 0x2, TWO_PRESSED = 0x3, THREE_PRESSED = 0x4,
   FOUR_PRESSED = 0x5, FIVE_PRESSED = 0x6, SIX_PRESSED = 0x7,
   SEVEN_PRESSED = 0x8, EIGHT_PRESSED = 0x9, NINE_PRESSED = 0xA
};
static char *cset_num = {"123456789"};


void convert_num(unsigned int, char[]); 
int convert_num_h(unsigned int, char[]); 
void println(char *string); 
extern void k_print(char *string, int string_length, int row, int col); 
void k_scroll(); 
void k_clearscr();
int myStringLen(char *string);
int row = 0;
int col = 0;

//Assignment 2
void initIDTEntry(idt_entry_t *entry, void* base, unsigned short selector, unsigned char access);
void initIDT();
void setupPIC();
void default_handler();
extern void kbd_enter();
void kbd_handler(int scancode);
extern void _got_key();
extern void _scan_buffer();
extern void sti_command();
void lidtr(gdt_r_t*);
void outportb();
char translate_scancode(int what);
char k_getchar();

//global variable for keyboard buffer
unsigned char keyboard_buffer[256]; 
int head = 0;
int tail = 0;
int size =0;


idt_entry_t idt[256];


int main()
{
  char* myArray = " ";
  k_clearscr();

  initIDT();
  setupPIC();
  sti_command();
  
  //println("hello");
    while(1)
    {
        myArray[0] = k_getchar();
        
        if(myArray[0] != 0)
        {
          if (myArray[0] == '\n')
            {
              row++;
              col = 0;
              if (row >= 24)
              {
                row = 24;
                k_scroll();
              }
            }   
            else
            {
              k_print(myArray, 1, row, col);
              col++;
              if (col >= 80)
              {
                col = 0;
                row++;
                  if (row >= 24)
                  {
                    row = 24;
                    k_scroll();
                  }

              }
            }
        }
    }

    return 0;
}

void println(char *msg)
{
    int numToPrint = 0;
    numToPrint = myStringLen(msg);
    while (numToPrint != 0)
    {
        if(numToPrint < 80)
        {
            k_print(msg, numToPrint, row, 0);
            numToPrint = 0;
        }

        else
        {
            k_print(msg, numToPrint, 0, 80);
            numToPrint = numToPrint - 80;
        }
        row++;

        if(row > 24)
        {
            k_scroll();
            row = 24;
        }
    }
}

int convert_num_h(unsigned int num, char buf[]) {
  if (num == 0) {
    return 0;
  }
  int idx = convert_num_h(num / 10, buf);
  buf[idx] = num % 10 + '0';
  buf[idx+1] = '\0';
  return idx + 1;
}


void convert_num(unsigned int num, char buf[]) {
  if (num == 0) {
    buf[0] = '0';
    buf[1] = '\0';
  } else {
    convert_num_h(num, buf);
  }
}

int myStringLen(char *string)
{
	int c = 0;
	while(*string != '\0')
	{
		c++;
		*string++;
	}
	return(c);
}

//Assignment 2
void initIDTEntry(idt_entry_t *entry, void* base, unsigned short selector, unsigned char access)
{
  entry->baselow_16 = (unsigned int) base&0x0000FFFF;
  entry->basehi_16 = ((unsigned int) base&0xFFFF0000)>>16;
  entry->selector = selector;
  entry->access = access;
  entry->always0 = 0;
  
}

void initIDT()
{
  int i;
  for (i = 0; i < 256; i++)
  {
    if (i < 32)
      initIDTEntry(idt + i, default_handler, 16, 0x8e);
    else if (i==32)
      initIDTEntry(idt + i, 0, 0, 0);
    else if (i == 33)
      initIDTEntry(idt + i, kbd_enter, 16, 0x8e);
    else
      initIDTEntry(idt + i, 0, 0, 0);
  }
  gdt_r_t idtr;
  idtr.base = idt;
  idtr.limit = (sizeof(idt_entry_t)*256) - 1;
  lidtr(&idtr);
}



void kbd_handler(int scancode)
{
  if (scancode ==0 || size == 256)
    return;
  char a = translate_scancode(scancode);
  if (a == 0)
    return;
  keyboard_buffer[tail] = a;
  tail++;
  tail = tail %256;
  size++;
  return;
}

void setupPIC()
{
  //cascading mode
  outportb(0x20, 0x11);
  outportb(0xA0, 0x11);
  outportb(0x21, 0x20);
  outportb(0xA1, 0x28);

  //tell master he has slave
  outportb(0x21, 0x04);
  outportb(0xA1, 0x02);

  //enable 8086 mode
  outportb(0x21, 0x01);
  outportb(0xA1, 0x01);

  //reset the IRQ masks
  outportb(0x21, 0x0);
  outportb(0xA1, 0x0);

  //Now enable the keyboard IRQ only
  outportb(0x21,0xfd);
  outportb(0xA1, 0xff);
}

void default_handler()
{
  println("error");

  while(1)
  {

  }
}

char translate_scancode(int what)
{
  if (Q_PRESSED<= what && what <= P_PRESSED)
    return cset_1_chars[what-Q_PRESSED];
  else if (what >= A_PRESSED && what <= L_PRESSED)
    return cset_2_chars[what-A_PRESSED];
  else if (what >= Z_PRESSED && what <= M_PRESSED)
    return cset_3_chars[what-Z_PRESSED];
  else if (what >= ONE_PRESSED && what <= NINE_PRESSED)
    return cset_num[what-ONE_PRESSED];
  else if (what == 0x0B)//zero
    return '0';
  else if (what == 0x39)//space
    return ' ';
  else if (what == 0x1C || what == 0xE)//new line
    return '\n';
  else if (what == 0x34)//point
    return '.';
  else if (what == 0x35)//slash
    return '/';
  else 
  {
    
    return 0;
  }
  
}

char k_getchar()
{
  char temp;
  if (size == 0)
  return 0;

  temp = keyboard_buffer[head];
  head++;
  head = head % 256;
  size--;
  return temp;
}

