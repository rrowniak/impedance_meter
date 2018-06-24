#ifndef __measure_H
#define __measure_H
#ifdef __cplusplus
extern "C" {
#endif

#define V_REF 3.3
#define SHUNT_R1 50.0
#define SHUNT_R2 5700.0
#define DIV_R1 1000.0
#define DIV_R2 2000.0

#define REPEAT_MEASURE 5

typedef enum {
    STATE_OK,
    STATE_INFINITY,
    STATE_LOW_PRECISION
} States;

void init_measurement_unit();
States get_last_meas_state();
float measure_input_v();
float measure_input_impedance();
float measeure_output_impedance();


#ifdef __cplusplus
}
#endif
#endif /*__measure_H */
