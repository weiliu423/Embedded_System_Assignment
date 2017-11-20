#include "contiki.h"
#include "net/rime/rime.h"
#include "random.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include "lib/list.h"
#include "lib/memb.h"
#include <stdio.h>
#include <math.h>
#include "dev/light-sensor.h"
#include "dev/sht11/sht11-sensor.h"
#include "/home/user/contiki-3.0/dev/cc2420/cc2420.h"
#define MAX_NEIGHBORS 4
/*---------------------------------------------------------------------------*/
PROCESS(broadcast_process, "Broadcast");
PROCESS(sensor_acq_process,"Sensor Acquisition");
AUTOSTART_PROCESSES(&broadcast_process, &sensor_acq_process);
/*---------------------------------------------------------------------------*/

static struct broadcast_conn broadcast;
static uint8_t active;
   static int final_num = 0;
      static unsigned int final_dec;
struct neighbor {
  //need pointer for contiki link list
  struct neighbor *next;
  //addr hold Rime address
  linkaddr_t addr;
  //last_RSSi hold the RSSI value and last_lqi holds the link quality value 
  uint16_t last_rssi, last_lqi;
  //uint16_t average_temp;
};

MEMB(neighbors_memb, struct neighbor, MAX_NEIGHBORS);
LIST(neighbors_list);

static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
  struct neighbor *n;

  uint16_t old_rssi = 0;
  uint16_t new_rssi = 0;

  // Check for the neighbor in the list. 
  for(n = list_head(neighbors_list); n != NULL; n = list_item_next(n)) {
    
   //check if the Rime Address match to the list, if it does break out of for loop
    if(linkaddr_cmp(&n->addr, from)) {
	printf("\nNode Already saved in list!\n");
	n->last_rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);
    	n->last_lqi = packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY);
        break;
    }
   /*else if(n == NULL) {
        // if n is empty, then allocate new memory block using struct neighbor
	printf("\nRegister Node into list\n");
        n = memb_alloc(&neighbors_memb);

        // Initialize the fields. 
        linkaddr_copy(&n->addr, from);

        // add neighbor into list.
        list_add(neighbors_list, n);

	
    }*/
    
	
  }
 old_rssi = cc2420_last_rssi;
 new_rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);
	printf("\nold RSSI %u\n", old_rssi);
	printf("\nnew RSSI %u\n", new_rssi);
 if(old_rssi < new_rssi){

	printf("new is bigger\n");
	}else{
	printf("old is bigger\n");
	}

  // if n is empty, then allocate new memory block using struct neighbor
  if(n == NULL) {
    printf("\nRegister Node into list\n");
    n = memb_alloc(&neighbors_memb);

    // Initialize the fields. 
    linkaddr_copy(&n->addr, from);

    // add neighbor into list.
    list_add(neighbors_list, n);
    n->last_rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);
    n->last_lqi = packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY);
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
/*------------------------------------------------------------------------*/
PROCESS_THREAD(sensor_acq_process,ev,data)
{
      static struct etimer et;
      static int val;
      static float s = 0;
      static int dec;
      static float frac;
      static int average = 0;
      static int temp_count = 0;
      static int hum_count = 0;
      static unsigned int decs;
    //  static int final_num = 0;
    //  static unsigned int final_dec;
      struct neighbor *n;

      PROCESS_BEGIN();

      printf("Starting Sensor Example.\n");
      
      while(1)
      {
	   etimer_set(&et, CLOCK_SECOND * 5);
      	   SENSORS_ACTIVATE(sht11_sensor);
        
	   PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));


           val = sht11_sensor.value(SHT11_SENSOR_TEMP);
      	   if(val != -1) 
      	   {
		s= ((0.01*val) - 39.60);
      	  	dec = s;
      	  	frac = s - dec;
      	  	printf("\nTemperature = %d.%02u C (%d)\n", dec, (unsigned int)(frac * 			100),val);  
		temp_count++;  
 		if(temp_count == 5){
			int i;
			for(i = 0; i < temp_count; i++){	  
	 			average = average + dec;
				decs = decs + (unsigned int)(frac*100);
			}	
			final_num = average / 5;
			final_dec = decs / 5;
	   		printf("Average Temperature = %d.%02u\n", final_num, 				final_dec);
			decs = 0;
			average = 0;
			temp_count = 0;
			final_num = 0;
			final_dec = 0;
		}           
           }
		
	   val=sht11_sensor.value(SHT11_SENSOR_HUMIDITY);
	   if(val != -1) 
      	   {
		s= (((0.0405*val) - 4) + ((-2.8 * 0.000001)*(pow(val,2))));  
      	  	dec = s;
      	  	frac = s - dec;
      	  	printf("Humidity=%d.%02u %% (%d)\n", dec, (unsigned int)(frac * 		100),val);  
		hum_count++;  
 		if(hum_count == 5){
			int i;
			for(i = 0; i < hum_count; i++){	  
	 			average = average + dec;
				decs = decs + (unsigned int)(frac*100);
			}	
			final_num = average / 5;
			final_dec = decs / 5;
	   		printf("Average Humidity = %d.%02u\n", final_num, 				final_dec);
			decs = 0;
			average = 0;
			hum_count = 0;
			final_num = 0;
			final_dec = 0;
		}                  
           }

         
	
	   etimer_reset(&et);
    	  // SENSORS_DEACTIVATE(light_sensor);
    	   SENSORS_DEACTIVATE(sht11_sensor);

      } //end of while
    
      PROCESS_END();

} 

