import json
import random
import time
import paho.mqtt.client as mqtt

BROKER = "localhost"
PORT = 1883
BASE_TOPIC = "supervision/device1"
STATUS_TOPIC = BASE_TOPIC + "/status"
MEASURE_TOPIC = BASE_TOPIC + "/measurements"
CMD_TOPIC = BASE_TOPIC + "/cmd"

load = 10.0
temperature = 55.0
overvoltage_ticks = 0

def on_connect(client, userdata, flags, reason_code, properties):
 	client.subscribe(CMD_TOPIC)


def on_message(client, userdata, msg):
	global overvoltage_ticks, temperature
	commande = msg.payload.decode()
	if commande == "overvoltage":
		overvoltage_ticks= 30
	if commande == "overheat":
                temperature += 40



client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
client.on_connect = on_connect
client.on_message = on_message
client.will_set(STATUS_TOPIC, "offline", qos=1, retain=True)
client.connect(BROKER, PORT)
client.loop_start()
client.publish(STATUS_TOPIC,"online",qos=1,retain=True)

try:

 while True:
    voltage = 230.0 + random.uniform(-2.0, 2.0)

    if overvoltage_ticks > 0:
        voltage += 40
        overvoltage_ticks -=1

    load = min(12.0, max(8.0, load + random.uniform(-0.2, 0.2)))
    current = load + random.uniform(-0.1, 0.1)
    power = voltage * current / 1000.0
    target = 40.0 + 1.5 * current
    temperature += (target - temperature) * 0.01

    
    message ={
		"ts" : int(time.time() * 1000),
		"voltage" : round(voltage,2),
		"current" : round(current,2),
		"power" : round(power,2),
		"temperature" : round(temperature,2),
	     }
    payload = json.dumps(message)

    client.publish(BASE_TOPIC + "/measurements", payload)
    print(payload)
    time.sleep(0.1)

except KeyboardInterrupt:
  info = client.publish(STATUS_TOPIC,"offline",qos=1,retain=True)
  info.wait_for_publish()

client.loop_stop()
client.disconnect()
