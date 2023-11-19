#include "stm32f10x.h"
#include "stm32f10x_abl_delay.h"
#include "stm32f10x_abl_key.h"
#include "stm32f10x_abl_led.h"
#include "stm32f10x_abl_oled.h"
#include "stm32f10x_abl_serial.h"
#include "stm32f10x_abl_iap.h"

IAP_InitTypeDef IAPx;

int main()
{
    KEY_InitTypeDef key1;
    KEY_Init(&key1, RCC_APB2Periph_GPIOB, GPIOB, GPIO_Pin_12);

    LED_InitTypeDef ledDefault;
    LedDefault_Init(&ledDefault);

    OLED_InitTypeDef oled1;
    Oled1_Init(&oled1);

    // IAP Mode
    OLED_ShowString(&oled1, 16, 20, " BOOTLOADER ", OLED_FONT_SIZE_16, OlED_COLOR_REVERSED);
    OLED_RefreshScreen(&oled1);

    SERIAL_InitTypeDef serial;
    Serial1_Init(&serial);

    IAP_Init(
        &IAPx,
        &serial,
        0x8002000);

    if (KEY_IsPressed(&key1) == 1) {
        IAP_WriteFlag(&IAPx, IAP_COMMAND_WAITING);
    } else if (!IAP_Execute(&IAPx)) {
        OLED_ShowString(&oled1, 0, 50, "Error", OLED_FONT_SIZE_12, OLED_COLOR_NORMAL);
        OLED_RefreshScreen(&oled1);
        IAP_WriteFlag(&IAPx, IAP_COMMAND_WAITING);
    }

    while (1) {
        LED_Toggle(&ledDefault);

        uint16_t flag = IAP_ReadFlag(&IAPx);
        switch (flag) {
            case IAP_COMMAND_EXECUTE:
                OLED_ShowString(&oled1, 0, 50, "Loading...", OLED_FONT_SIZE_12, OLED_COLOR_NORMAL);
                OLED_RefreshScreen(&oled1);
                if (!IAP_Execute(&IAPx)) {
                    OLED_ShowString(&oled1, 0, 50, "Error", OLED_FONT_SIZE_12, OLED_COLOR_NORMAL);
                    OLED_RefreshScreen(&oled1);
                    IAP_WriteFlag(&IAPx, IAP_COMMAND_WAITING);
                }
                break;
            case IAP_COMMAND_WAITING:
                OLED_ShowString(&oled1, 0, 50, "Wating...", OLED_FONT_SIZE_12, OLED_COLOR_NORMAL);
                OLED_RefreshScreen(&oled1);

                IAP_ShowMenu(&IAPx);

                uint8_t cmd[128] = {0};
                IAP_GetInputString(&IAPx, cmd);

                OLED_ShowChar(&oled1, 0, 0, cmd[0], OLED_FONT_SIZE_12, OLED_COLOR_NORMAL);
                OLED_RefreshScreen(&oled1);

                if (cmd[0] == '0') {
                    IAP_WriteFlag(&IAPx, IAP_COMMAND_EXECUTE);
                    break;
                } else if (cmd[0] == '1') {
                    IAP_WriteFlag(&IAPx, IAP_COMMAND_DOWNLOAD);
                    break;
                } else {
                    IAP_WriteFlag(&IAPx, IAP_COMMAND_WAITING);
                    break;
                }
                break;
            case IAP_COMMAND_DOWNLOAD:
                OLED_ShowString(&oled1, 0, 50, "Updating...", OLED_FONT_SIZE_12, OLED_COLOR_NORMAL);
                OLED_RefreshScreen(&oled1);

                if (!IAP_Download(&IAPx)) {
                    IAP_WriteFlag(&IAPx, IAP_COMMAND_EXECUTE);
                } else {
                    IAP_WriteFlag(&IAPx, IAP_COMMAND_WAITING);
                }
                break;
            default:
                break;
        }

        OLED_RefreshScreen(&oled1);
        Delay_ms(100);
    }
}

// void USART1_IRQHandler(void)
// {
//     IAP_SerialIRQHandler(&IAPx);
// }