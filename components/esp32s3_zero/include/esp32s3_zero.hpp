#pragma once

#include <memory>

#include "base_component.hpp"
#include "interrupt.hpp"
#include "neopixel.hpp"

namespace espp {

/// Board support for the Waveshare ESP32-S3-Zero (NeoPixel on GPIO 21)
class Esp32s3Zero : public BaseComponent {
public:
  using button_callback_t = espp::Interrupt::event_callback_fn;

  /// GPIO for the NeoPixel data line
  static constexpr gpio_num_t led_data_io = GPIO_NUM_21;
  /// No separate power pin on this board
  static constexpr int led_power_io = -1;
  /// Boot button
  static constexpr gpio_num_t button_io = GPIO_NUM_0;
  /// Number of on-board LEDs
  static constexpr size_t num_leds_ = 1;

  /// Singleton accessor
  static Esp32s3Zero &get() {
    static Esp32s3Zero instance;
    return instance;
  }

  Esp32s3Zero(const Esp32s3Zero &) = delete;
  Esp32s3Zero &operator=(const Esp32s3Zero &) = delete;
  Esp32s3Zero(Esp32s3Zero &&) = delete;
  Esp32s3Zero &operator=(Esp32s3Zero &&) = delete;

  /// Initialize the NeoPixel LED
  bool initialize_led();

  /// Set LED color (RGB)
  bool led(const Rgb &rgb);

  /// Set LED color (HSV)
  bool led(const Hsv &hsv);

  /// Get the number of LEDs
  static constexpr size_t num_leds() { return num_leds_; }

  /// Get the Neopixel instance
  std::shared_ptr<Neopixel> led() const { return led_; }

  /// Initialize the boot button with an optional callback
  bool initialize_button(const button_callback_t &callback = nullptr);

  /// Read the boot button state
  bool button_state() const;

  /// Get a reference to the interrupt manager
  espp::Interrupt &interrupts() { return interrupts_; }

protected:
  Esp32s3Zero()
      : BaseComponent("Esp32s3Zero") {}

  std::shared_ptr<Neopixel> led_{nullptr};

  espp::Interrupt interrupts_{
      {.isr_core_id = 0, .task_config = {.name = "esp32s3z_isr", .stack_size_bytes = 4096}}};
};

} // namespace espp
