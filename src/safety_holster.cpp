/*
 * Author: Thomas Lyet <thomas.lyet@intel.com>
 *
 * Copyright (c) 2015 Intel Corporation.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
// ip address 10.60.0.145
#include "mraa.hpp"

#include "UdpClient.hpp"
#include <grove.h>
#include <signal.h>
#include <ublox6.h>
#include <a110x.h>
#include <stdio.h>
#include <curl/curl.h>
#include <iostream>
#include <sstream>
#include <string>

using namespace upm;
using namespace std;

const size_t bufferLength = 256;

/*
 * IoT Cloud Analytics Example
 *
 * Demonstrate how to continuously send data to the IoT Cloud Analytics
 * (https://dashboard.us.enableiot.com/)
 * Read an analog voltage value from an input pin using the MRAA library,
 * then send its value to the IoT Cloud Analytics.
 * Any sensor that outputs a variable voltage can be used with this example
 * code. Suitable ones in the Grove Starter Kit are the Rotary Angle Sensor,
 * Light Sensor, Sound Sensor, Temperature Sensor.
 *
 * - analog in: analog sensor connected to pin A0 (Grove Base Shield Port A0)
 *
 * Additional linker flags: none
 */

/*
 * Preliminary Step
 *
 * Follow the IoT Cloud Analytics Getting Started Guide:
 * http://www.intel.com/support/motherboards/desktop/sb/CS-035346.htm
 *
 * Please check if the iotkit-agent is active on your device via
 *  $ systemctl status iotkit-agent
 * If not, activate it with
 *  $ systemctl start iotkit-agent
 *
 * Check the date of your device! It is this date that will be registered
 * in IoT Cloud Analytics.
 */

/*
 * NODE (host) and SERVICE (port)
 * iotkit-agent is listening for UDP data
 * as defined in /etc/iotkit-agent/config.json
 */
#define NODE "localhost"
#define SERVICE "41234"

/*
 * COMP_NAME is defined when a component is registered on the device
 *  $ iotkit-admin register ${COMP_NAME} ${CATALOG_ID}
 * In this example :
 *  $ iotkit-admin register temperature temperature.v1.0
 */
#define COMP_NAME "temperature"

int main()
{
	// Create the Grove LED object using GPIO pin 4
	upm::GroveLed* ledRed = new upm::GroveLed(4);
	upm::GroveLed* ledGreen = new upm::GroveLed(3);
	// create an analog input object from MRAA using pin A0
	mraa::Aio* a_pin = new mraa::Aio(0);
	// Create the button object using GPIO pin 8
	upm::GroveButton* button = new upm::GroveButton(8);
	// Instantiate a Ublox6 GPS device on uart 0.
	upm::Ublox6* nmea = new upm::Ublox6(0);

	int gunDrawn = 100;
	int magFieldAvg = 0;
	int magFieldCurrent = 0;
	int magField[10];
	int tempIndex = 0;
	int numSamples = 2;
	string tempData;

	// check that we are running on Galileo or Edison
	mraa_platform_t platform = mraa_get_platform_type();
	if ((platform != MRAA_INTEL_GALILEO_GEN1) &&
			(platform != MRAA_INTEL_GALILEO_GEN2) &&
			(platform != MRAA_INTEL_EDISON_FAB_C)) {
		std::cerr << "Unsupported platform, exiting" << std::endl;
		return MRAA_ERROR_INVALID_PLATFORM;
	}

	// Read in hall sensor data
	if (a_pin == NULL) {
		std::cerr << "Can't create mraa::Aio object, exiting" << std::endl;
		return MRAA_ERROR_UNSPECIFIED;
	}


	// GPS Setup

	// make sure port is initialized properly.  9600 baud is the default.
	if (!nmea->setupTty(B9600))
	{
	  cerr << "Failed to setup tty port parameters" << endl;
	  return 1;
	}

	// Curl setup
	//followed this curl example: http://curl.haxx.se/libcurl/c/http-post.html
	CURL *curl;
	CURLcode res;
	// In windows, this will init the winsock stuff
	curl_global_init(CURL_GLOBAL_ALL);
	// get a curl handle
	curl = curl_easy_init();
	// First set the URL that is about to receive our POST. This URL can
	//   just as well be a https:// URL if that is what should receive the
	//   data.
	curl_easy_setopt(curl, CURLOPT_URL, "https://flickering-inferno-5440.firebaseio.com/data.json");

	//this is only intended to collect NMEA data and not process it
	// should see output on console

	char nmeaBuffer[bufferLength];
	//char tempCharArray[10];
	// loop forever printing the input value every second
	while(1){
		uint16_t pin_value = a_pin->read();
		//std::cout << "analog input value " << pin_value << std::endl;
		//std::cout << "mag field average " << magFieldAvg << std::endl;

		// Calculate magnetic field average
		magFieldAvg = 0;

	    magField[magFieldCurrent++] = pin_value;

		if (magFieldCurrent >= numSamples) {
		  magFieldCurrent = 0;
		}
		for (int i = 0;i<numSamples;i++){
			magFieldAvg += magField[i];
		}
		magFieldAvg /= numSamples;

		sleep(1);

		if(magFieldAvg < gunDrawn) {
		  ledRed->off();
		  ledGreen->on();
		} else {
		      if (nmea->dataAvailable())
		        {
		          int rv = nmea->readData(nmeaBuffer, bufferLength);

		          if (rv > 0) {
		            write(1, nmeaBuffer, rv);
		            std::cout << nmeaBuffer << std::endl;
		          } else {
		        	 // some sort of read error occurred
		              cerr << "Port read error." << endl;
		              break;
		          }

		          //tempData = "{\"nmeaData\":\"";
		          //for(int i = 70;i<81;i++){
		          //	  tempCharArray[i-70] = nmeaBuffer[i];
		          // }

		          //tempData = tempData + string(nmeaBuffer) + "\"}";
		          //tempData.append(nmeaBuffer);
		          //tempData.append("\"}");


				  // Now specify the POST data

				  //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, tempData.c_str());
				  //std::cout << tempData.c_str() << std::endl;
				  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "{\"gunDrawn\":\"true\"}");

				  // Perform the request, res will get the return code
				  res = curl_easy_perform(curl);
				  // Check for errors
				  //std::cout << "curl output: " << res << std::endl;
				  if(res != CURLE_OK)
					fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));


		        }

	      ledRed->on();
		  ledGreen->off();
		}
		if (button->value() == 1) {
			break;
		}
	}

	// Delete the Grove LED object
	ledGreen->off();
	ledRed->off();

	delete ledGreen;
	delete ledRed;
	delete a_pin;
	delete button;
	delete nmea;

	return MRAA_SUCCESS;
}
