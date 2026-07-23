<p align="center">
  <img src="PCB.png" alt="Assembled PCB photo" width="600"/>
</p>

## 🛠️ Specifications & Board Limits

- **Maximum Voltage:** 12V DC.
- **Maximum Motor Current:** Supports up to **3A stall current**.
- **Protections:** Socket for a **5x20mm cylindrical fuse** (Slow-Blow) and a **1N5822** protection diode.
- **Motor Switching:** **IRLZ44N** MOSFET.

---

## 🧩 Bill of Materials (BOM)

### Main Board & Circuit
| Qty | Component | Description |
|---|---|---|
| 1 | Arduino Nano V3 | Main microcontroller |
| 1 | IRLZ44N MOSFET | Motor PWM control |
| 1 | 1N5822 Diode | Flyback / protection diode |
| 2 | 5mm Terminal Block (2-pin) | 12V power input & motor output |
| 1 | Fuse Socket | 5x20mm fuse holder |
| 2 | 5mm LED | Status / signaling LEDs |
| 1 | 4.7kΩ Resistor | Temperature sensor pull-up |
| 1 | 10kΩ Resistor | MOSFET pull-down |
| 2 | 220Ω Resistors | Current-limiting resistors for LEDs |
| 2 | 1x03 Female Headers | Hall Sensor & DS18B20 connectors |
| 1 | 1x06 Female Header | INA219 Module connector |

### Supported External Sensors
To enable all board features, connect the following external modules:
- **Temperature:** **DS18B20** Sensor (1-Wire, utilizes the onboard 4.7kΩ pull-up resistor).
- **Current & Voltage:** **INA219** Module (I2C Communication).
- **Rotation / RPM:** **3-Pin Hall Sensor** (Speed measurement).
