# esphome-mitsubishi-sez
Custom component to support Mitsubishi SEZ-KD as climate IR in ESPhome

## Using this component
Clone/download this repository into '/config/esphome/esphome-climate-mhi' and add to your esphome configuration:
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
