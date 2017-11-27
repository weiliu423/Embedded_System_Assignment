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
static struct unicast_conn uc;
static uint8_t active;

static int final_num = 0;
      static unsigned int final_dec;
static int final_num_hum = 0;
      static unsigned int final_dec_hum;
	
struct neighbor {
  //need pointer for contiki link list
  struct neighbor *next;
  //addr hold Rime address
  linkaddr_t addr;
  //last_RSSi hold the RSSI value and last_lqi holds the link quality value 
   signed char last_rssi;
   uint16_t last_lqi;
  //uint16_t average_temp;
};

MEMB(neighbors_memb, struct neighbor, MAX_NEIGHBORS);
LIST(neighbors_list);
 static struct unicast_conn uc;
static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from, signed char *rssi)
{
  struct neighbor *n;

  static signed char old_rssi;
  static signed char new_rssi;
  static signed char rss;
  static signed char rss_val;
  static signed char rss_offset;
 
  rss_val = cc2420_last_rssi;  // Get the RSSI from the last received packet
 
  rss = rss_val + rss_offset; // The RSSI correct value in dBm
 
  
  // Check for the neighbor in the list. 
  for(n = list_head(neighbors_list); n != NULL; n = list_item_next(n)) {
    
   //check if the Rime Address match to the list, if it does break out of for loop
    if(linkaddr_cmp(&n->addr, from)) {
	printf("\nNode Already saved in list!\n");
	//n->last_rssi = rss;
    	//n->last_lqi = packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY);
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
   
    
 }else{

 

  printf("items in list is: %d.%d\n", from->u8[0], from->u8[1]);
if(n->next != NULL){
 from  = *(&n->next);
 printf("items in list is: %d.%d\n", from->u8[0], from->u8[1]);
}

}
    //fill in RSSI and LQI into neighbor 
 // n->last_rssi = rss;
 // n->last_lqi = packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY);
    old_rssi = rss;
    new_rssi = rss;
    printf("\nold RSSI %d\n", old_rssi); 
    printf("\nnew RSSI %d\n", new_rssi);
 if(old_rssi < new_rssi){

	printf("new is bigger\n");
	}else{
	printf("old is bigger\n");
	}
 printf("broadcast message received from %d.%d with RSSI %ddBm, LQI %u:'%s'\n",
         from->u8[0], from->u8[1],
         rss,
         packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY), 
         (char *)packetbuf_dataptr());
}
static void
recv_uc(struct unicast_conn *c, const linkaddr_t *from)
{
  printf("unicast received from %d.%d- %s\n",
	 from->u8[0], from->u8[1], packetbuf_dataptr());
}

static const struct unicast_callbacks unicast_callbacks = {recv_uc};
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
        static int val_hum;
      static float s = 0;
	static float s_hum = 0;
      static int dec;
	static int dec_hum;
      static float frac;
	static float frac_hum;
      static int average = 0;
	static int average_hum = 0;
      static int count = 0;
      static unsigned int decs;
        static unsigned int decs_hum;
      char str[4];
	char str_hum[4];
      char temp_hum[60];
      PROCESS_EXITHANDLER(unicast_close(&uc);)
      
      PROCESS_BEGIN();
      
      
      while(1)
      {
	   etimer_set(&et, CLOCK_SECOND * 2);
      	   SENSORS_ACTIVATE(sht11_sensor);
        
	   PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));


           val = sht11_sensor.value(SHT11_SENSOR_TEMP);
           val_hum=sht11_sensor.value(SHT11_SENSOR_HUMIDITY);
      	   if(val != -1 && val_hum != -1) 
      	   {
		s = ((0.01*val) - 39.60);

                s_hum = (((0.0405*val_hum) - 4) 
                 + ((-2.8 * 0.000001)*                      
                 (pow(val_hum,2))));  

      	  	dec = s;
		dec_hum = s_hum;
      	  	frac = s - dec;
		frac_hum = s_hum - dec_hum;
      	  	printf("\nTemperature = %d.%02u C (%d)\n", dec, (unsigned int)(frac * 			100),val);  
		
 		printf("Humidity=%d.%02u %% (%d)\n", dec_hum, 
                (unsigned int)(frac_hum  *100),val_hum);

		count++;  
 		if(count == 5){
			int i;
			for(i = 0; i < count; i++){	  
	 			average = average + dec;
				decs = decs + (unsigned int)(frac*100);

				average_hum = average_hum + dec_hum;
				decs_hum = decs_hum + (unsigned int)(frac_hum*100);
			}	
			final_num = average / 5;
			final_dec = decs / 5;

	   		printf("Average Temperature = %d.%02u\n", final_num, 				final_dec);

			final_num_hum = average_hum / 5;
			final_dec_hum = decs_hum / 5;
			
			printf("Average Humidity = %d.%02u\n", final_num_hum, 				final_dec_hum);

			unicast_open(&uc, 146, &unicast_callbacks);
			linkaddr_t addr;
		
			//sprintf(str, "%d", final_num);
			//sprintf(str_hum, "%d", final_num_hum);
                      
			sprintf(temp_hum, "\nAverage Temperature is: %d and Humidity is: %d", final_num, final_num_hum);

			packetbuf_copyfrom(temp_hum, sizeof(temp_hum));
			
    			addr.u8[0] = 6;
    			addr.u8[1] = 0;
    			if(!linkaddr_cmp(&addr, &linkaddr_node_addr)) {
		      		unicast_send(&uc, &addr);
		    	}
			
			decs = 0;
			average = 0;
			count = 0;
			final_num = 0;
			final_dec = 0;

			decs_hum = 0;
			average_hum = 0;
			final_num_hum = 0;
			final_dec_hum = 0;
		}           
           }
		
	
	   etimer_reset(&et);
    	   SENSORS_DEACTIVATE(sht11_sensor);

      } //end of while
    
      PROCESS_END();

} 

