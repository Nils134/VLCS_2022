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

int tx_interval = 1;

int patterns[4][4] = {{1,0,0,1}, {0,1,1,0}, {1,0,1,1}, {1,1,0,0}};
int pattern_count = 0;
int bit_count = 0;
int t0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  t0 = millis();
}

void loop() {
  if ((millis() - t0) > 1000) {
    pattern_count++;
    Serial.println((String) "Pattern " + pattern_count);
    t0 = millis();
  }
  if (patterns[pattern_count%4][bit_count%4] == 0) {
    analogWrite(ledR, 255);
  } else {
    analogWrite(ledR, 0);
  }
  bit_count++;

  delay(tx_interval);
}
