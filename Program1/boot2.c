//assembly functions
extern void k_print(char *string, int string_length, int row, int col); 
void k_scroll(); 
void k_clearscr();

//C functions
void convert_num(unsigned int, char[]); 
int convert_num_h(unsigned int, char[]); 
void println(char *string); 

//global variable to hold row #
int row = 0;

int main()
{
  k_clearscr();
  int i = 0;
  int c = 0; 
  int count = 0; 
  
  char snum[80]; 

  //Print the first 20 prime numbers
  for(count = 1; count <= 20; i++)
  {
    //check if c is prime
    for(c = 2; c < i; c++)
    {
      if(i%c == 0)
      break;
    }

    //if c is prime
    if(c == i)  
    {
      convert_num(c, snum); 
      println(snum); //print the now converted number
      count++;    // increment amount of prime numbers
    }
  }

    while(1)
    {
        //infinite while loop
    }

    return 0;
}

//used to print character to screen
void println(char *msg)
{
    int numToPrint = 0;
    numToPrint = sizeof(msg);
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
  if (num == 0) 
  {
    return 0;
  }
  int idx = convert_num_h(num / 10, buf);
  buf[idx] = num % 10 + '0';
  buf[idx+1] = '\0';
  return idx + 1;
}


void convert_num(unsigned int num, char buf[]) {
  if (num == 0) 
  {
    buf[0] = '0';
    buf[1] = '\0';
  } else {
    convert_num_h(num, buf);
  }
}