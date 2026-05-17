# [ESP32] ESPHome Touch Panel — LVGL UI with MQTT

This project implements a **touch-enabled control panel** using the **ESP32-2432S028R** (2.8" TFT display with resistive touch) running **ESPHome** with an **LVGL** UI.

The panel shows the **water tank level** as a color-coded fill bar and provides **6 toggle buttons** to control lights and appliances via MQTT. State is synchronized bidirectionally: pressing a button publishes a command topic, and incoming state topics update the button's checked state automatically.

---

## Hardware

- [2.8" ESP32-2432S028R (ESP32 + ILI9341 + XPT2046)](https://pt.aliexpress.com/item/1005007574258198.html) — all-in-one board with integrated display, touch, and Wi-Fi

---

## Pin Mapping

The ESP32-2432S028R has the display and touch controller wired to fixed GPIOs on the board. No external wiring is required.

### Display (ILI9341 — SPI)

| Signal    | GPIO   |
| --------- | ------ |
| SPI CLK   | GPIO14 |
| SPI MOSI  | GPIO13 |
| SPI MISO  | GPIO12 |
| CS        | GPIO15 |
| DC        | GPIO2  |
| Backlight | GPIO21 |

> RST is tied to 3.3 V on the board (no GPIO needed).

### Touchscreen (XPT2046 — SPI)

| Signal    | GPIO   |
| --------- | ------ |
| SPI CLK   | GPIO25 |
| SPI MOSI  | GPIO32 |
| SPI MISO  | GPIO39 |
| CS        | GPIO33 |
| IRQ       | GPIO36 |

---

## UI Layout (240 × 320 portrait)

```
┌─────────────────────────────┐
│       WATER TANK  (~107 px) │  ← fill bar + percentage label
├──────────────┬──────────────┤
│ Varanda      │ Varanda      │
│ Quartos      │              │  ← row 1
├──────────────┼──────────────┤
│ Jardim       │ Churras-     │
│              │ queira       │  ← row 2
├──────────────┼──────────────┤
│ Filtro       │ Luzes        │
│              │ Piscina      │  ← row 3
└──────────────┴──────────────┘
```

- **Water tank bar**: fills from bottom to top; color changes from blue → orange → red as the level drops below 50 % and 30 %.
- **Buttons**: checkable (toggle); checked state is highlighted in amber/gold.

---

## MQTT Topics

| Entity          | Command topic                              | State topic                               | Payload      |
| --------------- | ------------------------------------------ | ----------------------------------------- | ------------ |
| Water tank      | —                                          | `chacara/painel/agua/state`               | `0`–`100`    |
| Varanda Quartos | `chacara/painel/varanda_quartos/cmd`       | `chacara/painel/varanda_quartos/state`    | `ON` / `OFF` |
| Varanda         | `chacara/painel/varanda/cmd`               | `chacara/painel/varanda/state`            | `ON` / `OFF` |
| Jardim          | `chacara/painel/jardim/cmd`                | `chacara/painel/jardim/state`             | `ON` / `OFF` |
| Churrasqueira   | `chacara/painel/churrasqueira/cmd`         | `chacara/painel/churrasqueira/state`      | `ON` / `OFF` |
| Filtro          | `chacara/painel/filtro/cmd`                | `chacara/painel/filtro/state`             | `ON` / `OFF` |
| Luzes Piscina   | `chacara/painel/luzes_piscina/cmd`         | `chacara/painel/luzes_piscina/state`      | `ON` / `OFF` |

---

## Secrets

The following entries are required in your ESPHome `secrets.yaml`:

```yaml
wifi_ssid: "your_wifi_ssid"
wifi_password: "your_wifi_password"
api_key: "your_esphome_api_encryption_key"
mqtt_username: "your_mqtt_user"
mqtt_password: "your_mqtt_password"
```

---

## Dependencies / Notes

- **ESPHome** with LVGL support (≥ 2024.x).
- The `mipi_spi` display platform with `model: ESP32-2432S028` is used — it pre-configures CS, DC, rotation, and resolution, so no extra parameters are needed.
- Icons come from **Material Design Icons v7.4.47** (fetched at build time from the official GitHub release).
- Touchscreen calibration values (`x_min`, `x_max`, `y_min`, `y_max`) are a reasonable starting point and may need tuning for your specific unit. Enable `log_level: DEBUG` and watch the raw values in the logs to fine-tune them.
- Backlight is controlled via LEDC PWM (`monochromatic` light entity), so brightness can be adjusted from Home Assistant.
