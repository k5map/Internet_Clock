/*
 NTP module
 */

// NTP Settings:
static const char ntpServerName[] = "us.pool.ntp.org";
//static const char ntpServerName[] = "time.nist.gov";
//static const char ntpServerName[] = "time-a.timefreq.bldrdoc.gov";
//static const char ntpServerName[] = "time-b.timefreq.bldrdoc.gov";
//static const char ntpServerName[] = "time-c.timefreq.bldrdoc.gov";

int OFFSET = -6;        // Central Standard Time (USA)
int OFFSET_DST = -5;    // Central Daylight Time (USA)
//int OFFSET = -5;      // Eastern Standard Time (USA)
//int OFFSET_DST = -4;  // Eastern Daylight Time (USA)
//int OFFSET = -8;      // Pacific Standard Time (USA)
//int OFFSET_DST = -7;  // Pacific Daylight Time (USA)
int timeZone = 0;


/**************************************************/
void setDST()  {
  // Start DST (US) on March 13
  tmElements_t tm;
  tm.Second = 1;  tm.Minute = 0;  tm.Hour = 2;
  tm.Month = 3;
  tm.Day = 13;
  tm.Year = year() - 1970;
  time_t startDST = makeTime(tm);
//  Serial.println(tm.Year);
//  Serial.println(year());
  Serial.print("startDST = ");
  Serial.println(startDST);
  // End DST (US) on November 6
  tm.Month = 11;
  tm.Day = 6;
  time_t endDST = makeTime(tm);
  Serial.print("now = ");
  Serial.println(now());
  Serial.print("endDST = ");
  Serial.println(endDST);
  if ((startDST <= now()) && (now() <= endDST)) {
    timeZone = OFFSET_DST;
    Serial.print("DST enabled; timeZone = ");
    Serial.println(timeZone);
  }
  else {
    timeZone = OFFSET;
    Serial.print("DST disabled; timeZone = ");
    Serial.println(timeZone);
  }
}


/*-------- NTP code ----------------------------------------------*/
const int NTP_PACKET_SIZE = 48;       // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE];   // buffer to hold incoming & outgoing packets

time_t getNtpTime()  {
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ;     // discard any previously received packets
  Serial.println("Transmit NTP Request");
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  Serial.print(ntpServerName);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}


// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)  {
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
