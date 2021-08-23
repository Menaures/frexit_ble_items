/*

   2019 Copyright (c) Seeed Technology Inc.  All right reserved.
   Author:Baozhu zuo (zuobaozhu@gmail.com)

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
   WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
   MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
   ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
   WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
   ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
   OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include "MG126_Ble.h"


MG126_Ble_Class::MG126_Ble_Class(int cs_pin, int irq_pin) {
  cs = cs_pin;
  irq = irq_pin;
}

MG126_Ble_Class::MG126_Ble_Class() {
  cs = 47;
  irq = 7;
}

MG126_Ble_Class::~MG126_Ble_Class() {

}

void MG126_Ble_Class::io_init() {
  BSP_Init(cs, irq);
}

void MG126_Ble_Class::ble_init() {
  SPI_init();
  io_init();
  SetBleIntRunningMode();

  radio_initBle(TXPWR_0DBM, &ble_mac_addr);
  delay(5);
  Serial.print("BLE Mac address: ");
  for (int i = 0; i < 6; i++)
  {
    SERIAL.print(ble_mac_addr[i]);
    SERIAL.print('.');
  }
  SERIAL.println();

  ble_run_interrupt_start(160 * 2); //320*0.625=200 ms
  Serial.println("BLE setup complete!");
}

unsigned char * MG126_Ble_Class::get_ble_mac_addr() {
  return ble_mac_addr;
}


void MG126_Ble_Class::test() {
  SPI_WriteRead(123, 0);
}
