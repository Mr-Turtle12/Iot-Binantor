import argparse
import datetime
import sys
import requests
from bs4 import BeautifulSoup


class BinDay:
    def scrape_dates_from_URl(self, url):
        try:
            response = requests.get(url)
            if response.status_code == 200:
                soup = BeautifulSoup(response.text, "html.parser")
                return soup
        except:
            print("can not connect to url")
            exit()

    def Get_dates_for_Canterbury(self, postcode, HouseNum):
        # Get usrn and uprn from canterbury website, if the user doesn't pass it in
        soup = self.scrape_dates_from_URl(
            "https://www.canterbury.gov.uk/bins-and-waste/find-your-bin-collection-dates?postcode="
            + postcode
        )
        option = soup.select("option")
        for address in option:
            addressLine = address.text
            if HouseNum in addressLine:
                Values = address["value"]
                Values = Values.split(",")
                uprn = Values[0]
                usrn = Values[1]
                break
        # Get the bin days and retunrs it as a dictortay of type: day
        website_url = (
            "https://www.canterbury.gov.uk/bins-and-waste/find-your-bin-collection-dates?uprn="
            + uprn
            + "&usrn="
            + usrn
        )
        soup = self.scrape_dates_from_URl(website_url)
        SpanLarge = soup.select("span.large")
        Strong = soup.select("strong")
        try:
            combined_dict = {
                Strong[i].get_text(): SpanLarge[i].get_text()
                for i in range(len(SpanLarge))
            }
            return combined_dict
        except:
            return []

    def run(self, location):
        locations = location.split(" ")

        if locations[0] == "Canterbury":
            return self.Get_dates_for_Canterbury(locations[1], locations[2])
        else:
            print("We do not have that bin area sorry")
            exit()

    # Check if the bin c

    def IsTomorrow(self, checkDate, tomorrow):
        # Parse the date string into a datetime object
        parsed_date = datetime.datetime.strptime(checkDate, "%A %d %B")
        tomorrow = datetime.datetime.strptime(tomorrow, "%A %d %B")
        # Compare the day and month of the parsed date with today's date
        if (parsed_date.day, parsed_date.month) == (tomorrow.day, tomorrow.month):
            return True
        else:
            return False
