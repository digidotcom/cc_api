#include "simulated_tank.h"

static int32_t volume = INIT_VOLUME;
static valve_status_t valveIN = INIT_VALVEIN;
static valve_status_t valveOUT = INIT_VALVEOUT;
static double temperature = INIT_TEMP;

double get_tank_temperature(void)
{
    return temperature;
}

int32_t get_tank_fill_percentage(void)
{
    return volume / MAX_VOLUME;
}

valve_status_t get_valveIN_status(void)
{
    return valveIN;
}

void set_valveIN_status(valve_status_t new_status)
{
    valveIN = new_status;
}

valve_status_t get_valveOUT_status(void)
{
    return valveOUT;
}

void set_valveOUT_status(valve_status_t new_status)
{
    valveOUT = new_status;
}

void update_simulated_tank_status(void)
{
    static uint32_t previous_temp_update_time = 0;
    uint32_t const now = time(NULL);
    int time_to_change_temperature = 0;

    if (previous_temp_update_time == 0)
    {
        previous_temp_update_time = time(NULL);
        srand(previous_temp_update_time);
    }

    time_to_change_temperature = now - previous_temp_update_time >= 5 ;

    if (time_to_change_temperature)
    {
        temperature = temperature - 1 + 1 * (rand() % 3);
        if (temperature < MIN_TEMP)
        {
            temperature = MIN_TEMP;
        }
        else if (temperature > MAX_TEMP)
        {
            temperature = MAX_TEMP;
        }
        previous_temp_update_time = now;
    }


    if (valveIN == VALVE_OPENED)
    {
        volume += VALVEIN_FLOW;
    }

    if (valveOUT == VALVE_OPENED)
    {
        volume -= VALVEOUT_FLOW;
    }

    if (volume < MIN_VOLUME)
    {
        volume = MIN_VOLUME;
    }
    else if (volume > MAX_VOLUME)
    {
        volume = MAX_VOLUME;
    }
}

void print_tank_status(void)
{
    printf("\n\tVolume = %d (%d%%)\n", volume, volume / MAX_VOLUME);
    printf("\tTemperature = %f\n", temperature);
    printf("\tValveIN = %s\n", valveIN == VALVE_OPENED ? "Opened" : "Closed");
    printf("\tValveOUT = %s\n\n", valveOUT == VALVE_OPENED ? "Opened" : "Closed");
}


