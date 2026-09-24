#include <mcp2515.h>


#define USER_LED PC13

#define CAN_ID     (0x01)
#define SPI_CS_PIN (PA0 )


MCP2515 mcp2515(SPI_CS_PIN);

struct can_frame tx, rx;


/**
 * Flash the user LED for a given duration
 *
 * @param duration The time, in milliseconds, to keep  the LED on for
 */
void
flash_led(int duration)
{
  digitalWrite(USER_LED, 1);
  delay(duration);
  digitalWrite(USER_LED, 0);
}

/**
 * Set-up serial, CAN peripheral, tx frame, and enable the user LED
 */
void
setup(void)
{
  /* Set-up serial, timeout at 5s */
  Serial.setTimeout(5000);
  Serial.begin(9600);

  /* Set-up CAN peripheral */
  mcp2515.reset();
  mcp2515.setBitrate(CAN_125KBPS);
  mcp2515.setNormalOneShotMode();

  /* Set tx frame id and clear it */
  tx.can_id = CAN_ID;
  tx.can_dlc = 0;
  for (int i = 0; i < sizeof(tx.data); i++) tx.data[i] = ' ';

  /* Enable user LED */
  pinMode(USER_LED, OUTPUT);
}

/**
 * Read and send up to 8 characters from serial over CAN bus, printing to serial the sent frames,
 * and also print any received frames.
 */
void
loop(void)
{
  /* Read in characters into a frame to transmit */
  while (Serial.available())
  {
    char c = Serial.read();
    flash_led(100);

    /* If a character is not an EOL marker, add it to the frame */
    if (c != '\r' && c != '\n')
    {
      tx.data[tx.can_dlc] = c;
      tx.can_dlc++;
    }

    /* If an EOL character was read or the frame is full */
    if (
        (tx.can_dlc > 0) &&
        (c == '\r' || c == '\n' || tx.can_dlc == sizeof(tx.data))
       )
    {
      /* Inform transmission over serial, e.g. 
       * "> Transmitting |G C1 S| with reported |DLC:6|" */
      Serial.print("> Transmitting |");
      for (int i = 0; i < sizeof(tx.data); i++) Serial.print((char)tx.data[i]);
      Serial.print("| with reported |DLC:");
      Serial.print(tx.can_dlc);
      Serial.println("|");

      /* Send the frame */
      mcp2515.sendMessage(&tx);

      /* Clear frame for next transmission */
      for (int i = 0; i < tx.can_dlc; i++) tx.data[i] = ' ';
      tx.can_dlc = 0;
    }
  }

  /* If a frame is received */
  if (mcp2515.readMessage(&rx) == MCP2515::ERROR_OK)
  {
    flash_led(50);

    /* Output it to serial, e.g.
     * "> Received: |ID:0xDC| |DLC:8| -> |S C1 001|" */
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

  delay(20);
}
