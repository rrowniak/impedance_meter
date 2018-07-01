#ifndef __measure_H
#define __measure_H
#ifdef __cplusplus
extern "C" {
#endif

#define V_REF 3.3
// R1 on the schematics
#define SHUNT_R1 46.2
// R2 on the schematics
#define SHUNT_R2 4590
// R6 on the schematics
#define DIV_R1 986.0
// R8 on the schematics
#define DIV_R2 550.0
// R9 on the schematics
#define DIV_R3 994.0
// R10 on the schematics
#define DIV_R4 549.0

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
