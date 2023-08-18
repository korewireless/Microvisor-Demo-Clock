# Microvisor C++ Clock Demo

This rudimentary digital clock sample demonstrates Microvisor application development using C++.

It makes use of a Microvisor Nucleo Development Board and a four-digit seven-segment LED display driven by the Holtek HT16K33 controller. These I&sup2;C devices are widely available from electronics suppliers. Interaction with the I&sup2; bus is mediated by the [STM32U585 HAL](#working-with-c-and-the-stm32u585-hal) (Hardware Abstraction Layer).

### Microvisor Configs

The application also makes use of Microvisor's system calls to retrieve a pre-loaded JSON configuration file which is parsed to apply non-default settings to the clock. This shows how you might use cloud-transferred configurations in your own application.

The settings payload is a JSON object:

```json
{ "mode": false,
  "bst": true,
  "brightness": 4,
  "colon": true,
  "flash": false }
```

The *mode* value is `true` for a 24-hour clock, `false` for a 12-hour AM/PM view. PM time is indicated by ligting the display’s rightmost decimal point.

The value *bst* is `true` if you wish the clock to observe [daylight savings time]() when it applies.

Set *brightness* to a value between 1 and 15 — this is the display brightness.

The values of *colon* and *flash* are closely related. The former is `true` if you would like the display’s center colon to be illuminated. If it is, setting *flash* to `true` will cause the colon symbol to turn on and off every second. The board’s user LED will flash in time.

To upload your settings object, use the Microvisor API:

```shell
curl -X POST "https://microvisor.twilio.com/v1/Devices/${MV_DEVICE_SID}/Configs" \
    --data-urlencode "Key=prefs" \
    --data-urlencode "Value={\"mode\": false,\"bst\": true,\"brightness\": 4,\"colon\": true,\"flash\": false}" \
    -u ${TWILIO_ACCOUNT_SID}:{$TWILIO_AUTH_TOKEN}
```

Each Config is a key:value pair which your application code can access. The key is `prefs`. Once uploaed, this Config can be retrieved by the application, which uses the [ArdunioJson library](https://arduinojson.org/) to validate and parse the Config’s JSON content.

### Working with C++ and the STM32U585 HAL

The STM32U585 HAL is written in C, and to safely receive calls from the HAL, your C++ functions should be declared as external C functions. For example, the sample uses the HAL-defined TIM8 IRQ handler callback `TIM8_BRK_IRQHandler()`. To ensure this is correctly address by the C++ linker, add a declaration to your `.cpp` file as follows:

```c++
#ifdef __cplusplus
extern "C" void TIM8_BRK_IRQHandler(void);
#endif
```

## Hardware

Adafruit offers an [inexpensive HT16K33-based display breakout](https://www.adafruit.com/product/878) which you can connect to your Nucleo as follows. CN12 is the right-had GPIO header (with the POWER connector at the top) and CN 11 is on the left (see [Nucleo Getting Started Guide](https://www.twilio.com/docs/iot/microvisor/get-started-with-microvisor#get-to-know-your-board) for details).

| Breakout Pin | Nucleo Pin |
| :--: | :--: |
| SCL | CN12 17 |
| SDA | CN12 5 |
| GND | CN12 71 |
| VCC | CN11 5 |

**Note** GND can be connected to any GND pin.

## License

This application is © 2023 KORE Wireless and is licensed under the [MIT License](.LICENSE.md).

It contains ArduinoJson, which is © 2023 Benoit Blanchon and licensed under the [MIT License](.LICENSE.md).
