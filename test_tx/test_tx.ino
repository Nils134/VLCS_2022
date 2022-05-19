/*
  test_tx.ino: testing the VLC transmitter
  Course: CS4425 Visible Light Communication & Sensing
*/
#include "CRC16.h"

#define F_TRANSMITTER 20 //
#define DT 1000 / F_TRANSMITTER

/*
 * The VLC transmitter is equipped with an RGB LED. 
 * The LED's three channels, R, G, B, can be controlled individually.
 * The R channel is connected to Pin 38 of the Arduino Due
 * The G channel is connected to Pin 42 of the Arduino Due
 * The B channel is connected to Pin 34 of the Arduino Due
 */
const int ledR= 38; // GPIO for controlling R channel
const int ledG= 42; // GPIO for controlling G channel
const int ledB= 34; // GPIO for controlling B channel


/*
 * Brightness of each channel.
 * The range of the brightness is [0, 255].
 * *  0 represents the highest brightness
 * *  255 represents the lowest brightness
 */
int britnessR = 255; // Default: lowest brightness
int britnessG = 255; // Default: lowest brightness
int britnessB = 255; // Default: lowest brightness

const uint16_t MESSAGE_LENGTH = 21;
char message[MESSAGE_LENGTH+1] = "Hello VLC&S 2021-2022";

const uint16_t MOD_MESSAGE_LENGTH = MESSAGE_LENGTH * 2 + 8;

uint8_t message_mod[MOD_MESSAGE_LENGTH];

unsigned long t0;
CRC16 crc;

uint16_t tx_count = 0u;

uint16_t modulate_byte(byte b) {
  uint16_t result = 0u;
  for (uint8_t i = 0; i < 8; i++) {
      uint8_t bit = (b >> i) & 1u;
      uint16_t modbit = (bit == 0u) ? 2u : 1u;
      result = result | (modbit << (2u * i));
      
  }
  return result;
}

/*
 * Some configurations
 */
void setup() {
  Serial.begin(115200); // Set the Baud rate to 115200 bits/s
  while (Serial.available() > 0)
    Serial.read();

  pinMode(ledR, OUTPUT);
  pinMode(ledG, OUTPUT);
  pinMode(ledB, OUTPUT);

  analogWrite(ledR, britnessR); // Turn OFF the R channel
  analogWrite(ledG, britnessG); // Turn OFF the G channel
  analogWrite(ledB, britnessB); // Turn OFF the B channel

  // Preamble (0xAAAA)
  message_mod[0] = 0xAA;
  message_mod[1] = 0xAA;

  // Length
  uint16_t length_mod = modulate_byte(MESSAGE_LENGTH);
  message_mod[2] = length_mod >> 8u;
  message_mod[3] = length_mod;

  // Payload
  for (uint8_t i = 0; i < MESSAGE_LENGTH; i++) {
    uint16_t tmp = modulate_byte(message[i]);
//    crc.add(tmp);
    message_mod[4 + 2*i] = tmp >> 8u;
    message_mod[5 + 2*i] = tmp;
  }
  crc.add(&message_mod[4], MESSAGE_LENGTH * 2u);
  uint16_t crc_val = crc.getCRC();
  Serial.println(crc_val, HEX);
  uint16_t tmp = modulate_byte(crc_val >> 8u);
  message_mod[MOD_MESSAGE_LENGTH - 4] = tmp >> 8u;
  message_mod[MOD_MESSAGE_LENGTH - 3] = tmp;
  tmp = modulate_byte(crc_val);
  message_mod[MOD_MESSAGE_LENGTH - 2] = tmp >> 8u;
  message_mod[MOD_MESSAGE_LENGTH - 1] = tmp;

  Serial.println("Start transmitting");
  Serial.println(MOD_MESSAGE_LENGTH * 8);
  delay(1000);
  t0 = micros();
}
  
/*
 * The Main function
 */
void loop() {
  /*
  * In this simple test, only the R channel is used to transmit data.
  * The R channel is turned ON and OFF alternatively to transmit 1 and 0. 
  * The TX frequency is set to 10 Hz, i.e., sending a symbol every 100 ms
  */
  t0 +=DT;
  
  if (tx_count < (MOD_MESSAGE_LENGTH * 8)) {
    britnessR = (message_mod[tx_count / 8u] & (1u << (7 - (tx_count % 8u)))) == 0 ? 255 : 0;  
    tx_count++;
    analogWrite(ledR, britnessR);
    delay(DT);
  } else {
    Serial.println("Done");
    analogWrite(ledR, 255);
    delay(5*DT);
    tx_count = 0;
  }
}
