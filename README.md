# ch32v003freqcounter

This is an electronic construction kit teaching material for kids' science experiment classes.
## Sound learning

It takes in sound, counts the frequencies, and displays them.

Learn what frequency various sounds are,
You can learn how the Doppler effect is generated by motion to a sound source.


In the future, the system will be able to display waveforms, measure ultrasonic frequencies, and output sounds of arbitrary frequencies.
This will allow students to learn that sound is a wave, that sounds are not audible to humans, and that different sounds can be heard at different ages.


## Concept
We try to make the kit as inexpensive as possible. We try to make the kit as inexpensive as possible and use inexpensive microcontrollers for this purpose.

## Soldering Training
The main components are soldered at the factory during manufacturing. The remaining few through-hole parts are made to be completed by soldering.

## Applications

With its small footprint and fast screen display, it can be used as a small signage.

# Curcit

## Schematic

![image](https://github.com/user-attachments/assets/f9bb966e-5fb7-449d-bd39-e4527e4bff9e)

## artdesign
![image](https://github.com/user-attachments/assets/651a34eb-79ba-451a-b981-b10114599e5b)



# pin assign

| CH32V003F4P6 PIN |           | OLED PIN | OLED |
|------------------|-----------|----------|------|
|                  |           | 1        | GND  |
|                  |           | 2        | 3.3V |
| 15               |           | 3        | D0   |
| 14               | PC4       | 4        | D1   |
| 17               | PC7(MISO) | 5        | RST  |
| 16               | MOSI      | 6        | DC   |
| 13               | PC3       | 7        | CS   |

| CH32V003F4P6 PIN |       | PROGRAM PIN |   | LED(D1) |         |
|------------------|-------|-------------|-------|---------|---------|
|                  |       | 1           | 3.3V  |         |         |
| 18               | SWDIO | 2           | SWDIO |         |         |
|                  |       | 3           | GND   |         | CATHODE |
| 4                | NRST  | 4           | RST   |         |         |
| 2                | UTX   | 5           | TX    |         | ANODE   |
| 3                | URX   | 6           | RX    |         |         |



| CH32V003F4P6 PIN |           | SW  |  |
|------------------|-----------|-----|------|
| 8                | PD0       | SW1 | OK   |
| 10               | PC0       | SW2 | ↑    |
| 1                | PD4       | SW3 | ↓    |


| CH32V003F4P6 PIN |         | SEEED_GROVE |    |
|------------------|---------|-------------|--------|
| 12               | PC2/SCL | 1           | YELLOW |
| 11               | PC1/SDA | 2           | WHITE  |
|                  |         | 3           | 3.3V   |
|                  |         | 4           | GND    |



| CH32V003F4P6 PIN |        | NAX4466_MIC_MODULE |  |
|------------------|--------|--------------------|------|
| 20               | PD3/A4 | 1                  | OUT  |
|                  |        | 2                  | GND  |
|                  |        | 3                  | 3.3V |

| CH32V003F4P6 PIN |      | SPEAKER |  |
|------------------|------|---------|------|
|                  |      | 1       | GND  |
| 19               | PD2  | 2       | IN   |
