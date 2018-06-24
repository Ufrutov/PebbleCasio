#pragma once

#include <pebble.h>

#include "../config.h"

#include "../modules/util.h"

#include "../windows/main_window.h"

typedef enum {
  FontSizeSmall = 0,
  FontSizeMedium,
  FontSizeLarge,
  FontSizeDigital,
  FontSizeCristal = 24,
} FontSize;

void data_init();
void data_deinit();

int data_get_current_steps();
void data_set_current_steps(int value);

int data_get_current_average();
void data_set_current_average(int value);

int data_get_daily_average();
void data_set_daily_average(int value);

void data_reload_averages();

GFont data_get_font(FontSize size);

GBitmap* data_get_bckg();

GBitmap* data_get_green_step();

void data_update_steps_buffer();

char* data_get_current_steps_buffer();
