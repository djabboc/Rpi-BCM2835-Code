#define my_printf(fmt, ...) printf("%s   "fmt, get_cur_time(), ##__VA_ARGS__)

#include <bcm2835.h>
#include <stdio.h>
#include <time.h>

#define PIN_LED RPI_BPLUS_GPIO_J8_07

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
    if (!bcm2835_init())
    {
        my_printf("BCM2835 initialization fails !!!");
        return -1;
    }

    srand((unsigned) time(NULL));

    bcm2835_gpio_fsel(PIN_LED, BCM2835_GPIO_FSEL_OUTP); // 输出

    while (1)
    {
        my_printf("ACT On!\n");
        bcm2835_gpio_write(PIN_LED, HIGH);
        bcm2835_delay(1000);

        my_printf("ACT Off!\n");
        bcm2835_gpio_write(PIN_LED, LOW);
        bcm2835_delay(1000);
    }
    return 0;
}
