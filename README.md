# Battery Powered Sensor
This project designed a battery powered IMU sensor. This sensor require to use a IMU, LSM6DSMTR, made by ST. This sensor require to be powered by battery and have a pushbutton to turn on and off.

## Table of Contents
1. [Hardware](hardware/README.md)
2. [Firmware](firmware/README.md)
3. [Parts Specifications](#parts-specifications)
4. [Operational Specifications](#operational-specifications)

## Parts Specifications
1. MCU
 - Chip Number: nRF52832
 - Module Number: PTR5618
2. Button
 - Single pole, momentary, normally open type
3. IMU
 - Part Number: [LSM6DSMTR](https://www.st.com/content/ccc/resource/technical/document/datasheet/76/27/cf/88/c5/03/42/6b/DM00218116.pdf/files/DM00218116.pdf/jcr:content/translations/en.DM00218116.pdf)
4. Battery:
 - Part number: [LP501218JH](https://www.jauch.com/downloadfile/5bf52a0e7827c7c5e9e1dbf47ce24c136/60mah_-_lp501218jh_1s1p_2_wire_50mm.pdf)

## Operational Specifications
1. Firmware requirements
 - Configure all that is required to communicate via SPI with the IMU
 - Configure the IMU
 - Sample the IMU at 833 hz, the sampled data can be discarded in this example.
 - ** NOTE: Keep the firmware simple, this is an example project and will not need to be maintained in the future.
2. IMU Configuration
 - SPI communication mode
 - Both accelerometer and gyro data should be sampled
 - Accelerometer range should be set to +-16g
 - Gyro range should be set to +-2000 dps
 - All other settings/features can be disabled or configured as desired/needed.
3. Button Operation
 - From the ‘OFF’ state: Press the button once to switch the device to the ‘ON’ state. The duration of the press is not important or specified.
 - From the ‘ON’ state: Press and hold the button for >500ms to switch the device to the ‘OFF’ state.
4. Power consumption
 - In the ‘OFF’ state, the device should not significantly drain the battery in 1 year. Design should minimize the power off current consumption.
 - In the ‘ON’ state, power consumption is not limited.
