#pragma once

/* Defines file for IT8951 controller used in M5Paper */

/* Command defines */
/* Built-in I80 command codes */
#define IT8951_TCON_SYS_RUN 0x0001
#define IT8951_TCON_STANDBY 0x0002
#define IT8951_TCON_SLEEP   0x0003
#define IT8951_TCON_REG_RD  0x0010
#define IT8951_TCON_REG_WR  0x0011

#define IT8951_TCON_MEM_BST_RD_T 0x0012
#define IT8951_TCON_MEM_BST_RD_S 0x0013
#define IT8951_TCON_MEM_BST_WR   0x0014
#define IT8951_TCON_MEM_BST_END  0x0015

#define IT8951_TCON_LD_IMG      0x0020
#define IT8951_TCON_LD_IMG_AREA 0x0021
#define IT8951_TCON_LD_IMG_END  0x0022

/* I80 user-defined command codes */
#define IT8951_I80_CMD_DPY_AREA     0x0034
#define IT8951_I80_CMD_GET_DEV_INFO 0x0302
#define IT8951_I80_CMD_DPY_BUF_AREA 0x0037
#define IT8951_I80_CMD_VCOM         0x0039

/* SPI transmission preamble words */
#define IT8951_SPI_WRITE_DATA_PREAMBLE 0x0000
#define IT8951_SPI_READ_DATA_PREAMBLE  0x1000
#define IT8951_SPI_CMD_PREAMBLE        0x6000

/* Mode defines */
/* Rotation mode */
#define IT8951_ROTATION_0   0
#define IT8951_ROTATION_90  1
#define IT8951_ROTATION_180 2
#define IT8951_ROTATION_270 3

/* Pixel mode */
#define IT8951_2BPP 0
#define IT8951_3BPP 1
#define IT8951_4BPP 2
#define IT8951_8BPP 3

/* Endianness */
#define IT8951_LDIMG_L_ENDIAN 0
#define IT8951_LDIMG_B_ENDIAN 1

/* Shifts */
#define IT8951_ROTATION_SHIFT 0
#define IT8951_PIXEL_MODE_SHIFT 4
#define IT8951_ENDIANNESS_SHIFT 8

/* Register defines */
/* Display registers */
#define IT8951_DISPLAY_REG_BASE 0x1000
#define IT8951_LUTAFSR (IT8951_DISPLAY_REG_BASE + 0x224)

/* System registers */
#define IT8951_SYS_REG_BASE 0x0000
#define IT8951_I80CPCR (IT8951_SYS_REG_BASE + 0x0004)

/* Memory converter registers */
#define IT8951_MCSR_BASE_ADDR 0x0200
#define IT8951_MCSR (IT8951_MCSR_BASE_ADDR + 0x0000)
#define IT8951_LISAR (IT8951_MCSR_BASE_ADDR + 0x0008)
