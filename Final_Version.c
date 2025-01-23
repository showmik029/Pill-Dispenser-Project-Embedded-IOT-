#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

#define MOTOR_PIN_1 2
#define MOTOR_PIN_2 3
#define MOTOR_PIN_3 6
#define MOTOR_PIN_4 13

#define calibration_pin 9
#define start_pin 8

#define PIEZOLED_PIN 20
#define STANDBYLED_PIN 21
#define CALIBLED_PIN 22

#define SENSOR_PIN 28
#define PIEZO_PIN 27

#define DELAY 10000
#define BLINK_LEN 1000

#define UART_PORT uart0

volatile bool piezo_triggered = false;
volatile uint32_t last_interrupt_time = 0;
volatile uint32_t last_calib_interrupt_time = 0;
bool calib_button_pressed = false;

typedef enum {
    STATE_STANDBY,
    STATE_CALIBRATION,
    STATE_WAIT_FOR_START,
    STATE_TABLET_DROP
} system_state_t;

void calibration_button_callback(uint gpio, uint32_t events) {
    if (gpio == calibration_pin && (events & GPIO_IRQ_EDGE_FALL)) {
        uint32_t current_time = to_ms_since_boot(get_absolute_time());
        if (current_time - last_calib_interrupt_time > 200) {
            calib_button_pressed = true;
            last_calib_interrupt_time = current_time;
        }
    }
}

void initialize_gpio(void)
{
    gpio_init(SENSOR_PIN);
    gpio_set_dir(SENSOR_PIN, GPIO_IN);
    gpio_pull_up(SENSOR_PIN);

    gpio_init(MOTOR_PIN_1);
    gpio_set_dir(MOTOR_PIN_1, GPIO_OUT);

    gpio_init(MOTOR_PIN_2);
    gpio_set_dir(MOTOR_PIN_2, GPIO_OUT);

    gpio_init(MOTOR_PIN_3);
    gpio_set_dir(MOTOR_PIN_3, GPIO_OUT);

    gpio_init(MOTOR_PIN_4);
    gpio_set_dir(MOTOR_PIN_4, GPIO_OUT);

    gpio_init(calibration_pin);
    gpio_set_dir(calibration_pin, GPIO_IN);
    gpio_pull_up(calibration_pin);

    gpio_init(start_pin);
    gpio_set_dir(start_pin,GPIO_IN);
    gpio_pull_up(start_pin);

    gpio_init(PIEZO_PIN);
    gpio_set_dir(PIEZO_PIN,GPIO_IN);
    gpio_pull_up(PIEZO_PIN);

    gpio_init(PIEZOLED_PIN);
    gpio_set_dir(PIEZOLED_PIN,GPIO_OUT);

    gpio_init(STANDBYLED_PIN);
    gpio_set_dir(STANDBYLED_PIN,GPIO_OUT);

    gpio_init(CALIBLED_PIN);
    gpio_set_dir(CALIBLED_PIN,GPIO_OUT);
}


void motor_stop(void)
{
    gpio_put(MOTOR_PIN_1, 0);
    gpio_put(MOTOR_PIN_2, 0);
    gpio_put(MOTOR_PIN_3, 0);
    gpio_put(MOTOR_PIN_4, 0);
    sleep_ms(1);
}

void blink(uint32_t pin) {
    gpio_put(pin, true);
    sleep_ms(BLINK_LEN);
    gpio_put(pin, false);
    sleep_ms(BLINK_LEN);
}

void middle_point(uint8_t (*activated_coils)[4], uint8_t activation_count, uint16_t activations_per_turn, uint16_t middle_gap)
{
    printf("moving to middle\n");
    uint16_t activation_counter = 0;
    static int current_step_index = 0;

    while(activation_counter <= (activations_per_turn + middle_gap)) {
        gpio_put(MOTOR_PIN_1, activated_coils[current_step_index][0]);
        gpio_put(MOTOR_PIN_2, activated_coils[current_step_index][1]);
        gpio_put(MOTOR_PIN_3, activated_coils[current_step_index][2]);
        gpio_put(MOTOR_PIN_4, activated_coils[current_step_index][3]);
        sleep_ms(1);
        ++activation_counter;

        current_step_index = (current_step_index + 1) % activation_count;
    }

    motor_stop();
}

