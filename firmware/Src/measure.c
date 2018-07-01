#include "measure.h"

#include "adc.h"
#include "gpio.h"
#include "hd44780.h"
#include "version.h"

#define VOLT_EPSILON 0.0007
#define INFINITY 100000
#define ADC_DELAY_MS 5
#define ADC_DISCHARGE_MS 5
#define MEASURE_SLEEP_MS 3

#define IMP_ACCURACY_THRESHOLD 700

#define CHECK_NON_ZERO(v) \
if (v < VOLT_EPSILON) { \
    state = STATE_INFINITY; \
    goto infinity; \
} \

static States state = STATE_OK;

static void error(char* msg)
{
    lcd_println("ERROR FW:v" VERSION, 0);
    lcd_println(msg, 1);
    while (1);
}

void init_measurement_unit()
{
    HAL_ADC_Start(&hadc1);
    HAL_ADC_Start(&hadc2);
}

States get_last_meas_state() {
    return state;
}

static inline void init_measure() {
    state = STATE_OK;
}

static inline void select_shunt_low_res() {
    HAL_GPIO_WritePin(REL_1_GPIO_Port, REL_1_Pin, GPIO_PIN_SET);
}

static inline void select_shunt_high_res() {
    HAL_GPIO_WritePin(REL_1_GPIO_Port, REL_1_Pin, GPIO_PIN_RESET);
}

static inline void close_shunt_to_gnd() {
    HAL_GPIO_WritePin(REL_2_GPIO_Port, REL_2_Pin, GPIO_PIN_RESET);
}

static inline void redirest_shunt_to_output() {
    HAL_GPIO_WritePin(REL_2_GPIO_Port, REL_2_Pin, GPIO_PIN_SET);
}

static inline void prepare_for_measurement() {
    // discharge all capacitors
    HAL_GPIO_WritePin(DISCHARGE_1_GPIO_Port, DISCHARGE_1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(DISCHARGE_2_GPIO_Port, DISCHARGE_2_Pin, GPIO_PIN_SET);

    HAL_Delay(ADC_DISCHARGE_MS);

    HAL_GPIO_WritePin(DISCHARGE_1_GPIO_Port, DISCHARGE_1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DISCHARGE_2_GPIO_Port, DISCHARGE_2_Pin, GPIO_PIN_RESET);

    // wait a bit
    HAL_Delay(ADC_DELAY_MS);
}

static inline void enable_volt_divs() {
    HAL_GPIO_WritePin(DIV_1_GPIO_Port, DIV_1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(DIV_2_GPIO_Port, DIV_2_Pin, GPIO_PIN_SET);
}

static inline void disable_volt_divs() {
    HAL_GPIO_WritePin(DIV_1_GPIO_Port, DIV_1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DIV_2_GPIO_Port, DIV_2_Pin, GPIO_PIN_RESET);
}

static float measure_v(ADC_HandleTypeDef* hadc)
{
    float sum = 0;
    // repeat measurements to get average voltage
    for (uint16_t i = 0; i < REPEAT_MEASURE; ++i) {
        HAL_StatusTypeDef ret = HAL_ADC_PollForConversion(hadc, 500);

        if (ret == HAL_ERROR) {
            error("POOL_CONV");
        } else if (ret == HAL_TIMEOUT) {
            error("POOL_CONV_TIM");
        }

        uint16_t rawValue = HAL_ADC_GetValue(hadc);
        sum = sum + ((float)rawValue) / 4095 * V_REF;
        HAL_Delay(MEASURE_SLEEP_MS);
    }
    return sum / REPEAT_MEASURE;
}

static float measure_v_auto(ADC_HandleTypeDef* hadc, float div1, float div2)
{
    enable_volt_divs();

    HAL_Delay(ADC_DELAY_MS);

    float v1 = measure_v(hadc);
    // calculate real voltage
    v1 = v1 * (div1 + div2) / div2;

    if (v1 < V_REF) {
        // try to acquire better results
        disable_volt_divs();

        HAL_Delay(ADC_DELAY_MS);

        v1 = measure_v(hadc);

        enable_volt_divs();
    }

    return v1;
}

float measure_input_impedance()
{
    init_measure();

    // open the circuit as we're measuring input impedance
    redirest_shunt_to_output();

    select_shunt_high_res();

    prepare_for_measurement();

    // measure input voltage
    float vpp1 = measure_v_auto(&hadc1, DIV_R1, DIV_R2);

    // measure voltage on output
    float vpp2 = measure_v_auto(&hadc2, DIV_R3, DIV_R4);

    CHECK_NON_ZERO(vpp1 - vpp2);

    float z1 = SHUNT_R2 * vpp2 / (vpp1 - vpp2);

    if (z1 < IMP_ACCURACY_THRESHOLD) {
        // we may get better acurracy using lower shunt resistor
        select_shunt_low_res();

        prepare_for_measurement();
        // measure input voltage
        float vpp1 = measure_v_auto(&hadc1, DIV_R1, DIV_R2);
        float vpp2 = measure_v_auto(&hadc2, DIV_R3, DIV_R4);

        CHECK_NON_ZERO(vpp1 - vpp2);

        z1 = SHUNT_R1 * vpp2 / (vpp1 - vpp2);
    }

    // go to default state
    select_shunt_high_res();

    return z1;
infinity:
    // go to default state
    select_shunt_high_res();
    return INFINITY;
}

float measeure_output_impedance()
{
    init_measure();

    // open the circuit as we're going to measure input voltage
    redirest_shunt_to_output();

    prepare_for_measurement();

    // measure input voltage
    float vpp1 = measure_v_auto(&hadc1, DIV_R1, DIV_R2);

    // close the circuit as we're going to measure voltage under load
    select_shunt_high_res();
    close_shunt_to_gnd();

    prepare_for_measurement();

    // measure voltage under load
    float vpp2 = measure_v_auto(&hadc1, DIV_R1, DIV_R2);

    CHECK_NON_ZERO(vpp2);

    float z1 = SHUNT_R2 * (vpp1 - vpp2) / vpp2;

    if (z1 < IMP_ACCURACY_THRESHOLD) {
        // we may get better acurracy using lower shunt resistor
        select_shunt_low_res();

        prepare_for_measurement();
        // measure input voltage
        float vpp2 = measure_v_auto(&hadc1, DIV_R1, DIV_R2);

        CHECK_NON_ZERO(vpp2);

        z1 = SHUNT_R1 * (vpp1 - vpp2) / vpp2;
    }

    // go to default state
    select_shunt_high_res();
    redirest_shunt_to_output();

    return z1;
infinity:
    // go to default state
    select_shunt_high_res();
    redirest_shunt_to_output();
    return INFINITY;
}

float measure_input_v()
{
    init_measure();

    prepare_for_measurement();

    return measure_v_auto(&hadc1, DIV_R1, DIV_R2);
}
