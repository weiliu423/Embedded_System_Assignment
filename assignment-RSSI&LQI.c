#include "contiki.h"
#include "net/rime/rime.h"
#include "random.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include "lib/list.h"
#include "lib/memb.h"
#include <stdio.h>
#define MAX_NEIGHBORS 4
/*---------------------------------------------------------------------------*/
PROCESS(broadcast_process, "Broadcast");
AUTOSTART_PROCESSES(&broadcast_process);
/*---------------------------------------------------------------------------*/

static struct broadcast_conn broadcast;
static uint8_t active;

struct neighbor {
  //need pointer for contiki link list
  struct neighbor *next;
  //addr hold Rime address
  linkaddr_t addr;
  //last_RSSi hold the RSSI value and last_lqi holds the link quality value 
  uint16_t last_rssi, last_lqi;
};

MEMB(neighbors_memb, struct neighbor, MAX_NEIGHBORS);
LIST(neighbors_list);

static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
  struct neighbor *n;

  // Check for the neighbor in the list. 
  for(n = list_head(neighbors_list); n != NULL; n = list_item_next(n)) {
    
   //check if the Rime Address match to the list, if it does break out of for loop
    if(linkaddr_cmp(&n->addr, from)) {
	printf("\nNode Already saved in list!\n");
        break;
    }
  }

  // if n is empty, then allocate new memory block using struct neighbor
  if(n == NULL) {
    printf("\nRegister Node into list\n");
    n = memb_alloc(&neighbors_memb);

    // Initialize the fields. 
    linkaddr_copy(&n->addr, from);

    // add neighbor into list.
    list_add(neighbors_list, n);
  }

    //fill in RSSI and LQI into neighbor 
  n->last_rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);
  n->last_lqi = packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY);

 printf("broadcast message received from %d.%d with RSSI %u, LQI %u:'%s'\n",
         from->u8[0], from->u8[1],
         packetbuf_attr(PACKETBUF_ATTR_RSSI),
         packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY), 
         (char *)packetbuf_dataptr());
}
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
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