void till_opto_activation(uint8_t (*activated_coils)[4], uint8_t activation_count)
{
    bool sensor_state = gpio_get(SENSOR_PIN);
    bool last_sensor_state = sensor_state;
    static int current_step_index = 0;

    while(true) {
        sensor_state = gpio_get(SENSOR_PIN);
        if(!sensor_state && last_sensor_state) {
            break;
        }

        gpio_put(MOTOR_PIN_1, activated_coils[current_step_index][0]);
        gpio_put(MOTOR_PIN_2, activated_coils[current_step_index][1]);
        gpio_put(MOTOR_PIN_3, activated_coils[current_step_index][2]);
        gpio_put(MOTOR_PIN_4, activated_coils[current_step_index][3]);
        sleep_ms(1);

        current_step_index = (current_step_index + 1) % activation_count;
        last_sensor_state = sensor_state;
    }
    motor_stop();
    printf("At sensor position.\n");
}

void calibration(uint8_t (*activated_coils)[4], uint8_t activation_count, bool *calibration_flag, uint16_t *activations_per_turn, uint16_t *middle_gap)
{   printf("calibration starts :)\n");
    gpio_set_irq_enabled_with_callback(calibration_pin, GPIO_IRQ_EDGE_FALL, false, &calibration_button_callback);
    till_opto_activation(activated_coils, activation_count);

    uint32_t activations_outside_sensor = 0;
    uint32_t activations_inside_sensor = 0;
    static int current_step_index = 0;

    uint8_t revolutions = 0;

    while(revolutions != 1) {
        while(!gpio_get(SENSOR_PIN)) {
            gpio_put(MOTOR_PIN_1, activated_coils[current_step_index][0]);
            gpio_put(MOTOR_PIN_2, activated_coils[current_step_index][1]);
            gpio_put(MOTOR_PIN_3, activated_coils[current_step_index][2]);
            gpio_put(MOTOR_PIN_4, activated_coils[current_step_index][3]);
            sleep_ms(1);
            ++activations_inside_sensor;

            current_step_index = (current_step_index + 1) % activation_count;
        }
        while(gpio_get(SENSOR_PIN)) {
            gpio_put(MOTOR_PIN_1, activated_coils[current_step_index][0]);
            gpio_put(MOTOR_PIN_2, activated_coils[current_step_index][1]);
            gpio_put(MOTOR_PIN_3, activated_coils[current_step_index][2]);
            gpio_put(MOTOR_PIN_4, activated_coils[current_step_index][3]);
            sleep_ms(1);
            ++activations_outside_sensor;

            current_step_index = (current_step_index + 1) % activation_count;
        }
        ++revolutions;
    }

    *middle_gap = (activations_inside_sensor)/2;
    *activations_per_turn = (activations_inside_sensor + activations_outside_sensor);

    motor_stop();
    *calibration_flag = true;
}

void tablet_movement(uint8_t (*activated_coils)[4],uint8_t activation_count, uint16_t activations_per_turn)
{
    static int current_step_index = 0;
    uint32_t steps_to_execute;
    steps_to_execute = activations_per_turn / 8;
    printf("steps\n");
    printf("steps_to_execute = %d\n", steps_to_execute);

    for(uint32_t i = 0; i < steps_to_execute; i++) {
        gpio_put(MOTOR_PIN_1, activated_coils[current_step_index][0]);
        gpio_put(MOTOR_PIN_2, activated_coils[current_step_index][1]);
        gpio_put(MOTOR_PIN_3, activated_coils[current_step_index][2]);
        gpio_put(MOTOR_PIN_4, activated_coils[current_step_index][3]);
        sleep_ms(1);

        current_step_index = (current_step_index + 1) % activation_count;
    }

}

