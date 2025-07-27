#define my_printf(fmt, ...) printf("%s   "fmt, get_cur_time(), ##__VA_ARGS__)

#include <bcm2835.h>
#include <stdio.h>
#include <time.h>

#define PIN11 RPI_BPLUS_GPIO_J8_11
#define PIN13 RPI_BPLUS_GPIO_J8_13

char *get_cur_time()
{
    static char s[20];
    time_t t;
    struct tm* ltime; 
 
    time(&t); 
    ltime = localtime(&t); 
    strftime(s, 20, "%Y-%m-%d %H:%M:%S", ltime); 
    return s;
}

int main (void)
{
  if(!bcm2835_init())
  {
      my_printf("BCM2835 initialization fails !!!");
      return -1;
  }


  uint8_t k=0;
  char buf[64];

  srand((unsigned)time(NULL));

  bcm2835_gpio_fsel(PIN11, BCM2835_GPIO_FSEL_OUTP);
  bcm2835_gpio_fsel(PIN13, BCM2835_GPIO_FSEL_INPT);

  for (int i=0;i<10;i++)
  {
      my_printf("GPIO 11 is High\n");
      bcm2835_gpio_write(PIN11, HIGH);
      bcm2835_delay(30);
      k = bcm2835_gpio_lev(PIN13);
      my_printf("GPIO 13 is %d\n",k);

      my_printf("GPIO 11 is OFF\n");
      bcm2835_gpio_write(PIN11, LOW);
      bcm2835_delay(30);
      k = bcm2835_gpio_lev(PIN13);
      my_printf("GPIO 13 is %d\n",k);

      int value = rand()%2;
      my_printf("GPIO 11 is set to %d in random\n", value);
      bcm2835_gpio_write(PIN11, value);
      bcm2835_delay(30);
      k = bcm2835_gpio_lev(PIN13);
      my_printf("GPIO 13 is %d\n",k);

      my_printf("\n\n");
      bcm2835_delay(80);
      
      my_printf(" ********************************\n\n");
      bcm2835_delay(80);


  }  
  return 0 ;
}
