#include "logic.h"

#include "gpio.h"
#include "main.h"
#include "hd44780.h"
#include "measure.h"
#include "version.h"

typedef uint8_t bool;
#define true 1
#define false 0

enum State {
    ST_IDLE,
    ST_MEASURE_IN,
    ST_MEASURE_OUT,
};

static enum State state = ST_IDLE;
static bool measure_output = true;

static float input_v = 0.0;
static int input_v_state = STATE_OK;
static float impedance_resutl = 0.0;
// static int impedance_result_state = STATE_OK;
static enum State last_measurement = ST_IDLE;

char omega = 0b11110100;

void process_inputs(uint32_t tick_ms)
{
    static uint32_t last_tick_ms = 0;

    if (state != ST_IDLE) {
        return;
    }

    if (tick_ms - last_tick_ms < 300) {
        return;
    }

    if (HAL_GPIO_ReadPin(SEL_MODE_GPIO_Port, SEL_MODE_Pin) == GPIO_PIN_RESET) {
        measure_output = !measure_output;
    }

    if (HAL_GPIO_ReadPin(MEASURE_GPIO_Port, MEASURE_Pin) == GPIO_PIN_RESET) {
        if (measure_output) {
            state = ST_MEASURE_OUT;
        } else {
            state = ST_MEASURE_IN;
        }
    }

    last_tick_ms = tick_ms;
}

void ctrl_led(bool on)
{
    if (on) {
        HAL_GPIO_WritePin(CTRL_LED_GPIO_Port, CTRL_LED_Pin, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(CTRL_LED_GPIO_Port, CTRL_LED_Pin, GPIO_PIN_RESET);
    }
}

void refresh_display(uint32_t tick_ms, bool force)
{
    static uint32_t last_ms = 0;

    if ((tick_ms - last_ms < 500) && !force) {
        return;
    }

    lcd_set_cursor(0, 0);
    if (input_v_state & STATE_OUT_OF_RANGE) {
        lcd_printfln("Input: %.3f V OOR", input_v);
    } else {
        lcd_printfln("Input: %.3f V", input_v);
    }

    char mode;
    if (measure_output) {
        mode = 'O';
    } else {
        mode = 'I';
    }

    if (state != ST_IDLE) {
        if (state == ST_MEASURE_IN) {
            lcd_println("Meas. Input...", 1);
        } else {
            lcd_println("Meas. Output...", 1);
        }
    } else {
        lcd_set_cursor(0, 1);
        if (last_measurement == ST_IDLE) {
            lcd_printfln("[%c] Press key...", mode);
        } else if (last_measurement == ST_MEASURE_IN) {
            lcd_printfln("[%c] I=%.1f%c", mode, impedance_resutl, omega);
        } else if (last_measurement == ST_MEASURE_OUT) {
            lcd_printfln("[%c] O=%.1f%c", mode, impedance_resutl, omega);
        }
    }

    last_ms = tick_ms;
}

void logic_init(void)
{
    // initialize LCD controller
    lcd_init();
    lcd_on();
    lcd_clear();
	lcd_disable_cursor();
	lcd_println("Impedance meter", 0);
    lcd_println("Firmware: v" VERSION, 1);
    // welcome message
    for (uint32_t i = 0; i < 10; ++i) {
        ctrl_led(true);
        HAL_Delay(100);
        ctrl_led(false);
        HAL_Delay(100);
    }

    init_measurement_unit();
}

void logic_update(void)
{
    static uint32_t last_inp_v_ms = 0;

    uint32_t tick = HAL_GetTick();

    process_inputs(tick);
    if (tick - last_inp_v_ms > 300) {
        input_v = measure_input_v(tick);
        input_v_state = get_last_meas_state();
        last_inp_v_ms = tick;
    }

    refresh_display(tick, false);

    if (state == ST_MEASURE_IN) {
        impedance_resutl = measure_input_impedance();
        last_measurement = ST_MEASURE_IN;
    } else if (state == ST_MEASURE_OUT) {
        impedance_resutl = measeure_output_impedance();
        last_measurement = ST_MEASURE_OUT;
    }

    state = ST_IDLE;
}
