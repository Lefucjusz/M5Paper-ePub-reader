#pragma once

#define GT911_I2C_ADDRESS 0x5D
#define GT911_REG_ADDR_SIZE_BYTES 2

#define GT911_TOUCH_STATUS_REG 0x814E
#define GT911_TOUCH_POINTS_COUNT_MASK 0x0F
#define GT911_TOUCH_DATA_VALID_MASK 0x80

#define GT911_POINT_1_X_COORD_LSB 0x8150
#define GT911_POINT_1_X_COORD_MSB 0x8151
#define GT911_POINT_1_Y_COORD_LSB 0x8152
#define GT911_POINT_1_Y_COORD_MSB 0x8153
#define GT911_POINT_1_SIZE_LSB 0x8154
#define GT911_POINT_1_SIZE_MSB 0x8155
#define GT911_POINT_1_RESERVED 0x8156
#define GT911_POINT_1_TRACK_ID 0x8157
#define GT911_POINT_2_X_COORD_LSB 0x8158
#define GT911_POINT_2_X_COORD_MSB 0x8159
#define GT911_POINT_2_Y_COORD_LSB 0x815A
#define GT911_POINT_2_Y_COORD_MSB 0x815B
#define GT911_POINT_2_SIZE_LSB 0x815C
#define GT911_POINT_2_SIZE_MSB 0x815D
#define GT911_POINT_2_RESERVED 0x815E
#define GT911_POINT_2_TRACK_ID 0x815F

#define GT911_TOUCH_COORDS_SIZE 4
#define GT911_TOUCH_DATA_SIZE 8
