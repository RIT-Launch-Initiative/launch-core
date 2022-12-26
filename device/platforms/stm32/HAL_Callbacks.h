/**
 * Header for callback functions
 *
 * @author Aaron Chan
 */

#ifndef LAUNCH_CORE_HAL_CALLBACKS_H
#define LAUNCH_CORE_HAL_CALLBACKS_H

#include "stm32f4xx_hal_i2c.h"


void HAL_I2C_RxCpltCallback(I2C_HandleTypeDef *hi2c);

void HAL_I2C_TxCpltCallback(I2C_HandleTypeDef *hi2c);

#endif //LAUNCH_CORE_HAL_CALLBACKS_H
