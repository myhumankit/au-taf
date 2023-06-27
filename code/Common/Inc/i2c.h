typedef struct {
  GPIO_TypeDef  *sclGpio;
  uint16_t      sclPin;
  GPIO_TypeDef  *sdaGpio;
  uint16_t      sdaPin;
  uint32_t      delay_us;
} i2c_config_t;

void wait_us(int32_t n);
void hw_i2c_init(i2c_config_t *pI2c);
void i2c_read(i2c_config_t *pI2c, uint8_t address, uint8_t *pData, int length);
void i2c_write(i2c_config_t *pI2c, uint8_t address,
	       uint8_t *pData_write, int length_write);
void i2c_read_extended(i2c_config_t *pI2c, uint8_t address,
		       uint8_t *pData_write, int length_write,
		       uint8_t *pData_read, int length_read);
