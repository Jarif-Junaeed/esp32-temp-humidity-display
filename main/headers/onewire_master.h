// SPDX-FileCopyrightText: 2025 Jarif Junaeed <jarif_secure@proton.me>
// SPDX-License-Identifier: MIT

#ifndef ONEWIRE_MASTER_H
#define ONEWIRE_MASTER_H

#include "config.h"

void onewire_master_create_timer(isr_device_context_t *dev);
void onewire_master_start_sequence(isr_device_context_t *dev, uint32_t slot_period_us, uint32_t num_slots, uint32_t slack_us);
void onewire_master_stop_sequence(isr_device_context_t *dev);

#endif