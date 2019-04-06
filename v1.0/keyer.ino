// inspired from https://sv1obt.wordpress.com/2011/12/01/iambic-keyer-with-arduino-in-5-minutes/

// pin definitions
#define DIT_PIN 3
#define DAH_PIN 2

#define OUT_PIN 6

#define HZ_PIN 0 // analog
#define DUR_PIN 1 // analog

#define LED_PIN 13 // onboard led

#define LED_20_PIN 10 // 20 wpm
#define LED_25_PIN 11 // 25 wpm

#define DIT_20_MS 60 // 60ms = 20 wpm
#define DIT_25_MS 48 // 48ms = 25 wpm

// timings
int dit_time = DIT_20_MS;
int dat_time = dit_time*3;
int interbaud_time = dit_time;
int interletter_time = dit_time*2;

int buzz = 500; // in hz

// timeseries of buzz values for poor man's smoothing
int buzz_0 = 500; // most recent
int buzz_1 = 500;
int buzz_2 = 500;

// state enum
enum { IDLE, DIT, DAH, PAUSE, };
int state;
int dit, dah;

void setup() {
  // put your setup code here, to run once:
  
  pinMode(DIT_PIN, INPUT);
  pinMode(DAH_PIN, INPUT);
  pinMode(OUT_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_20_PIN, OUTPUT);
  pinMode(LED_25_PIN, OUTPUT);
  state = 0; // IDLE
}

void adjustTime() {
  dat_time = dit_time*3;
  interbaud_time = dit_time;
  interletter_time = dit_time*2;
}

void readDit() {
  int dit_pin = digitalRead(DIT_PIN);
  if (dit_pin) { dit = 1; }
  else { dit = 0; }
}

void readDah() {
  int dah_pin = digitalRead(DAH_PIN);
  if (dah_pin) { dah = 1; }
  else { dah = 0; }
}

void readHz() {
  int hz_pin = analogRead(HZ_PIN);
  int rem = hz_pin % 10;

  // advance time series
  buzz_2 = buzz_1;
  buzz_1 = buzz_0;

  // calculate new buzz
  buzz_0 = 900 - (hz_pin - rem);

  // only change if persists
  if (buzz_0 == buzz_2 && buzz_0 == buzz_1) { buzz = buzz_0; }
}

void readDur() {
  int dur_pin = analogRead(DUR_PIN);

  if (dur_pin < 340) { 
    dit_time = DIT_20_MS; 
    adjustTime();
    digitalWrite(LED_20_PIN, HIGH);
    digitalWrite(LED_25_PIN, LOW);
  }
  else {
    dit_time = DIT_25_MS; 
    adjustTime();
    digitalWrite(LED_25_PIN, HIGH);
    digitalWrite(LED_20_PIN, LOW);
  }
}

void contact(unsigned char val) {
  if (val) {
    digitalWrite(LED_PIN, HIGH);
    tone(OUT_PIN, buzz);
  }
  else {
    digitalWrite(LED_PIN, LOW);
    noTone(OUT_PIN);
  }
}

void loop() {
  // put your main code here, to run repeatedly:

  // read analog dials
  readDur();
  readHz();

  switch(state) {
    case IDLE:
      readDit();
      if (dit) { state = DIT; }
      else {
        delayMicroseconds(30);
        readDah();
        if (dah) { state = DAH; }
      }
    break;

    case DIT:
      contact(1);
      delay(dit_time);
      contact(0);
      delay(interbaud_time);
      readDah();
      if (dah) { state = DAH; }
      else {
        readDit();
        if (dit) { state = DIT; }
        else { 
          delay(interletter_time); 
          state = IDLE;
        }
      }
    break;

    case DAH:
      contact(1);
      delay(dat_time);
      contact(0);
      delay(interbaud_time);
      readDit();
      if (dit) { state = DIT; }
      else {
        readDah();
        if (dah) { state = DAH; }
        else {
          delay(interletter_time);
          state = IDLE;
        }
      }
    break;

    case PAUSE:
    break;
  }
  
  delay(1);
}
