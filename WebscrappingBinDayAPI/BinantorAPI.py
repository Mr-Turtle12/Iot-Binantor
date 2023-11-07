from flask import Flask, jsonify
from WebScrapper import BinDay
import requests

# Define the base URL
base_url = "http://192.168.0.7:5000/"

# Define the endpoint
endpoint = "BinAPI"  # Change this to the desired endpoint

# Creating the complete URL
url = f"{base_url}{endpoint}"

# Making a GET request to the URL
response = requests.get(url)

# Printing the response
print(response.text)


app = Flask(__name__)

# Get tomorrow's date
# tomorrow = datetime.date.today() + datetime.timedelta(days=1)
# For Testing can change what day it is:


@app.route("/GeneralCheckValue/<string:location>/", methods=["GET"])
def check_value(location):
    # Define your condition here
    BinDays = Scrapper.run(location)
    print(BinDays)
    print(DATA_TOMORROW)
    # Finds out if tomorrow is bin day if it is then will need to put it out, replace in the dict the data with true or false
    for key in BinDays:
        BinDays[key] = Scrapper.IsTomorrow(BinDays[key], DATA_TOMORROW)
    return jsonify(BinDays["general"])


if __name__ == "__main__":
    # datetime.date.today() + datetime.timedelta(days=1)
    DATA_TOMORROW = "Thursday 27 October"  # datetime.date.tommor()
    Scrapper = BinDay()

    app.run(host="0.0.0.0")