void piezo_interrupt(uint gpio, uint32_t events)
{
    if (gpio == PIEZO_PIN) {
        if (events & GPIO_IRQ_EDGE_FALL) {
            piezo_triggered = true;
        }
    }
}

void tablet_drop(uint8_t (*activated_coils)[4],uint8_t activation_count, uint16_t activations_per_turn,uint8_t compartment,bool *calibration_flag)
{
    printf("tablet_drop\n");

    for(uint32_t i=0;i<compartment;i++) {
        piezo_triggered = false;
        gpio_set_irq_enabled_with_callback(PIEZO_PIN, GPIO_IRQ_EDGE_FALL, true, &piezo_interrupt);
        tablet_movement(activated_coils,activation_count,activations_per_turn);

        sleep_ms(200);

        if (piezo_triggered==true)
        {
            printf("piezo_triggered\n");
            if(i==compartment-1) {
                //nosleep if on last
            }
            else {
                sleep_ms(DELAY);
            }

        }
        else
        {
            printf("piezo didn't trigger\n");
            for(uint8_t count=0;count<5;count++) {
                blink(PIEZOLED_PIN);
            }
            if(i==compartment-1) {
                //nosleep if on last
            }
            else {
                sleep_ms(DELAY);
            }

            gpio_set_irq_enabled_with_callback(PIEZO_PIN, GPIO_IRQ_EDGE_FALL, false, &piezo_interrupt);
        }
        *calibration_flag=false;
    }
}

int main(void)
{
    stdio_init_all();
    printf("System booting...\n");

    initialize_gpio();

    uint8_t activated_coils[][4] = {
        { 1, 0, 0, 0 },
        { 1, 1, 0, 0 },
        { 0, 1, 0, 0 },
        { 0, 1, 1, 0 },
        { 0, 0, 1, 0 },
        { 0, 0, 1, 1 },
        { 0, 0, 0, 1 },
        { 1, 0, 0, 1 }
    };

    uint8_t activation_count = sizeof(activated_coils) / sizeof(activated_coils[0]);
    uint16_t step_to_middle = 0;
    bool calibration_flag = false;
    uint16_t activations_per_turn = 0;
    uint8_t compartment=7;

    system_state_t current_state = STATE_STANDBY;

    while (true) {
        switch (current_state) {

            case STATE_STANDBY:
                blink(STANDBYLED_PIN);
                gpio_set_irq_enabled_with_callback(calibration_pin, GPIO_IRQ_EDGE_FALL, true, &calibration_button_callback);
                if(calib_button_pressed) {
                    calib_button_pressed = false;
                    current_state = STATE_CALIBRATION;
                }
                break;

            case STATE_CALIBRATION:
                calibration(activated_coils,activation_count,&calibration_flag,&activations_per_turn,&step_to_middle);
                middle_point(activated_coils,activation_count,activations_per_turn,step_to_middle);
                gpio_put(CALIBLED_PIN,1);

                if(calibration_flag) {
                    current_state = STATE_WAIT_FOR_START;
                }
                break;

            case STATE_WAIT_FOR_START:
                gpio_put(STANDBYLED_PIN,0);
                if(!gpio_get(start_pin)) {
                    while(!gpio_get(start_pin)){
                        sleep_ms(50);
                    }

                    gpio_put(CALIBLED_PIN,0);
                    current_state = STATE_TABLET_DROP;
                }
                break;

            case STATE_TABLET_DROP:
                tablet_drop(activated_coils,activation_count,activations_per_turn,compartment, &calibration_flag);
                if(!calibration_flag) {
                    printf("tablet_drop completed, returning to standby\n");
                    current_state = STATE_STANDBY;
                    calib_button_pressed = false;
                    calibration_flag = false;
                }
                break;

            default:
                printf("unknown state\n");
                current_state = STATE_STANDBY;
                break;
        }
        sleep_ms(1);
    }
}
