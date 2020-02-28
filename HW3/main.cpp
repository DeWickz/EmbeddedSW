////////////////////////////////////////////////
HW3 NUM1 4 THREADS

#include "mbed.h"
#include "stm32f413h_discovery.h"
#include "stm32f413h_discovery_ts.h"
#include "stm32f413h_discovery_lcd.h"
#include "string"
#include "mbed.h"
 
DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
Thread thread1, thread2, thread3;
 
void led2_thread() 
{
    while (true) 
    {
        led2 = !led2;
        wait(0.1);
    }
}

void led3_thread() 
{
    while (true) 
    {
        led3 = !led3;
        wait(0.5);
    }
}

void lcd_thread()
{
    while(true)
    {
        BSP_LCD_Clear(LCD_COLOR_YELLOW);
        thread_sleep_for(1000);
        BSP_LCD_Clear(LCD_COLOR_BLACK);
        thread_sleep_for(1000);

    }
}
 
int main()
{
    thread1.start(led2_thread);
    thread2.start(led3_thread);
    thread3.start(lcd_thread);
    
    while (true) 
    {
        led1 = !led1;
        wait(1.5);
    }
}

///////////////////////////////////////////////////////

HW3 NUM2 MUTEX
  
#include "mbed.h"
#include "stm32f413h_discovery.h"
#include "stm32f413h_discovery_ts.h"
#include "stm32f413h_discovery_lcd.h"
#include "string"

Mutex stdio_mutex;
Thread temp, humid, light;
TS_StateTypeDef  TS_State = {0};
    
void notify(int val, string type) 
{
    stdio_mutex.lock();

    string text;
    text = to_string(val);

    BSP_LCD_Clear(LCD_COLOR_BLACK);
    BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_SetFont(&Font20);

    if(type == "Temperature")
    {
        BSP_LCD_DisplayStringAt(0, LINE(3), (uint8_t *)"Temperature", CENTER_MODE);
    }
    else if(type == "Humidity")
    {
        BSP_LCD_DisplayStringAt(0, LINE(3), (uint8_t *)"Humidity", CENTER_MODE);
    }
    else if(type == "Light intensity")
    {
        BSP_LCD_DisplayStringAt(0, LINE(3), (uint8_t *)"Light Intensity", CENTER_MODE);
    }
    BSP_LCD_DisplayStringAt(0, LINE(5), (uint8_t *)&text, CENTER_MODE);

    stdio_mutex.unlock();
    thread_sleep_for(1000);
}

void test_thread(void const *args)
 {
    int val = 0;
    string type;
    if((const char*)args == "TEMPERATURE") {
        type = "Temperature";
    } 
    else if ((const char*)args == "HUMIDITY") {
        type = "Humidity";
    } 
    else if ((const char*)args == "LIGHT INTENSITY") {
        type = "Light intensity";
    }
    
    while (true) 
    {
        val = rand() % 100;
        BSP_TS_GetState(&TS_State);
        if(TS_State.touchDetected) {
            notify(val, type);   
            thread_sleep_for(500);
        }
    }
}

int main() 
{
    BSP_LCD_Init();
    /* Touchscreen initialization */
    if (BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize()) == TS_ERROR) {
        printf("BSP_TS_Init error\n");
    }

    BSP_LCD_Clear(LCD_COLOR_BLACK);
    BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_SetFont(&Font20);

    temp.start(callback(test_thread, (void *)"TEMPERATURE"));
    humid.start(callback(test_thread, (void *)"HUMIDITY"));
    light.start(callback(test_thread, (void *)"LIGHT INTENSITY"));

}
/////////////////////////////////////////////////////////

HW3 NUM3 SEMAPHORE
#include "mbed.h"
#include "stm32f413h_discovery.h"
#include "stm32f413h_discovery_ts.h"
#include "stm32f413h_discovery_lcd.h"
#include "string"

Semaphore one_slot(2);
Thread humid, light, temp;
TS_StateTypeDef  TS_State = {0};

void test_thread(void const *args) 
{
    int val = 0;
    int pos = 0;
    string type;
    uint16_t color;

    if((const char*)args == "LIGHT") {
        color = LCD_COLOR_YELLOW;
        pos = -50;
    } else if ((const char*)args == "TEMPERATURE") {
        color = LCD_COLOR_RED;
        pos = 0;
    } else if ((const char*)args == "HUMIDITY") {
        color = LCD_COLOR_BLUE;
        pos = 50;
    }

    while (true) 
    {
        val = rand() % 100;
        one_slot.wait();
        string text;
        text = to_string(val);
        BSP_LCD_SetTextColor(color);
        BSP_LCD_DisplayStringAt(0, LINE(2), (uint8_t *)&type, CENTER_MODE);
        BSP_LCD_DisplayStringAt(pos, LINE(5), (uint8_t *)&text, CENTER_MODE);
        wait(2);
        one_slot.release();
    }
}

int main (void) 
{
    BSP_LCD_Init();

    if (BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize()) == TS_ERROR) {
        printf("BSP_TS_Init error\n");
    }

    BSP_LCD_Clear(LCD_COLOR_BLACK);
    BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_SetFont(&Font20);

    humid.start(callback(test_thread, (void *)"HUMIDITY"));
    light.start(callback(test_thread, (void *)"LIGHT"));
    temp.start(callback(test_thread, (void *)"TEMPERATURE"));

}
