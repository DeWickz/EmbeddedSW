#include "mbed.h"
#include "stm32f413h_discovery.h"
#include "stm32f413h_discovery_ts.h"
#include "stm32f413h_discovery_lcd.h"
#include "string"
 
Timer t;
 
int main() 
{
    TS_StateTypeDef TS_State = {0};
    uint16_t x1, y1;
    int touchCount = 0;

    BSP_LCD_Init();
    if (BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize()) == TS_ERROR) {
        printf("BSP_TS_Init error\n");
    }
    BSP_LCD_Clear(LCD_COLOR_BLACK);
    BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
  
    t.start();
        while(1){
        string currentTime = to_string(t.read());
        BSP_LCD_DisplayStringAt(0, LINE(7), (uint8_t *)&currentTime, CENTER_MODE);
        if(t.read()<5.0){
            BSP_TS_GetState(&TS_State);
            if(TS_State.touchDetected) {
                thread_sleep_for(50);
                touchCount++;
                string touchNum = to_string(touchCount);
    
                BSP_LCD_Clear(LCD_COLOR_BLACK);
                BSP_LCD_DisplayStringAt(0, LINE(3), (uint8_t *)"Touch count:", CENTER_MODE);
                BSP_LCD_DisplayStringAt(0, LINE(5), (uint8_t *)&touchNum, CENTER_MODE);

            }
        }
        else{

            t.stop();
            break;

        }
    }

	string touchNum = to_string(touchCount);
    printf("FINAL TOUCH COUNT = %d\n", touchCount);
	
    BSP_LCD_Clear(LCD_COLOR_RED);
    BSP_LCD_SetBackColor(LCD_COLOR_RED);
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_DisplayStringAt(0, LINE(3), (uint8_t *)"Final Touch Count", CENTER_MODE);
    BSP_LCD_DisplayStringAt(0, LINE(5), (uint8_t *)&touchNum, CENTER_MODE);


}
