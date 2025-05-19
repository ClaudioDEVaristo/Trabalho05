#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#include "hardware/pwm.h"
#include "hardware/pio.h"
#include "lib/config_matriz.h"
#include "filas.pio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
#define ADC_JOYSTICK_X 26
#define ADC_JOYSTICK_Y 27
#define LED_RED 13
#define LED_BLUE 12
#define LED_GREEN  11
#define BUZZER_PIN 10

typedef struct
{
uint8_t x;
uint8_t y;
} joystick_data_t;

QueueHandle_t xQueueJoystickData;

void vJoystickTask(void *params)
{
    adc_gpio_init(ADC_JOYSTICK_Y);
    adc_gpio_init(ADC_JOYSTICK_X);
    adc_init();

    joystick_data_t joydata;

    while (true)
    {
        adc_select_input(0); // GPIO 26 = ADC0
        uint16_t y_pos = adc_read();

        adc_select_input(1); // GPIO 27 = ADC1
        uint16_t x_pos = adc_read();

        int16_t x_raw = x_pos - 2048;
            if (x_raw < 0) x_raw = 0;
            joydata.x = (x_raw * 100) / 2047;

        uint16_t y_raw = 2048 - y_pos;
            if (y_raw < 0) y_raw = 0; // Módulo
            if (y_raw > 2047) y_raw = 2047; 
            joydata.y = (y_raw * 100) / 2047;

        xQueueSend(xQueueJoystickData, &joydata, 0); // Envia o valor do joystick para a fila
        vTaskDelay(pdMS_TO_TICKS(100));              // 10 Hz de leitura
    }
}

void vDisplayTask(void *params)
{
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    bool cor = true;
                         
    ssd1306_t ssd;
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT);
    ssd1306_config(&ssd);

    joystick_data_t joydata;
    while (true)
    {
        if (xQueueReceive(xQueueJoystickData, &joydata, portMAX_DELAY) == pdTRUE)
        {
            if(joydata.x >= 80){
                ssd1306_fill(&ssd, !cor);
                ssd1306_draw_string(&ssd, "ALERTA", 40, 20);
                ssd1306_draw_string(&ssd, "DE", 55, 30); 
                ssd1306_draw_string(&ssd, "CHUVAS FORTES", 10, 40);
                ssd1306_send_data(&ssd);
            }
            else if(joydata.y >= 70){
                ssd1306_fill(&ssd, !cor);
                ssd1306_draw_string(&ssd, "ALERTA", 40, 20);
                ssd1306_draw_string(&ssd, "DE", 55, 30); 
                ssd1306_draw_string(&ssd, "ENCHENTE", 33, 40);  
                ssd1306_send_data(&ssd);
            } 
            else{
                ssd1306_fill(&ssd, !cor);
                ssd1306_draw_string(&ssd, "Nivel de agua", 3, 10);  // Desenha uma string
                ssd1306_rect(&ssd, 20, 3, 100, 12, true, false);
                ssd1306_line(&ssd, 82, 115, 82, 125, cor);  
                ssd1306_draw_string(&ssd, "Volume de chuva", 3, 40); 
                ssd1306_rect(&ssd, 50, 3, 100, 12, true, false);
                ssd1306_line(&ssd, 72, 20, 72, 30, true);  
                ssd1306_rect(&ssd, 50, 3, joydata.x, 12, true, true);
                ssd1306_rect(&ssd, 20, 3, joydata.y, 12, true, true);
                ssd1306_send_data(&ssd);
            }
        }
    }
}

void vMatrizTask(void *params)
{
    PIO pio = pio_config();

    const uint16_t period = 20000; //  20ms = 20000 ticks
    const float DIVIDER_PWM = 125.0; //Divisor de clock
    uint slice_num;

    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
    slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
  
    pwm_set_clkdiv(slice_num, 125.0); // 125 MHz / 125 = 1 MHz
    pwm_set_wrap(slice_num, 250);     // 1 MHz / 250 = 4 kHz (frequência audível)
    pwm_set_enabled(slice_num, true);// Inicia o PWM
    uint slice = pwm_gpio_to_slice_num(BUZZER_PIN);
    uint chan = pwm_gpio_to_channel(BUZZER_PIN);

    joystick_data_t joydata;
    while (true)
    {
        if (xQueueReceive(xQueueJoystickData, &joydata, portMAX_DELAY) == pdTRUE)
        {
            if(joydata.x >= 80){
                define_numero(1, pio, 0);
                pwm_set_chan_level(slice, chan, 150);
                vTaskDelay(pdMS_TO_TICKS(500));

                define_numero(0, pio, 0);
                pwm_set_chan_level(slice, chan, 0);
                vTaskDelay(pdMS_TO_TICKS(500));
            }
            else if(joydata.y >= 70){
                define_numero(2, pio, 0);
                pwm_set_chan_level(slice, chan, 150);
                vTaskDelay(pdMS_TO_TICKS(250));

                define_numero(0, pio, 0);
                pwm_set_chan_level(slice, chan, 0);
                vTaskDelay(pdMS_TO_TICKS(250));
            } 
            else{
                define_numero(0, pio, 0);
                pwm_set_chan_level(slice, chan, 0);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50)); // Atualiza a cada 50ms
    }
}

void vLedRedTask(void *params)
{
    gpio_init(LED_RED);
    gpio_set_dir(LED_RED, OUT_PIN);

    joystick_data_t joydata;
    while (true)
    {
        if (xQueueReceive(xQueueJoystickData, &joydata, portMAX_DELAY) == pdTRUE)
        {

            if(joydata.x >= 80 || joydata.y >= 70){
                gpio_put(LED_RED, 1);
            }else{
                gpio_put(LED_RED, 0);
            }
            
        }
        vTaskDelay(pdMS_TO_TICKS(50)); // Atualiza a cada 50ms
    }
}


// Modo BOOTSEL com botão B
#include "pico/bootrom.h"
#define botaoB 6
void gpio_irq_handler(uint gpio, uint32_t events)
{
    reset_usb_boot(0, 0);
}

int main()
{
    // Ativa BOOTSEL via botão
    gpio_init(botaoB);
    gpio_set_dir(botaoB, GPIO_IN);
    gpio_pull_up(botaoB);
    gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    stdio_init_all();

    // Cria a fila para compartilhamento de valor do joystick
    xQueueJoystickData = xQueueCreate(5, sizeof(joystick_data_t));

    // Criação das tasks
    xTaskCreate(vJoystickTask, "Joystick Task", 256, NULL, 1, NULL);
    xTaskCreate(vDisplayTask, "Display Task", 512, NULL, 1, NULL);
    xTaskCreate(vMatrizTask, "Matriz Task", 256, NULL, 1, NULL);
    xTaskCreate(vLedRedTask, "Led Red Task", 256, NULL, 1, NULL);
    // Inicia o agendador
    vTaskStartScheduler();
    panic_unsupported();
}