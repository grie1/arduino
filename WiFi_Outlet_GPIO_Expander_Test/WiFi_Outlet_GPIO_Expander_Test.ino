// Test i2c expander PI4IOE5V6408
// Turns relays on and off a few times
// https://www.diodes.com/assets/Datasheets/PI4IOE5V6408.pdf
// Linux Driver: https://coral.googlesource.com/linux-imx/+/refs/tags/10-1/drivers/gpio/gpio-pi4ioe5v6408.c

#include <Wire.h>

const uint32_t SERIAL_SPEED         = 115200; // Set the baud rate for Serial I/O
const uint8_t  SCL_PIN              = 33;
const uint8_t  SDA_PIN              = 32;
const uint8_t  SPRINTF_BUFFER_SIZE  = 32;   
const uint8_t  NUM_LOOPS            = 2;

char inputBuffer[SPRINTF_BUFFER_SIZE];   

int gpioExpanderAddress = 0x43;
int toggleDelay = 5000;

// Only 3 ports used on this board
// Port addresses: "0x01" "0x02" "0x04" "0x08" "0x10" "0x20" "0x40" "0x80"
// Inverse of port addresses: "0xfe" "0xfd" "0xfb" "0xf7" "0xef" "0xdf" "0xbf" "0x7f"
#define PORT0 0x01 // Relay 1
#define PORT1 0x02 // Relay 2
#define PORT2 0x08 // Relay 3

// Port states
#define HI_IMPEDANCE 0x07 // 0x07 Output High-Impedance, default on (1).  Disable to set output pin hi/low.
#define IO_DIRECTION 0x03 // 0x03 I/O Direction: 0 input, 1 output
#define IO_STATE 0x05 // 0x05 Output State: high/low: 0 low, 1 high



void setup() {
  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.begin(SERIAL_SPEED);
  
  reset_expander();
  delay(1000);
}

void loop() {
  toggle_relays();
}

void set_register(int address, unsigned char r, unsigned char v){
  Wire.beginTransmission(address);
  Wire.write(r);
  Wire.write(v);
  Wire.endTransmission();
}

void reset_expander(){
  Wire.beginTransmission(gpioExpanderAddress);
  Wire.write(0x01); // queue byte
  Wire.write(0x01); // queue byte
  Wire.endTransmission(); // send bytes
  Serial.println("Expander Reset Register 0x01 set to 0x01");
}

void toggle_relays(){
  Wire.beginTransmission(gpioExpanderAddress);
  Wire.write(0x07); // queue byte
  Wire.write(0x00); // queue byte
  Wire.endTransmission(); // send bytes
  Serial.println("Relays On");
  delay(toggleDelay);
  Wire.beginTransmission(gpioExpanderAddress);
  Wire.write(0x07); // queue byte
  Wire.write(0xFF); // queue byte
  Wire.endTransmission(); // send bytes
  Serial.println("Relays Off");
  delay(toggleDelay);
}
