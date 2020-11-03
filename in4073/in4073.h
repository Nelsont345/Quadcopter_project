/*------------------------------------------------------------------
 *  in4073.h -- defines, globals, function prototypes
 *
 *  I. Protonotarios
 *  Embedded Software Lab
 *
 *  July 2016
 *------------------------------------------------------------------
 */

#ifndef IN4073_H__
#define IN4073_H__

#include <inttypes.h>
#include <stdio.h>
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "ml.h"
#include "app_util_platform.h"
#include <math.h>

#define RED		22
#define YELLOW		24
#define GREEN		28
#define BLUE		30
#define INT_PIN		5

#define SAFE		0
#define PANIC		1
#define MANUAL		2
#define CALIBRATION	3
#define YAW		4
#define FULL		5
#define RAW		6
#define HEIGHT		7
#define EXIT		8
#define DATASIZE        37

// logging
void flash_data();
void log_data();
bool demo_done;

//Mode

uint8_t mode;


uint8_t crc;
// Control
uint16_t throttle;
uint16_t f_throttle;
int16_t roll, pitch, yaw;
uint8_t P, P1, P2;
uint16_t Q;
int16_t motor[4],ae[4];
void run_filters_and_control();
int32_t cal_phi, cal_theta, cal_psi, cal_sp, cal_sq, cal_sr;
int16_t c_phi, c_theta, c_psi, c_sp, c_sq, c_sr;
int16_t y_err;
int32_t pitch_new, roll_new, yaw_new;
//uint32_t t_receive[256];
int32_t pitch_rate_err, roll_rate_err;     
int32_t pitch_angle_err, roll_angle_err;

//raw mode
uint8_t raw_mode;
int32_t processed_yaw;
int32_t prev_yaw_x[2];
int32_t prev_yaw_y[2];


//height mode
bool height_mode;
int32_t throttle_new;
int32_t fixed_pressure;


//kalman
int32_t bias;
int32_t error;
int32_t P2PHI; 
int32_t C1;
int32_t C2;

int32_t p_bias;
int32_t sphi;
int32_t phi_error;
int32_t p_kalman;
int32_t phi_kalman;

int32_t q_bias;
int32_t stheta;
int32_t theta_error;
int32_t q_kalman;
int32_t theta_kalman;


//kalman
int32_t bias;
int32_t error;
int32_t P2PHI; 
int32_t C1;
int32_t C2;


// Timers
#define TIMER_PERIOD	50 //50ms=20Hz (MAX 23bit, 4.6h)
void timers_init(void);
uint32_t get_time_us(void);
bool check_timer_flag(void);
void clear_timer_flag(void);
uint32_t last_receiving_time; 

// Profiling

uint32_t start_time, loop_time, tot_intr_time, prev_loop_time;
uint32_t intr_start_time, intr_stop_time;
uint32_t filter_start_time, filter_stop_time;
uint32_t cycle_time;
uint32_t response_time;


// GPIO
void gpio_init(void);

// Queue
#define QUEUE_SIZE 256
typedef struct {
	uint8_t Data[QUEUE_SIZE];
	uint16_t first,last;
  	uint16_t count; 
} queue;



void init_queue(queue *q);
void enqueue(queue *q, char x);
char dequeue(queue *q);

// UART
#define RX_PIN_NUMBER  16
#define TX_PIN_NUMBER  14
queue rx_queue;
queue tx_queue;
uint32_t last_correct_checksum_time;
void uart_init(void);
void uart_put(uint8_t);

// TWI
#define TWI_SCL	4
#define TWI_SDA	2
void twi_init(void);
bool i2c_write(uint8_t slave_addr, uint8_t reg_addr, uint8_t length, uint8_t const *data);
bool i2c_read(uint8_t slave_addr, uint8_t reg_addr, uint8_t length, uint8_t *data);

// MPU wrapper
int16_t phi, theta, psi;
int16_t sp, sq, sr;
int16_t sax, say, saz;
uint8_t sensor_fifo_count;
void imu_init(bool dmp, uint16_t interrupt_frequency); // if dmp is true, the interrupt frequency is 100Hz - otherwise 32Hz-8kHz
void get_dmp_data(void);
void get_raw_sensor_data(void);
bool check_sensor_int_flag(void);
void clear_sensor_int_flag(void);

// Barometer
int32_t pressure;
int32_t temperature;
void read_baro(void);
void baro_init(void);

// ADC
uint16_t bat_volt;
void adc_init(void);
void adc_request_sample(void);

// Flash
bool spi_flash_init(void);
bool flash_chip_erase(void);
bool flash_write_byte(uint32_t address, uint8_t data);
bool flash_write_bytes(uint32_t address, uint8_t *data, uint32_t count);
bool flash_read_byte(uint32_t address, uint8_t *buffer);
bool flash_read_bytes(uint32_t address, uint8_t *buffer, uint32_t count);

// BLE
queue ble_rx_queue;
queue ble_tx_queue;
volatile bool radio_active;
void ble_init(void);
void ble_send(void);

#endif // IN4073_H__
