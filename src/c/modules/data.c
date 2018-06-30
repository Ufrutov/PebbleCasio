#include "data.h"

typedef enum {
  AppKeyCurrentAverage = 0,
  AppKeyDailyAverage,
  AppKeyCurrentSteps
} AppKey;

typedef enum {
  AverageTypeCurrent = 0,
  AverageTypeDaily
} AverageType;

static GBitmap *s_casio_bckg, *s_pawprint_green;
static GFont s_font_small, s_font_big, s_font_med, s_font_cristal, s_font_digital;

static int s_current_steps, s_daily_average, s_current_average;
static char s_current_steps_buffer[8];

static char s_current_battery_level[5];

static void update_average(AverageType type) {
  // Start time is midnight
  const time_t start = time_start_of_today();

  time_t end = start;
  int steps = 0;
  switch(type) {
    case AverageTypeDaily:
      // One whole day
      end = start + SECONDS_PER_DAY;
      break;
    case AverageTypeCurrent:
      // Time from midnight to now
      end = start + (time(NULL) - time_start_of_today());
      break;
    default:
      if(DEBUG) APP_LOG(APP_LOG_LEVEL_ERROR, "Unknown average type!");
      break;
  } 

  // Check the average data is available
  HealthServiceAccessibilityMask mask = health_service_metric_averaged_accessible(
                                HealthMetricStepCount, start, end, HealthServiceTimeScopeDaily);
  if(mask & HealthServiceAccessibilityMaskAvailable) {
    // Data is available, read it
    steps = (int)health_service_sum_averaged(HealthMetricStepCount, start, end, 
                                                                    HealthServiceTimeScopeDaily);
  } else {
    if(DEBUG) APP_LOG(APP_LOG_LEVEL_DEBUG, "No data available for daily average");
  }

  // Store the calculated value
  switch(type) {
    case AverageTypeDaily:
      s_daily_average = steps;
      persist_write_int(AppKeyDailyAverage, s_daily_average);

      if(DEBUG) APP_LOG(APP_LOG_LEVEL_DEBUG, "Daily average: %d", s_daily_average);
      break;
    case AverageTypeCurrent:
      s_current_average = steps;
      persist_write_int(AppKeyCurrentAverage, s_current_average);

      if(DEBUG) APP_LOG(APP_LOG_LEVEL_DEBUG, "Current average: %d", s_current_average);
      break;
    default: break;  // Handled by previous switch
  }
}

void data_update_steps_buffer() {
  int thousands = s_current_steps / 1000;
  int hundreds = s_current_steps % 1000;
  if(thousands > 0) {
    snprintf(s_current_steps_buffer, sizeof(s_current_steps_buffer), "%d,%03d", thousands, hundreds);
  } else {
    snprintf(s_current_steps_buffer, sizeof(s_current_steps_buffer), "%d", hundreds);
  }
  
  int b = battery_state_service_peek().charge_percent;
  snprintf(s_current_battery_level, sizeof(s_current_battery_level), "%d%%", b);

  main_window_redraw();
}

static void load_health_data_handler(void *context) {
  const struct tm *time_now = util_get_tm();

  s_current_steps = health_service_sum_today(HealthMetricStepCount);
  persist_write_int(AppKeyCurrentSteps, s_current_steps);

  update_average(AverageTypeDaily);
  update_average(AverageTypeCurrent);

  data_update_steps_buffer();
}

void data_reload_averages() {
  app_timer_register(LOAD_DATA_DELAY, load_health_data_handler, NULL);
}

void data_init() {
  // Load resources
  s_casio_bckg = gbitmap_create_with_resource(RESOURCE_ID_CASIO_BCKG);
  s_pawprint_green = gbitmap_create_with_resource(RESOURCE_ID_GREEN_PAWSTEP);
  s_font_small = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  s_font_med = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  s_font_big = fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK);
  s_font_cristal = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_CRISTAL_16));
  s_font_digital = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIGITAL_42));

  // First time persist
  if(!persist_exists(AppKeyCurrentSteps)) {
    s_current_steps = 0;
    s_current_average = 0;
    s_daily_average = 0;
  } else {
    s_current_average = persist_read_int(AppKeyCurrentAverage);
    s_daily_average = persist_read_int(AppKeyDailyAverage);
    s_current_steps = persist_read_int(AppKeyCurrentSteps);
  }
  data_update_steps_buffer();

  // Avoid half-second delay loading the app by delaying API read
  data_reload_averages();
}

void data_deinit() {
  gbitmap_destroy(s_casio_bckg);
  gbitmap_destroy(s_pawprint_green);
}

int data_get_current_steps() {
  return s_current_steps;
}

int data_get_current_average() {
  return s_current_average;
}

int data_get_daily_average() {
  return s_daily_average;
}

void data_set_current_steps(int value) {
  s_current_steps = value;
}

void data_set_current_average(int value) {
  s_current_average = value;
}

void data_set_daily_average(int value) {
  s_daily_average = value;
}

GFont data_get_font(FontSize size) {
  switch(size) {
    case FontSizeSmall:  return s_font_small;
    case FontSizeMedium: return s_font_med;
    case FontSizeLarge:  return s_font_big;
    case FontSizeCristal:  return s_font_cristal;
    case FontSizeDigital: return s_font_digital;
    default: return s_font_small;
  }
}

GBitmap* data_get_bckg() {
  return s_casio_bckg;
}

GBitmap* data_get_green_step() {
  return s_pawprint_green;
}

char* data_get_current_steps_buffer() {
  return s_current_steps_buffer;
}

char* data_get_batter_level() {
  return s_current_battery_level;
}
