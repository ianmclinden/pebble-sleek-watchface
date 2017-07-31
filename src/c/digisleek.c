#include "pebble.h"
#include <time.h>

struct date {
	char* weekday[32];
	char* day[32];
	char* month[32];
	char* date[48];
};

// Declare Window
static Window *s_main_window;

// Declare Layers
static TextLayer *s_time_layer, *s_date_layer, *s_battery_layer;
static BitmapLayer *s_batteryimg_layer, *s_batteryoverlay_layer, *s_bt_layer;

// Create Text Format Consts
static char battery_text[] = "100%";
static char s_time_text[] = "00:00";
static char s_date_text[48];
static char s_weekday_text[32];
static char s_daynum_text[32];
static char s_month_text[32];

// Declare Custom Fonts
static GFont roboto_condensed_58, open_sans_light_12, open_sans_14;

// Declare Icons
static GBitmap *batt_0, *batt_10, *batt_20, *batt_30, *batt_40, *batt_50, *batt_60, *batt_70, *batt_80, *batt_90, *batt_100, *batt_charging, *batt_overlay;
static GBitmap *bt_off, *bt_on;

// Declare Utility Vars
struct tm last_time;

/******************************** Battery Handlers ********************************/

static void handle_battery(BatteryChargeState charge_state) {

  if (charge_state.is_charging) {
		bitmap_layer_set_bitmap(s_batteryimg_layer, batt_charging);

  } else {
	switch (charge_state.charge_percent) {
		case 96 ... 100: bitmap_layer_set_bitmap(s_batteryimg_layer, batt_100);
			break;
		case 81 ... 95: bitmap_layer_set_bitmap(s_batteryimg_layer, batt_90);
			break;
		case 71 ... 80: bitmap_layer_set_bitmap(s_batteryimg_layer, batt_80);
			break;
		case 61 ... 70: bitmap_layer_set_bitmap(s_batteryimg_layer, batt_70);
			break;
		case 51 ... 60: bitmap_layer_set_bitmap(s_batteryimg_layer, batt_60);
			break;
		case 41 ... 50: bitmap_layer_set_bitmap(s_batteryimg_layer, batt_50);
			break;
		case 31 ... 40: bitmap_layer_set_bitmap(s_batteryimg_layer, batt_40);
			break;
		case 21 ... 30: bitmap_layer_set_bitmap(s_batteryimg_layer, batt_30);
			break;
		case 11 ... 20: bitmap_layer_set_bitmap(s_batteryimg_layer, batt_20);
			break;
		case 6 ... 10:  bitmap_layer_set_bitmap(s_batteryimg_layer, batt_10);
			break;
		case 0 ... 5:   bitmap_layer_set_bitmap(s_batteryimg_layer, batt_0);
			break;
	}
  }
  snprintf(battery_text, sizeof(battery_text), "%d%%", charge_state.charge_percent);
  text_layer_set_text(s_battery_layer, battery_text);
}

/******************************** Clock Use ********************************/

static void handle_second_tick(struct tm* tick_time, TimeUnits units_changed) {

  // Hour : Min
  if (clock_is_24h_style()) {
	  strftime(s_time_text, sizeof(s_time_text), "%T", tick_time);
	  text_layer_set_text(s_time_layer, s_time_text);
  } else {
	  strftime(s_time_text, sizeof(s_time_text), "%r", tick_time);
	  text_layer_set_text(s_time_layer, s_time_text);
  }
  // Month and day.
  if (tick_time->tm_mon != last_time.tm_mon || tick_time->tm_mday != last_time.tm_mday) {
	  
	  strftime(s_weekday_text, sizeof(s_weekday_text), "%A", tick_time);
	  strftime(s_daynum_text, sizeof(s_daynum_text), "%e", tick_time);
	  strftime(s_month_text, sizeof(s_month_text), "%b", tick_time);
	  
	  snprintf(s_date_text, sizeof(s_date_text), "%s,  %s %s", s_weekday_text, s_month_text, s_daynum_text);
	  
	  text_layer_set_text(s_date_layer, s_date_text);
  }

  handle_battery(battery_state_service_peek());
  last_time = *tick_time;
}

/******************************** Bluetooth Handlers ********************************/

static void handle_bluetooth(bool connected) {
  bitmap_layer_set_bitmap(s_bt_layer, connected ? bt_on : bt_off);
	if (!connected) {
		vibes_short_pulse();
	}
}

/******************************** Main Load ********************************/

