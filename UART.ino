#include <mcp2515.h>


#define USER_LED PC13

#define GROUND_CONTROL_CAN_ID (0x01)
#define SPI_CS_PIN            (PA0)


MCP2515 mcp2515(SPI_CS_PIN);

struct can_frame tx, rx;


void
flash_led(int duration)
{
  digitalWrite(USER_LED, 1);
  delay(duration);
  digitalWrite(USER_LED, 0);
}

void setup() {
  Serial.setTimeout(5000);
  Serial.begin(9600);

  /* Set tx frame id and clear it */
  tx.can_id = GROUND_CONTROL_CAN_ID;
  tx.can_dlc = 0;
  for (int i = 0; i < sizeof(tx.data); i++) tx.data[i] = ' ';

  pinMode(USER_LED, OUTPUT);

  mcp2515.reset();
  mcp2515.setBitrate(CAN_125KBPS);
  mcp2515.setNormalOneShotMode();
}

void loop() {
  char c;

  if (Serial.available())
  {
    c = Serial.read();
    flash_led(100);

    if (c != '\r' && c != '\n')
    {
      tx.data[tx.can_dlc] = c;
      tx.can_dlc++;
    }

    if (
        (tx.can_dlc > 0) &&
        (c == '\r' || c == '\n' || tx.can_dlc == sizeof(tx.data))
       )
    {
      /* e.g. > Transmitting |G C1 S| with reported |DLC:6| */
      Serial.print("> Transmitting |");
      for (int i = 0; i < sizeof(tx.data); i++) Serial.print((char)tx.data[i]);
      Serial.print("| with reported |DLC:");
      Serial.print(tx.can_dlc);
      Serial.println("|");

      mcp2515.sendMessage(&tx);

      for (int i = 0; i < sizeof(tx.data); i++) tx.data[i] = ' ';
      tx.can_dlc = 0;
      while (Serial.available()) Serial.read();
    }
  }

  if (mcp2515.readMessage(&rx) == MCP2515::ERROR_OK)
  {
    flash_led(50);
    flash_led(50);

    /* e.g. > Received: |ID:0xDC| |DLC:8| -> |S C1 001| */
    Serial.print("> Received: ");
    Serial.print("|ID:");
    Serial.print(rx.can_id, HEX);
    Serial.print("| |DLC:"); 
    Serial.print(rx.can_dlc);
    Serial.print("| -> |");
    for (size_t i = 0; i < rx.can_dlc; i++)
    {
      Serial.print((char)rx.data[i]);
    }
    Serial.println("|");
  }

  delay(100);
}
