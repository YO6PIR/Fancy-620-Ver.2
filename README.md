# Fancy-620-Ver.2

<p align="center">
  <img width="800" alt="Fancy-620-Ver.2" src="https://github.com/user-attachments/assets/1f68c2be-1070-45a7-9eaf-f0f71355ca51" />
</p>

A modern software-defined HF/VHF receiver with a beautiful analog instrument display and full VFO/BFO functionality.

Fancy-620-Ver.2 is an advanced version of the Fancy-620 receiver platform, featuring a high-quality analog-style display rendered on a color TFT screen, combined with traditional receiver controls like VFO, BFO, RIT, and frequency calibration.

This project demonstrates how modern embedded systems can deliver both practical amateur radio functionality and an intuitive, visually appealing interface using a compact microcontroller platform.

---

## Overview

The new controller combines:

- powerful STM32F103 processing
- beautiful analog instrumentation drawn on screen
- traditional VFO/BFO receiver architecture
- full frequency control and memory management
- EEPROM-backed persistent settings

The result is a receiver that feels professional and responsive while leveraging the advantages of digital signal processing and user-friendly software control.

---

## Key Features

- **Analog Instrument Display** — S-meter, SWR indicator, and modulation level
- **VFO/DDS Control** — Si5351 synthesizer with precise frequency generation
- **BFO Mode** — beat frequency oscillator for SSB reception
- **RIT Function** — receiver incremental tuning for fine frequency adjustment
- **Graduated Frequency Scale** — main dial and RIT sub-dial with smooth rendering
- **Color TFT Display** — 320x240 ILI9341 for high-quality analog graphics
- **Touchscreen or Encoder Control** — responsive user interaction
- **Persistent Memory** — EEPROM storage of frequency, VFO/BFO settings, and calibration
- **Professional Interface** — easy frequency selection and mode switching

---

## Hardware Architecture

| Component | Specification |
|---|---|
| **Microcontroller** | STM32F103CBT6 (128 KB Flash, 20 KB RAM) |
| **Display** | ILI9341 color TFT, 320 × 240 pixels, SPI interface |
| **Frequency Synthesizer** | Si5351 programmable clock generator |
| **ADC Input** | Received signal level for S-meter, modulation detection |
| **Memory** | I2C EEPROM 24C02 (2 KB) or larger |
| **User Interface** | Rotary encoder and/or touch screen |

The hardware platform is kept minimal and focused on the essential receiver functionality.

---

## Receiver Architecture

### VFO / DDS

The Si5351 synthesizer generates the main local oscillator frequency with high precision and stability.

The VFO is tuned via a large graduated frequency dial rendered on the display, providing intuitive frequency selection.

### BFO

The beat frequency oscillator is used for SSB and CW reception, allowing the user to adjust the audio pitch and carrier frequency for optimal listening.

### RIT (Receiver Incremental Tuning)

The RIT function allows fine frequency adjustment without moving the main VFO, useful for tracking slight frequency drift or tuning to the exact frequency of a distant station.

The RIT offset is displayed on a secondary smaller dial for visual feedback.

### Analog Instrument Display

The display renders three analog instruments:

- **S-Meter** — shows received signal strength
- **SWR Indicator** — indicates antenna impedance match (when applicable)
- **Modulation Meter** — shows audio modulation level or AGC action

These are drawn as traditional analog instruments with needle and scale, providing a familiar and easy-to-read display.

---

## Signal Flow

```text
RF Input
     │
     ▼
Analog RF Front-End
     │
     ▼
Si5351 LO (VFO/BFO)
     │
     ▼
Mixing / Detection
     │
     ▼
ADC Sampling
     │
     ▼
Digital Signal Processing
     │
     ├── S-Meter Computation
     ├── Modulation Detection
     └── SWR Calculation
     │
     ▼
Analog Display Rendering
     │
     ▼
User Interface & Control
```

---

## Software Architecture

The firmware is organized in modular blocks:

- **Main Control** — VFO/BFO management, frequency tuning
- **Analog Display** — rendering of S-meter, SWR, and modulation needles
- **Si5351 Interface** — frequency synthesis and calibration
- **ADC Processing** — signal level measurement and filtering
- **EEPROM Storage** — frequency memory and user settings
- **User Interface** — encoder/touch input handling and navigation
- **Calibration** — frequency calibration and display adjustment

---

## Operating Modes

### Frequency Tuning

The user selects the main VFO frequency using the graduated dial on the display. The dial can be rotated smoothly for quick tuning or fine-tuned for precision.

