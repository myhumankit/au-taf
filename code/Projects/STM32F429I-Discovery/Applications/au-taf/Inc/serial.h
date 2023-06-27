#ifndef SERIAL_H_
#define SERIAL_H_


#define RATE_30KHz 6000
#define RATE_33KHz 5400
#define RATE_36KHz 5000
#define RATE_38KHz 4737
#define RATE_56KHz 3214

void serialInit();
void spiInit();
void spi_send(uint8_t *data, int n);
void l3gd20_set_three_state(void);
void timerInit();
void mux_set_switch(int n);

void set_modulation(int rate);

uint32_t generate_philips(uint32_t time, uint32_t code, uint32_t *pData, int max);
uint32_t generate_akai(uint32_t time, uint32_t code, uint32_t *pData, int max);
uint32_t generate_rc5(uint32_t time, uint32_t code, uint32_t *pData, int max);
uint32_t generate_aaa(uint32_t time, uint32_t code, uint32_t *pData, int max);

uint32_t generate_perel(uint32_t time, uint32_t code, uint32_t *pData, int max);
void timer_send(uint32_t time, uint32_t *pData, int n);

void L2Loop(void);

enum {
      MUX_SWITCH_INFRA_RED,
      MUX_SWITCH_RF_433,
      MUX_SWITCH_SPARE_2,
      MUX_SWITCH_SPARE_3
};


#endif
