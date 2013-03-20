/******************************************************************************
**+-------------------------------------------------------------------------+**
**|                            ****                                         |**
**|                            ****                                         |**
**|                            ******o***                                   |**
**|                      ********_///_****                                  |**
**|                      ***** /_//_/ ****                                  |**
**|                       ** ** (__/ ****                                   |**
**|                           *********                                     |**
**|                            ****                                         |**
**|                            ***                                          |**
**|                                                                         |**
**|         Copyright (c) 1998-2006 Texas Instruments Incorporated          |**
**|                        ALL RIGHTS RESERVED                              |**
**|                                                                         |**
**| Permission is hereby granted to licensees of Texas Instruments          |**
**| Incorporated (TI) products to use this computer program for the sole    |**
**| purpose of implementing a licensee product based on TI products.        |**
**| No other rights to reproduce, use, or disseminate this computer         |**
**| program, whether in part or in whole, are granted.                      |**
**|                                                                         |**
**| TI makes no representation or warranties with respect to the            |**
**| performance of this computer program, and specifically disclaims        |**
**| any responsibility for any damages, special or consequential,           |**
**| connected with the use of this program.                                 |**
**|                                                                         |**
**+-------------------------------------------------------------------------+**
******************************************************************************/

/**
 *  \file   i2cParams_evmdm6437.c
 *
 *  \brief  Paramter file for I2C driver
 *
 *  This file contains the configuration paramters for I2C driver
 *
 *  (C) Copyright 2006, Texas Instruments, Inc
 *
 *  \note     Set tabstop to 4 (:se ts=4) while viewing this file in an
 *            editor
 *
 *  \author   Maulik Desai
 *
 *  \version  0.1   Created
 */

#include <ti/sdo/pspdrivers/drivers/i2c/psp_i2c.h>

/* Initialization Parameters                        */
#define I2C_OWN_ADDR                (0x10u)
#define I2C_BUS_FREQ                (200000u)
#define I2C_INPUT_BUS_FREQ          (27000000u)
#define I2C_NUM_BITS                (8u)

/* Default Initial Parameters */
PSP_I2cConfig I2C_devParams =   {
                                    /** Driver operation mode                       */
                                    PSP_OPMODE_POLLED,
                                    /**< Own address (7 or 10 bit)                  */
                                    I2C_OWN_ADDR,
                                    /**< Number of bits/byte to be sent/received    */
                                    I2C_NUM_BITS,
                                    /**< I2C Bus Frequency                          */
                                    I2C_BUS_FREQ,
                                    /**< Module input clock freq                    */
                                    I2C_INPUT_BUS_FREQ,
                                    /**< 7bit/10bit Addressing mode                 */
                                    FALSE,
                                    /**< Digital Loob Back (DLB) mode enabled       */
                                    FALSE
                                };
