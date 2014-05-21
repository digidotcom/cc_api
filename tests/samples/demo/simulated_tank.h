#ifndef _SIMULATED_TANK_H_
#define _SIMULATED_TANK_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#define MAX_VOLUME      4000
#define MIN_VOLUME      0

#define INIT_VOLUME     (MAX_VOLUME / 2)
#define INIT_VALVEIN    VALVE_CLOSED
#define INIT_VALVEOUT   VALVE_OPENED
#define INIT_TEMP       25
#define VALVEOUT_FLOW   15
#define VALVEIN_FLOW    10
#define MIN_TEMP        20
#define MAX_TEMP        30

typedef enum {
  VALVE_OPENED = 0,
  VALVE_CLOSED = 1
} valve_status_t;

void update_simulated_tank_status(void);
double get_tank_temperature(void);
int32_t get_tank_fill_percentage(void);
valve_status_t get_valveIN_status(void);
void set_valveIN_status(valve_status_t new_status);
valve_status_t get_valveOUT_status(void);
void set_valveOUT_status(valve_status_t new_status);
void print_tank_status(void);

#endif
