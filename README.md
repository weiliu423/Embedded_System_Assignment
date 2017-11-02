# Embedded_System_Assignment
Task:
Each mote sends periodically a broadcast. However, it does not send any broadcast message until the user click the button.
Each mote maintains a list of its neighbours which is populated from the receipt of broadcast messages from its neighbours.
Each mote also maintains the last RSSI and LQI received for each neighbour.
Each mote periodically samples temperature and humidity and computes its average over 5 samples.
Each mote sends its sample to its neighbour with the highest RSSI value.
When a mote receives a packet, the node turns on the green LED.
Last node in the chain prints the message and turns on the red LED. The message indicates the source of the reading along with additional detail.
