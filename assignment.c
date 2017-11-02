#include "contiki.h"
#include "net/rime/rime.h"
#include "random.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include <stdio.h>
/*---------------------------------------------------------------------------*/
PROCESS(broadcast_process, "Broadcast");
AUTOSTART_PROCESSES(&broadcast_process);
/*---------------------------------------------------------------------------*/
static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
  printf("broadcast message received from %d.%d: '%s'\n",
         from->u8[0], from->u8[1], (char *)packetbuf_dataptr());
}
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;
static uint8_t active;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(broadcast_process, ev, data)
{

  SENSORS_ACTIVATE(button_sensor);
  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

  PROCESS_BEGIN();

  broadcast_open(&broadcast, 129, &broadcast_call);

  while(1) {

    PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event &&
			     data == &button_sensor);
    packetbuf_copyfrom("wei test", 9);
    broadcast_send(&broadcast);
    printf("broadcast message sent\n");
  }

  PROCESS_END();
}

