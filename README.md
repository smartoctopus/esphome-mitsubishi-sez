# esphome-mitsubishi-sez
Custom component to support Mitsubishi SEZ-KD as climate IR in ESPhome

## Using this component
Add to your esphome configuration:
```
external_components:
  source: github://smartoctopus/esphome-mitsubishi-sez
```
Then, add the climate config:

```
remote_transmitter:
  pin: GPIO32
  carrier_duty_percent: 50%
  
climate:
  - platform: mitsubishi_sez
    name: "AC"
```

## Credits
This component is based on [esphome-climate-mhi](https://github.com/Dennis-Q/esphome-climate-mhi). The actual protocol comes from [arduino-heatpumpir](https://github.com/ToniA/arduino-heatpumpir).
