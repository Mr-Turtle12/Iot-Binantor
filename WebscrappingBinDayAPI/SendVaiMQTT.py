# import schedule
import time
import paho.mqtt.client as mqtt
from WebScrapper import BinDay


BROKER_ADDRESS = "test.mosquitto.org"
TOPIC = "uok/iot/wah20/subscribe"  # Replace with your desired MQTT topic
SENT = False


def on_connect(client, userdata, flags, rc):
    print(f"Connected with result code {rc}")
    client.subscribe(TOPIC)


def on_disconnect(client, userdata, rc):
    print(f"Disconnected with result code {rc}")


def send_mqtt_message(message):
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_disconnect = on_disconnect

    try:
        client.connect(BROKER_ADDRESS, 1883, 60)
        client.loop_start()

        # Publish the message
        client.publish(TOPIC, message)
        print(f"Published MQTT message: {message}")
    finally:
        client.loop_stop()
        client.disconnect()


def check_bin_day():
    global SENT, DATA_TOMORROW, location
    BinDays = SCRAPPER.run(location)  # Replace 'location' with the actual location
    for key in BinDays:
        BinDays[key] = SCRAPPER.IsTomorrow(BinDays[key], DATA_TOMORROW)

    print(BinDays["general"])

    if BinDays["general"]:
        send_mqtt_message("Put out the bin!")
        SENT = True
    elif SENT:
        send_mqtt_message("No need to put out the bin.")
        SENT = False


# Schedule the job to run every day at 7pm and then 10am
# schedule.every().day.at(":00").do(check_bin_day)
# schedule.every().day.at("08:00").do(check_bin_day)


if __name__ == "__main__":
    # Run the scheduled jobs
    DATA_TOMORROW = "Thursday 16 November"  # datetime.date.tommor()
    SCRAPPER = BinDay()
    location = "Canterbury ct27lh 64"
    while True:
        check_bin_day()
        time.sleep(1)
