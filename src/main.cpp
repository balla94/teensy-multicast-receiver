#include <Arduino.h>
#include <Audio.h>
#include <QNEthernet.h>
#include <Wire.h>

constexpr uint32_t kDHCPTimeout = 15'000; // 15 seconds
using namespace qindesign::network;
EthernetUDP udp;

// GUItool: begin automatically generated code
AudioPlayQueue queue1;        // xy=338,298
AudioPlayQueue queue2;        // xy=338,336
AudioAnalyzePeak peak1;       // xy=514,186
AudioMixer4 mixer1;           // xy=538,556
AudioAmplifier amp2;          // xy=579,337
AudioAmplifier amp1;          // xy=580,300
AudioOutputI2SQuad i2s_quad1; // xy=861,347
AudioConnection patchCord1(queue1, amp1);
AudioConnection patchCord2(queue1, peak1);
AudioConnection patchCord3(queue2, amp2);
AudioConnection patchCord4(mixer1, 0, i2s_quad1, 2);
AudioConnection patchCord5(amp2, 0, i2s_quad1, 1);
AudioConnection patchCord6(amp2, 0, mixer1, 1);
AudioConnection patchCord7(amp1, 0, i2s_quad1, 0);
AudioConnection patchCord8(amp1, 0, mixer1, 0);
AudioControlSGTL5000 sgtl5000_1; // xy=370,527
// GUItool: end automatically generated code


void ethernet_link_cb(bool state)
{
  printf("[Ethernet] Link %s\r\n", state ? "ON" : "OFF");
}

void ethernet_address_changed_cb()
{
  Serial.print("Got IP Address:");
  Serial.print(Ethernet.localIP());
  Serial.print("\r\n");
}

void setup_ethernet()
{
  uint8_t mac[6];
  Ethernet.macAddress(mac); // This is informative; it retrieves, not sets

  printf("MAC = %02x:%02x:%02x:%02x:%02x:%02x\r\n",
         mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  Ethernet.onLinkState(ethernet_link_cb);

  Ethernet.onAddressChanged(ethernet_address_changed_cb);
  
  if (!Ethernet.begin())
  {
    printf("Failed to start Ethernet\r\n");
  }
}

int i2c_scan(TwoWire i2c)
{
  byte count = 0;

  i2c.begin();
  for (byte i = 1; i < 120; i++)
  {
    i2c.beginTransmission(i);
    if (i2c.endTransmission() == 0)
    {
      Serial.print(F("Found address: "));
      Serial.print(i, DEC);
      Serial.print(F(" (0x"));
      Serial.print(i, HEX);
      Serial.println(F(")"));
      count++;
    } // end of good response
    delay(5); // give devices time to recover
  } // end of for loop

  Serial.println(F("I2C scan done.."));
  Serial.print(F("Found "));
  Serial.print(count, DEC);
  Serial.println(F(" device(s)."));
  return count;
}

void setup()
{
  Serial.begin(1000000);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  delay(1000);
  Serial.println("Teensy Audio Player");

  Serial.println("Scan on i2c0");
  i2c_scan(Wire);
  Serial.println("Scan on i2c1");
  i2c_scan(Wire1);
  Serial.println("Scan on i2c2");
  i2c_scan(Wire2);

  delay(3500);

  setup_ethernet();

  Serial.println("Audio board begin");
  AudioMemory(500);

  queue1.setBehaviour(AudioPlayQueue::ORIGINAL);
  queue1.setMaxBuffers(200);
  queue2.setBehaviour(AudioPlayQueue::ORIGINAL);
  queue2.setMaxBuffers(200);
  mixer1.gain(0, 0.25);
  mixer1.gain(1, 0.25);
  mixer1.gain(2, 1.0);
  mixer1.gain(3, 1.0);
  sgtl5000_1.enable();
  sgtl5000_1.volume(100);
  udp.beginMulticastWithReuse(IPAddress(239, 0, 1, 118), 8077);
}

void handleUdp()
{
  int packetSize = udp.parsePacket();
  if (packetSize > 0)
  {

    // Read the packet into a buffer
    char packetBuffer[1500];
    int len = udp.read(packetBuffer, 1500);
    if (len > 0)
    {
      // Calculate the number of samples
      int numSamples = len / 4; // Each sample is 4 bytes (2 bytes left + 2 bytes right)

      // Allocate arrays to hold the left and right channel data
      int16_t leftChannelData[numSamples];
      int16_t rightChannelData[numSamples];

      // Extract the left and right channel data
      for (int i = 0; i < numSamples; i++)
      {
        // Extract the left channel 16-bit value
        leftChannelData[i] = (packetBuffer[i * 4 + 1] << 8) | (packetBuffer[i * 4]);

        // Extract the right channel 16-bit value
        rightChannelData[i] = (packetBuffer[i * 4 + 3] << 8) | (packetBuffer[i * 4 + 2]);
      }

      // Pass the left channel data array to queue1.play
      queue1.play(leftChannelData, numSamples);

      // Pass the right channel data array to queue2.play
      queue2.play(rightChannelData, numSamples);
    }
  }
}

int convertFloatToByte(float value)
{
  if (value < 0.0)
    value = 0.0;
  if (value > 1.0)
    value = 1.0;
  return (int)(value * 255.0);
}

void loop()
{
  handleUdp();

  while (Ethernet.linkStatus() != LinkON)
  {
    analogWrite(LED_BUILTIN, 50);
    delay(500);
    analogWrite(LED_BUILTIN, 200);
    delay(500);
  }
  if (peak1.available())
  {
    analogWrite(LED_BUILTIN, convertFloatToByte(peak1.read()));
  }
}
