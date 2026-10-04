//Par Gunderson
//10/04/2026
//V-ARC Daisy Seed Firmware
#include "DaisyDuino.h"
Uart hwSerial(14, 15);
using namespace daisysp;
DaisyHardware hw;

// Init Pots
enum PotIndices {
  POT_EQ1 = 0,
  POT_EQ2,
  POT_EQ3,
  POT_VOL,
  POT_HAZE,
  NUM_POTS
};

//Map pins
const int pot_pins[NUM_POTS] = {A0, A1, A2, A3, A4};

//Init Switches
Switch sw_looper_pre_post;
Switch sw_octave_up;
Switch sw_octave_down;
Switch sw_fx[5];
Switch sw_looper_stomp;

//Audio vars
float master_gain = 1.0f;
bool fx1_active = false;

void AudioCallback(float **in, float **out, size_t size) {
  hw.ProcessAnalogControls();
  hw.ProcessDigitalControls();

  //Process Controls
  master_gain = hw.GetKnobValue(POT_VOL);
  fx1_active = sw_fx[0].Pressed();

  for (size_t i = 0; i < size; i++) {
    float in_l = in[0][i];
    float in_r = in[1][i];

    float out_l = in_l * master_gain;
    float out_r = in_r * master_gain;

    out[0][i] = out_l;
    out[1][i] = out_r;
  }
}

void setup() {
  //Init Daisyduino
  hw = DAISY.init(DAISY_SEED, AUDIO_SR_48K);
  
  //Set block size
  DAISY.SetAudioBlockSize(4);

 //Config Pots
  pinMode(pot_pins[POT_EQ1], INPUT);
  pinMode(pot_pins[POT_EQ2], INPUT);
  pinMode(pot_pins[POT_EQ3], INPUT);
  pinMode(pot_pins[POT_VOL], INPUT);
  pinMode(pot_pins[POT_HAZE], INPUT);

  //Config Switches
  sw_looper_pre_post.Init(1000.0f, true, 12, INPUT_PULLUP);
  sw_octave_up.Init(1000.0f, true, 11, INPUT_PULLUP);
  sw_octave_down.Init(1000.0f, true, 10, INPUT_PULLUP);

  int fx_pins[5] = {9, 8, 7, 6, 5};
  for (int i = 0; i < 5; i++) {
    sw_fx[i].Init(1000.0f, true, fx_pins[i], INPUT_PULLUP);
  }
  sw_looper_stomp.Init(1000.0f, true, 4, INPUT_PULLUP);

  //Config UART
  hwSerial.begin(115200);

  //Start Audio
  DAISY.begin(AudioCallback);
}

void loop() {
  //Debounce Switches
  sw_looper_pre_post.Debounce();
  sw_octave_up.Debounce();
  sw_octave_down.Debounce();
  for (int i = 0; i < 5; i++) {
    sw_fx[i].Debounce();
  }
  sw_looper_stomp.Debounce();

  //UART Check
  if (hwSerial.available() >= 1) {
    uint8_t rx_buf[8];
    
    int bytesRead = hwSerial.readBytes(rx_buf, 1); 
    if (bytesRead > 0) {
       // Run received command :) (WIP)
    }
  }

  delay(5);
}