static void main_window_load(Window *window) {
	
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
	
  roboto_condensed_58 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ROBOTO_CONDENSED_58));
  open_sans_light_12 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_OPEN_SANS_LIGHT_12));
  open_sans_14 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_OPEN_SANS_14));
		
  // Create Time Layer
  s_time_layer = text_layer_create(GRect(0, 38, bounds.size.w, 62));
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_font(s_time_layer, roboto_condensed_58);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
	
  // Create Date Layer
  s_date_layer = text_layer_create(GRect(0, 100, bounds.size.w, 24));
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_font(s_date_layer, open_sans_14);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
	
  //Create Battery % Layer
  s_battery_layer = text_layer_create(GRect((bounds.size.w-60), 3, 32, 12));
  text_layer_set_text_color(s_battery_layer, GColorWhite);
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_font(s_battery_layer, open_sans_light_12);
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentRight);
  text_layer_set_text(s_battery_layer, "100%");
	
  // Create BT Bitmaps
  bt_off = gbitmap_create_with_resource(RESOURCE_ID_BT_OFF);
  bt_on = gbitmap_create_with_resource(RESOURCE_ID_BT_ON);
  s_bt_layer = bitmap_layer_create(GRect(3, 4, 12, 12));
  bitmap_layer_set_bitmap(s_bt_layer, bt_on);
	
  // Create Battery % Bitmaps
  batt_0 = gbitmap_create_with_resource(RESOURCE_ID_BATT_0);
  batt_10 = gbitmap_create_with_resource(RESOURCE_ID_BATT_10);
  batt_20 = gbitmap_create_with_resource(RESOURCE_ID_BATT_20);
  batt_30 = gbitmap_create_with_resource(RESOURCE_ID_BATT_30);
  batt_40 = gbitmap_create_with_resource(RESOURCE_ID_BATT_40);
  batt_50 = gbitmap_create_with_resource(RESOURCE_ID_BATT_50);
  batt_60 = gbitmap_create_with_resource(RESOURCE_ID_BATT_60);
  batt_70 = gbitmap_create_with_resource(RESOURCE_ID_BATT_70);
  batt_80 = gbitmap_create_with_resource(RESOURCE_ID_BATT_80);
  batt_90 = gbitmap_create_with_resource(RESOURCE_ID_BATT_90);
  batt_100 = gbitmap_create_with_resource(RESOURCE_ID_BATT_100);	
  batt_charging = gbitmap_create_with_resource(RESOURCE_ID_BATT_CHARGING);
  batt_overlay = gbitmap_create_with_resource(RESOURCE_ID_BATT_OVERLAY_WHITE);
  s_batteryimg_layer = bitmap_layer_create(GRect((bounds.size.w-26), 4, 24, 12));
  bitmap_layer_set_bitmap(s_batteryimg_layer, batt_100);
  s_batteryoverlay_layer = bitmap_layer_create(GRect((bounds.size.w-26), 4, 24, 12));
  bitmap_layer_set_bitmap(s_batteryoverlay_layer, batt_overlay);
  bitmap_layer_set_compositing_mode(s_batteryoverlay_layer, GCompOpOr);
	
  // Ensures time is displayed immediately (will break if NULL tick event accessed).
  // (This is why it's a good idea to have a separate routine to do the update itself.)
  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_second_tick(current_time, SECOND_UNIT);

  tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
  battery_state_service_subscribe(handle_battery);
  bluetooth_connection_service_subscribe(handle_bluetooth);
	
  // Add layers to window	
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_battery_layer));
  layer_add_child(window_layer, bitmap_layer_get_layer(s_batteryimg_layer));
  layer_add_child(window_layer, bitmap_layer_get_layer(s_batteryoverlay_layer));
  layer_add_child(window_layer, bitmap_layer_get_layer(s_bt_layer));

}

/******************************** Main Unload ********************************/

static void main_window_unload(Window *window) {
  // Unsubscribe From Services
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();

  // Destroy Layers
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  bitmap_layer_destroy(s_batteryimg_layer);
  bitmap_layer_destroy(s_bt_layer);
  text_layer_destroy(s_battery_layer);
  bitmap_layer_destroy(s_batteryoverlay_layer);
	
  // Destroy Image Pointers
  gbitmap_destroy(batt_0);
  gbitmap_destroy(batt_10);
  gbitmap_destroy(batt_20);
  gbitmap_destroy(batt_30);
  gbitmap_destroy(batt_40);
  gbitmap_destroy(batt_50);
  gbitmap_destroy(batt_60);
  gbitmap_destroy(batt_70);
  gbitmap_destroy(batt_80);
  gbitmap_destroy(batt_90);
  gbitmap_destroy(batt_100);
  gbitmap_destroy(batt_charging);
  gbitmap_destroy(bt_off);
  gbitmap_destroy(bt_on);
  gbitmap_destroy(batt_overlay);
	
}

/******************************** Maintenance ********************************/

static void init() {
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
//   app_message_open(64, 64);
}

static void deinit() {
  window_destroy(s_main_window);
//   app_sync_deinit(&s_sync);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
