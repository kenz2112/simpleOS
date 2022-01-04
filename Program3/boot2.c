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

struct pcb
{
    unsigned int esp;
    unsigned int pid;
}__attribute__((packed));
typedef struct pcb pcb_t;

#define QUEUE_SIZE 100
struct queue
{
  unsigned int head;
  unsigned int tail;
  pcb_t pcb_s[QUEUE_SIZE];
};
typedef struct queue qt;

//Assignment 1
void convert_num(unsigned int, char[]); 
int convert_num_h(unsigned int, char[]); 
void println(char *string); 
extern void k_print(char *string, int string_length, int row, int col); 
extern void k_scroll(); 
extern void k_clearscr();
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

idt_entry_t idt[256];

//Assignment 3
#define MAX_P 6
#define STACK_SIZE 1024
unsigned int stacks[5][1024];
qt queues;
int num_process = 0; 
int s_allocated = 0;
int p_allocated = 0;
int current_process;

int create_process(unsigned int processEntry);
unsigned int* allocate_stack();
pcb_t* allocatePCB();
void init_timer_dev(int);
void dispatch();
unsigned int retval;
extern void go();
void pt1();
void pt2();
void pt3();
void pt4();
void pt5();
void process_idle();



int main()
{
  k_clearscr();
  println("Running Processes :)");
  initIDT();
  setupPIC();

  //program the timer device
  init_timer_dev(50);
 

  //initialize RR data structs
  queues.head = 0;
	queues.tail = 0;

  retval = create_process((unsigned long) process_idle);

  retval = create_process((unsigned long) pt1);
    if (retval < 0)
    println("p1 0 error");

  retval = create_process((unsigned long) pt2);
    if (retval < 0)
    println("p2 0 error");

  retval = create_process((unsigned long) pt3);
    if (retval < 0)
    println("p3 0 error");

  retval = create_process((unsigned long) pt4);
    if (retval < 0)
    println("p4 0 error");

  retval = create_process((unsigned long) pt5);
    if (retval < 0)
    println("p5 0 error");

  go();

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
      initIDTEntry(idt + i, dispatch, 16, 0x8e);
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
  outportb(0x21,0xfc); //changed for Program 3
  outportb(0xA1, 0xff);
}

void default_handler()
{
  println("error");

  while(1)
  {

  }
}


//queue functtions
void enqueue(pcb_t *pcb)
{
	queues.pcb_s[queues.tail] = *pcb;
	queues.tail = ++queues.tail%QUEUE_SIZE;
}

pcb_t* dequeue()
{
	int head = queues.head;
	queues.head = ++queues.head%QUEUE_SIZE;
	return &(queues.pcb_s[head]);
}

//Create Process
//processEntry = pointer to function containing process code
int create_process(unsigned int processEntry)
{

  //set stackptr to return value from allocate stack 
  unsigned int *stackptr = allocate_stack();  
  

  //set up stack 
  unsigned int *st = (unsigned int *)stackptr + STACK_SIZE;
  st--;
  *st = (unsigned int) go;
  st--;
  *st = 0x0200;
  st--;
  *st = 16;
  st--;
  //st = address of process
  *st = processEntry;
  st --;

  //ebp
  *st = 0;
  st--;
  //esp
  *st = 0;
  st--;
  //edi
  *st = 0;
  st--;
  //esi
  *st = 0;
  st--;
  //edx
  *st = 0;
  st--;
  //ecx
  *st = 0;
  st--;
  //ebx
  *st = 0;
  st--;
  //eax
  *st = 0;
  st--;
  //ds
  *st = 8;
  st--;
  //es
  *st = 8;
  st--;
  //fs
  *st = 8;
  st--;
  //gs
  *st = 8;
  //st--;

  pcb_t *pcb = allocatePCB();
	pcb->esp = (unsigned int) st;
	num_process++;
	pcb->pid = num_process;
  enqueue(pcb);
  return 0;
}

unsigned int* allocate_stack()
{
  return stacks[s_allocated++];
}

pcb_t* allocatePCB()
{
  p_allocated++;
  return &queues.pcb_s[p_allocated-1];
}

void pt1()
{
  int i =0;
  while (1)
    {
      char num[5];
      char temp[100] = "process p1: ";
      //set msg to ("process p1: " + i) where i is converted to string form
      convert_num(i, num);

      int j_print = 0;
      for (int j = 0; temp[j]!='\0'; j++)
      {
        j_print++;
      }
      k_print(temp, j_print, 5, 0);

      j_print = 0;
      for (int j = 0; num[j]!='\0'; j++)
      {
        j_print++;
      }
      k_print(num, j_print, 5, 12);
      i = ((i+1) % 500);
    }
}

void pt2()
{
  int i =0;
  while (1)
    {
      char num[5];
      char temp[100] = "process p2: ";
      //set msg to ("process p1: " + i) where i is converted to string form
      convert_num(i, num);

      int j_print = 0;
      for (int j = 0; temp[j]!='\0'; j++)
      {
        j_print++;
      }
      k_print(temp, j_print, 6, 0);

      j_print = 0;
      for (int j = 0; num[j]!='\0'; j++)
      {
        j_print++;
      }
      k_print(num, j_print, 6, 12);
      i = ((i+1) % 500);
    }
}

void pt3()
{
  int i =0;
  while (1)
    {
      char num[5];
      char temp[100] = "process p3: ";
      //set msg to ("process p1: " + i) where i is converted to string form
      convert_num(i, num);

      int j_print = 0;
      for (int j = 0; temp[j]!='\0'; j++)
      {
        j_print++;
      }
      k_print(temp, j_print, 7, 0);

      j_print = 0;
      for (int j = 0; num[j]!='\0'; j++)
      {
        j_print++;
      }
      k_print(num, j_print, 7, 12);
      i = ((i+1) % 500);
    }
}

void pt4()
{
  int i =0;
  while (1)
    {
      char num[5];
      char temp[100] = "process p4: ";
      //set msg to ("process p1: " + i) where i is converted to string form
      convert_num(i, num);

      int j_print = 0;
      for (int j = 0; temp[j]!='\0'; j++)
      {
        j_print++;
      }
      k_print(temp, j_print, 8, 0);

      j_print = 0;
      for (int j = 0; num[j]!='\0'; j++)
      {
        j_print++;
      }
      k_print(num, j_print, 8, 12);
      i = ((i+1) % 500);
    }
}

void pt5()
{
  int i =0;
  while (1)
    {
      char num[5];
      char temp[100] = "process p5: ";
      //set msg to ("process p1: " + i) where i is converted to string form
      convert_num(i, num);

      int j_print = 0;
      for (int j = 0; temp[j]!='\0'; j++)
      {
        j_print++;
      }
      k_print(temp, j_print, 9, 0);

      j_print = 0;
      for (int j = 0; num[j]!='\0'; j++)
      {
        j_print++;
      }
      k_print(num, j_print, 9, 12);
      i = ((i+1) % 500);
    }
}

void process_idle()
{
  while(1)
  {

  }
}