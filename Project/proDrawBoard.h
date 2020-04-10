#include "mbed.h"
#include "stm32f413h_discovery.h"
#include "stm32f413h_discovery_ts.h"
#include "stm32f413h_discovery_lcd.h"
#include "TCPSocket.h"
#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"
#include <cstdlib>
#include <string>

void drawSubMsg()
{
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_FillRect(0, 0, BSP_LCD_GetXSize(), 40);
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(0, 15, (uint8_t *)"SYSTEM OK", CENTER_MODE);
}

void drawTopbar()
{
    BSP_LCD_SetTextColor(LCD_COLOR_DARKMAGENTA);
    BSP_LCD_FillRect(0, 0, BSP_LCD_GetXSize(), 40);
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_SetBackColor(LCD_COLOR_DARKMAGENTA);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(0, 15, (uint8_t *)"Haunted House Control", CENTER_MODE);
}

void drawLightWhite()
{
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_FillRect(0, 40, BSP_LCD_GetXSize()/2, BSP_LCD_GetYSize()/2 - 20);
    BSP_LCD_DisplayStringAt(0, 40, (uint8_t *)"Light", LEFT_MODE);
}

void drawLightYellow()
{
    
    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
    BSP_LCD_FillRect(0, 40, BSP_LCD_GetXSize()/2, BSP_LCD_GetYSize()/2 - 20);
    BSP_LCD_DisplayStringAt(0, 40, (uint8_t *)"Light", LEFT_MODE);
}

void drawSmokeWhite()
{
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_FillRect(BSP_LCD_GetXSize()/2, 40, BSP_LCD_GetXSize()/2, BSP_LCD_GetYSize()/2 - 20);
    BSP_LCD_DisplayStringAt(80, 40, (uint8_t *)"Smoke", CENTER_MODE);
}

void drawSmokeLB()
{
    BSP_LCD_SetTextColor(LCD_COLOR_LIGHTBLUE);
    BSP_LCD_FillRect(BSP_LCD_GetXSize()/2, 40, BSP_LCD_GetXSize()/2, BSP_LCD_GetYSize()/2 - 20);
    BSP_LCD_DisplayStringAt(80, 40, (uint8_t *)"Smoke", CENTER_MODE);
}

void drawSFXWhite()
{
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_FillRect(0, BSP_LCD_GetYSize()/2 +20, BSP_LCD_GetXSize()/2, BSP_LCD_GetYSize()/2 - 20);
    BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()/2 +20, (uint8_t *)"SFX", LEFT_MODE);
}

void drawSFXGreen()
{
    BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
    BSP_LCD_FillRect(0, BSP_LCD_GetYSize()/2 +20, BSP_LCD_GetXSize()/2, BSP_LCD_GetYSize()/2 - 20);
    BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()/2 +20, (uint8_t *)"SFX", LEFT_MODE);
}

void drawUploadWhite()
{
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_FillRect(BSP_LCD_GetXSize()/2, BSP_LCD_GetYSize()/2 +20, BSP_LCD_GetXSize()/2, BSP_LCD_GetYSize()/2 - 20);
    BSP_LCD_DisplayStringAt(80, BSP_LCD_GetYSize()/2 +20, (uint8_t *)"Upload", CENTER_MODE);
}

void drawUploadGray()
{
    BSP_LCD_SetTextColor(LCD_COLOR_GRAY);
    BSP_LCD_FillRect(BSP_LCD_GetXSize()/2, BSP_LCD_GetYSize()/2 +20, BSP_LCD_GetXSize()/2, BSP_LCD_GetYSize()/2 - 20);
    BSP_LCD_DisplayStringAt(80, BSP_LCD_GetYSize()/2 +20, (uint8_t *)"Upload", CENTER_MODE);
}

void drawUploadGrayUploaded()
{
    BSP_LCD_SetTextColor(LCD_COLOR_GRAY);
    BSP_LCD_FillRect(BSP_LCD_GetXSize()/2, BSP_LCD_GetYSize()/2 +20, BSP_LCD_GetXSize()/2, BSP_LCD_GetYSize()/2 - 20);
    BSP_LCD_DisplayStringAt(80, BSP_LCD_GetYSize()/2 +20, (uint8_t *)"Uploaded", CENTER_MODE);
}
