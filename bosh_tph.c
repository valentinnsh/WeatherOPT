/*
 * Copyright (c) 2018 V.Shishkin
 *
 */


/*
 * Compile my cool program by
 *
 * arm-linux-gnueabihf-gcc-5 -g -W -Wall -o bosh_tph bosh_tph.c
 *
 */
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include <stdint.h>
#define BME280_ADDRESS 0x76 //from documentation

struct BME280_s {
  char* name;
  int file_po;

  int32_t t_fine;
  
  /*Compensation parameter storage*/
  //Temperature
  uint16_t dig_T1;
  int16_t dig_T2;
  int16_t dig_T3;

  //Pressaure
  uint16_t dig_P1;
  int16_t dig_P2;
  int16_t dig_P3;
  int16_t dig_P4;
  int16_t dig_P5;
  int16_t dig_P6;
  int16_t dig_P7;
  int16_t dig_P8;
  int16_t dig_P9;

  //Humisity
  uint8_t dig_H1;
  int16_t dig_H2;
  uint8_t dig_H3;
  int16_t dig_H4;
  int16_t dig_H5;
  int8_t dig_H6;
};


typedef struct BME280_s bme280_t;
int bme280_read_array(bme280_t *ctx, uint8_t register_num, uint8_t *arr, uint8_t len);
int32_t bme280_comp_temp_32bit(bme280_t *dev, uint32_t uncomp_temp);
uint32_t bme280_comp_pres_int32(bme280_t *dev, uint32_t adc_P);
uint32_t bme280_comp_hum_int32(bme280_t *dev, uint32_t adc_H);

int open_BME_sensor(bme280_t *ctx)
{
  uint8_t buf[32];
  int check;

  ctx->name = "/dev/i2c-0";

  //Open up the I2C
  /*
   *
   * Shaman stuff ahead
   *
   */
  ctx->file_po = open(ctx->name, O_RDWR);
  if (ctx->file_po == -1)
  {
    perror(ctx->name);
    return -1;
  }

  // Specify the address of the slave device.(Another shaman stuff)
  if (ioctl(ctx->file_po, I2C_SLAVE, BME280_ADDRESS) < 0)
  {
    perror("Failed to acquire bus access and/or talk to slave");
    return -1;
  }

  check = bme280_read_array(ctx, 0x88, buf, 25);
  if(check < 0)
  {
    perror("Failed to read Compensation parameters from T1 to H1, including P1..9");
    return -1;
  }
  //Compensation parameters for temperature
  ctx->dig_T1 = ((uint16_t)buf[1]<<8)|((uint16_t)buf[0]);
  ctx->dig_T2 = (((int16_t)buf[3])<<8)|((int16_t)buf[2]);
  ctx->dig_T3 = (((int16_t)buf[5])<<8)|((int16_t)buf[4]);

  //Compensation parameters for pressure
  ctx->dig_P1 = (((uint16_t)buf[7])<<8)|((uint16_t)buf[6]);
  ctx->dig_P2 = (((int16_t)buf[9])<<8)|((int16_t)buf[8]);
  ctx->dig_P3 = (((int16_t)buf[11])<<8)|((int16_t)buf[10]);
  ctx->dig_P4 = (((int16_t)buf[13])<<8)|((int16_t)buf[12]);
  ctx->dig_P5 = (((int16_t)buf[15])<<8)|((int16_t)buf[14]);
  ctx->dig_P6 = (((int16_t)buf[17])<<8)|((int16_t)buf[16]);
  ctx->dig_P7 = (((int16_t)buf[19])<<8)|((int16_t)buf[18]);
  ctx->dig_P8 = (((int16_t)buf[21])<<8)|((int16_t)buf[20]);
  ctx->dig_P9 = (((int16_t)buf[23])<<8)|((int16_t)buf[22]);


  //Compensation parameters for humidity
  ctx->dig_H1 = (uint8_t)(buf[24]);
  //reading other 5 parameters for humidity
  check = bme280_read_array(ctx, 0xE1, buf, 7);
  if(check < 0)
  {
    perror("Failed to read Compensation parameters from H2 to H6");
    return -1;
  }
  ctx->dig_H2 = ((int16_t)(buf[1]<<8))|((int16_t)buf[0]);
  ctx->dig_H3 = (uint8_t)(buf[2]);
  ctx->dig_H4 = ((int16_t)buf[3]<<4)|((int16_t)buf[4]&15);
  ctx->dig_H5 = ((int16_t)buf[5]<<4)|((int16_t)buf[4]&240);
  ctx->dig_H6 = (int8_t)buf[6];
  return 0;
}