### BFO Adjustment

In SSB/CW mode, the BFO can be adjusted to set the audio pitch and carrier frequency for comfortable listening.

### RIT Offset

The RIT dial allows the receiver to be tuned slightly away from the VFO frequency without changing the main tuning point, useful for rapid band scanning.

### Memory and Presets

Frequency and mode settings can be stored in EEPROM and recalled quickly.

---

## Display and User Experience

The TFT display renders high-quality analog instruments in real time:

- analog meter needles move smoothly
- frequency dials update instantly
- S-meter responds to signal variations
- SWR and modulation levels are shown continuously

The combination of visual feedback and responsive controls makes the receiver feel professional and intuitive to operate.

---

## Calibration

The firmware includes calibration routines for:

- **Frequency Calibration** — setting the Si5351 correction factor
- **S-Meter Calibration** — matching the analog display to actual RF levels
- **Display Calibration** — ensuring the needles and scales are accurate

These calibration values are stored in EEPROM.

---

## Project Structure

```text
Fancy-620-Ver.2/
├── Fancy620_STM32_Ver6_Analog.ino       # Main firmware
├── analog.ino                            # Analog display rendering
├── si5351.ino                            # Si5351 control
├── scala.ino                             # Frequency scale and dial
├── setari.ino                            # Settings and calibration
├── eprom.ino                             # EEPROM management
├── main.h                                # Main definitions
├── Smeter_bitmap.h                       # S-meter graphic data
├── font24.h                              # Display font
├── Fancy620_STM32_Ver6_Analog_SI60.bin  # Compiled binary (Si5351 variant)
├── Fancy620_STM32_Ver6_Analog_SI62.bin  # Compiled binary (Si5351 variant)
└── README.md
```

---

## Hardware Schematic

<p align="center">
  <img width="800" alt="[PLACEHOLDER: Fancy-620-Ver.2 electronic schematic showing the STM32F103 microcontroller, Si5351 synthesizer, ILI9341 display connections, and RF front-end circuit. Include all signal paths, power distribution, and major component pinouts.]" src="PLACEHOLDER_SCHEMATIC" />
</p>

*Replace `PLACEHOLDER_SCHEMATIC` with the actual URL of the circuit diagram from the project documentation.*

---

## Use Cases

Fancy-620-Ver.2 is suitable for:

- amateur radio reception on HF/VHF bands
- receiver design experimentation
- SDR-style receiver prototyping
- educational amateur radio projects
- compact listening post or monitoring receiver
- signal investigation and frequency scanning

---

## Project Status

Fancy-620-Ver.2 is a functional HF/VHF receiver platform with a modern firmware implementation.

**Current features are stable for:**

- frequency tuning and synthesis
- analog instrument display
- S-meter and signal level indication
- BFO and RIT operation
- memory and calibration storage

---

## Building and Uploading

### Prerequisites

- Arduino IDE with STM32 board support (stm32duino)
- USB-to-serial or ST-Link programmer
- Required libraries:
  - Adafruit_ILI9341
  - si5351 (Etherkit or compatible)
  - EEPROM management for I2C

### Compilation

1. Install STM32 board package in Arduino IDE
2. Select board: **STM32F1xx Series** → **Generic STM32F103CB**
3. Configure upload method
4. Compile and upload the sketch

---

## Technical Notes

### Frequency Stability

Frequency stability depends on Si5351 crystal calibration. The firmware includes calibration routines to compensate for crystal drift.

With proper calibration, frequency error should be <100 ppm across the operating range.

### S-Meter Range

The S-meter display covers typical amateur radio signal levels. Calibration constants can be adjusted for different RF front-end configurations.

### Display Performance

The TFT display updates continuously without flickering, using efficient rendering techniques to keep CPU overhead minimal.

---

## References

- **Project Documentation:** https://www.qsl.net/yo6pir/fancy620.html
- **Si5351 Library:** Etherkit Si5351 Arduino Library
- **Display Library:** Adafruit ILI9341
- **Microcontroller:** STM32F103 Datasheet

---

## Credits

Developed by Ovidiu — YO6PIR

Fancy-620-Ver.2 represents a modern software-defined receiver combining digital signal processing with a traditional, intuitive user interface suitable for amateur radio operation.

---

## License

This project is distributed under the license included in the repository.

See LICENSE.txt for full licensing terms.

---

**Last Updated:** October 2026  
**Firmware Version:** 6  
**Status:** Active Development
