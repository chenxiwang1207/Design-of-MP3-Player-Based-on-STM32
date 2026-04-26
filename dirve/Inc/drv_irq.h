#ifndef _DRV_IRQ_H_
#define _DRV_IRQ_H_

#include "stm32f10x.h"                  // Device header
#include "com_type.h"

void drv_irq_register_callback(s32 irq_no, user_irq_callback fun,void *arg);

#endif