int write_to_register(int file, unsigned char register_num, unsigned char to_register)
{
  int check;
  unsigned char buf[2];

  buf[0] = register_num;
  buf[1] = to_register;

  check = write(file, buf, 2);
  if(check != 2){
    perror("Failed to write to the i2c bus");
    return -1;
  }
  return 0;
}


int read_from_one_register(int file, unsigned char register_num, unsigned char *result)
{

  if(write(file, &register_num, 1) < 0)
  {
    perror("Fail, while writing to single register in read_from_one_register function");
    return -1;
  }

  if(read(file, result, 1) < 0)
  {
    perror("Fail, while reading from single register in read_from_one_register function");
    return -1;
  }

  return 0;
}

int bme280_read_array(bme280_t *ctx, uint8_t register_num, uint8_t *arr, uint8_t len)
{
  int check;

  if(write(ctx->file_po, &register_num, 1) < 0)
  {
    perror("Fail, while reading from single register in read_from_one_register function");
    return -1;
  }

  check = read(ctx->file_po, arr, len);
  if (check < 0)
  {
    perror("Failed to read from the i2c bus");
    return -1;
  }

  return 0;
}

//getting measurements
int bme280_getmgrmt(bme280_t *ctx, int32_t *temp, uint32_t *pres, uint32_t *hum)
{
  uint8_t buf[24];
  int check;
  uint32_t uncomp_temp;
  uint32_t uncomp_pres;
  uint32_t uncomp_hum;

  check = bme280_read_array(ctx, 0xF7, buf, 8);
  if(check < 0)
  {
    perror("Failed to read uncompensate temp, hum and pres");
    return -1;
  }

  //calculating uncoprensate temperature value. TOCHECK: what is it without copensate?
  uncomp_temp = (((uint32_t)buf[3])<<12)|(((uint32_t)buf[4])<<4)|(((uint32_t)buf[5])>>4);

  //calculating uncoprensate humidity value. TOCHECK: what is it without copensate?
  uncomp_hum = ((uint32_t)buf[6]<<8)|((uint32_t)buf[7]);

  //calculating uncoprensate pressure value
  uncomp_pres = (((uint32_t)buf[0])<<12)|(((uint32_t)buf[1])<<4)|(((uint32_t)buf[2])>>4);

  *temp = bme280_comp_temp_32bit(ctx,uncomp_temp);
  *pres = bme280_comp_pres_int32(ctx,uncomp_pres);
  *hum = bme280_comp_hum_int32(ctx,uncomp_hum);
  return 0;
}


//returns temp in grad.[grad_dole] form. 2515 for example is 25.15 gradC
int32_t bme280_comp_temp_32bit(bme280_t *dev, uint32_t uncomp_temp)
{
  int32_t var1;
  int32_t var2;
  int32_t temperature = 0;

  var1 = ((((uncomp_temp >> 3) - ((int32_t) dev->dig_T1 << 1)))
          * ((int32_t) dev->dig_T2)) >> 11;
  var2 = (((((uncomp_temp >> 4) - ((int32_t) dev->dig_T1))
            * ((uncomp_temp >> 4) - ((int32_t) dev->dig_T2))) >> 12)
          * ((int32_t) dev->dig_T3)) >> 14;
  dev->t_fine = var1 + var2;
  temperature = (dev->t_fine * 5 + 128) >> 8;

  return temperature;
}



//Returns pressure in pascals
uint32_t bme280_comp_pres_int32(bme280_t *dev, uint32_t adc_P)
{
  int32_t var1, var2;
  uint32_t p;
  var1 = (((int32_t)(dev->t_fine))>>1) - (int32_t)64000;
  var2 = (((var1>>2) * (var1>>2)) >> 11 ) * ((int32_t)(dev->dig_P6));
  var2 = var2 + ((var1*((int32_t)(dev->dig_P5)))<<1);
  var2 = (var2>>2)+(((int32_t)(dev->dig_P4))<<16);
  var1 = ((((dev->dig_P3) * (((var1>>2) * (var1>>2)) >> 13 )) >> 3) + ((((int32_t)(dev->dig_P2)) * var1)>>1))>>18;
  var1 =((((32768+var1))*((int32_t)(dev->dig_P1)))>>15);
  if (var1 == 0)
  {
    return 0; // avoid exception caused by division by zero
  }
  p = (((uint32_t)(((int32_t)1048576)-adc_P)-(var2>>12)))*3125;
  if (p < 0x80000000)
  {
    p = (p << 1) / ((uint32_t)var1);
  }
  else
  {
    p = (p / (uint32_t)var1) * 2;
  }
  var1 = (((int32_t)dev->dig_P9) * ((int32_t)(((p>>3) * (p>>3))>>13)))>>12;
  var2 = (((int32_t)(p>>2)) * ((int32_t)dev->dig_P8))>>13;
  p = (uint32_t)((int32_t)p + ((var1 + var2 + dev->dig_P7) >> 4));
  return p;
}


//
uint32_t bme280_comp_hum_int32(bme280_t *dev, uint32_t adc_H)
{
  int32_t v_x1_u32r;
  v_x1_u32r = (dev->t_fine - ((int32_t)76800));
  v_x1_u32r = (((((adc_H << 14) - (((int32_t)dev->dig_H4) << 20) - (((int32_t)dev->dig_H5) * v_x1_u32r)) +
                 ((int32_t)16384)) >> 15) * (((((((v_x1_u32r * ((int32_t)dev->dig_H6)) >> 10) * (((v_x1_u32r *
((int32_t)dev->dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) *
((int32_t)dev->dig_H2) + 8192) >> 14));
  v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)dev->dig_H1)) >> 4));
  v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
  v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
  return (uint32_t)(v_x1_u32r>>12);
}
//
//
//
//  MAIN AHEAD
//
//
//
int main(int argc, char **argv)
{
  bme280_t sens;
  uint8_t buf[32];

  int check_BME;

  int32_t ch_temp = 0;
  uint32_t ch_hum;
  uint32_t ch_pres;

  check_BME = open_BME_sensor(&sens);
  if (check_BME == -1){
    printf("\nError occured, while openning sensor as i2c slave\n");
  }

  buf[0] = 0xf5;              /* config */
  buf[1] = (4 << 1)                 /* filter coeff 3 */
    | (1<<4);                   /* 62.5ms standby */

  if (write(sens.file_po, buf, 2) != 2){
    perror("Failed to write to the i2c bus");
    exit(1);
  }

  buf[0] = 0xf2;              /* ctrl_hum */
  buf[1] = 3;                 /* x8 */
  if (write(sens.file_po, buf, 2) != 2){
    perror("Failed to write to the i2c bus");
    exit(1);
  }

  unsigned char mode = (3 /* normal mode */ //1 or 2 - fast mode, 0 - sleep mode
                        | (4<<2) /* press oversampling * 8 */
                        | (4<<5) /* temp oversampling * 8 */
    );
  /*  */
  buf[0] = 0xf4;              /* ctrl_meas */
  buf[1] = mode;

  if (write(sens.file_po, buf, 2) != 2)
  {
    perror("Failed to write to the i2c bus");
    exit(1);
  }

  check_BME = bme280_getmgrmt(&sens, &ch_temp, &ch_pres, &ch_hum);
  printf("Temp = %f gradC\nPres = %fmmHg\nHum = %f procent\n", ((double)ch_temp)/100, ((double)ch_pres)/133.322, ((double)ch_hum)/1024.);
  //TODO - Add compensation functions, that already returns mesurements as a double
  return 0;
}

