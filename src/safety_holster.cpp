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

#include "mraa.hpp"

#include "UdpClient.hpp"
#include <grove.h>
#include <signal.h>
#include <ublox6.h>
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

	int gunDrawn = 750;
	int magFieldAvg = 0;
	int magFieldCurrent = 0;
	int magField[10];
	int tempIndex = 0;
	int numSamples = 5;

	// check that we are running on Galileo or Edison
	mraa_platform_t platform = mraa_get_platform_type();
	if ((platform != MRAA_INTEL_GALILEO_GEN1) &&
			(platform != MRAA_INTEL_GALILEO_GEN2) &&
			(platform != MRAA_INTEL_EDISON_FAB_C)) {
		std::cerr << "Unsupported platform, exiting" << std::endl;
		return MRAA_ERROR_INVALID_PLATFORM;
	}

	/*// UdpClient class is wrapper for sending UDP data to iotkit-agent
	UdpClient client;
	if (client.connectUdp(NODE, SERVICE) < 0) {
		std::cerr << "Connection to iotkit-agent failed, exiting" << std::endl;
		return MRAA_ERROR_UNSPECIFIED;
	}

	// create an analog input object from MRAA using pin A0
	mraa::Aio* a_pin = new mraa::Aio(0);
	if (a_pin == NULL) {
		std::cerr << "Can't create mraa::Aio object, exiting" << std::endl;
		return MRAA_ERROR_UNSPECIFIED;
	}

	// loop forever sending the input value every second
    for (;;) {
		uint16_t data = a_pin->read();
		std::stringstream ss;
		ss << "{\"n\":\"" << COMP_NAME << "\",\"v\":" << data << "}" << std::endl;
		client.writeData(ss);
		sleep(1);
	}*/


	// Read in hall sensor data
	/*if (a_pin == NULL) {
		std::cerr << "Can't create mraa::Aio object, exiting" << std::endl;
		return MRAA_ERROR_UNSPECIFIED;
	}*/


	// GPS Setup

	// make sure port is initialized properly.  9600 baud is the default.
	if (!nmea->setupTty(B9600))
	{
	  cerr << "Failed to setup tty port parameters" << endl;
	  return 1;
	}


	//this is only inteded to collect NMEA data and not process it
	// should see output on console

	char nmeaBuffer[bufferLength];

	// loop forever printing the input value every second
	while(0){
		/*uint16_t pin_value = a_pin->read();
		std::cout << "analog input value " << pin_value << std::endl;
		std::cout << "mag field average " << magFieldAvg << std::endl;

		// Calculate magnetic field average
		magFieldAvg = 0;
		if (pin_value > 850) {
		  if (magFieldCurrent == 0) {
		    tempIndex = numSamples-1;
		  } else {
		    tempIndex = magFieldCurrent - 1;
		  }
		  magField[magFieldCurrent++] = magField[tempIndex];
		} else {
		  magField[magFieldCurrent++] = pin_value;
		}
		if (magFieldCurrent >= numSamples) {
		  magFieldCurrent = 0;
		}
		for (int i = 0;i<numSamples;i++){
			magFieldAvg += magField[i];
		}
		magFieldAvg /= numSamples;
		if (magFieldAvg > gunDrawn) {
		  ledRed->on();
		  ledGreen->off();
		} else {
		  ledRed->off();
		  ledGreen->on();
		}*/

		sleep(1);


		if(button->value() == 1) {
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
		        	 // some sort of read error occured
		              cerr << "Port read error." << endl;
		              break;
		          }
		        }

	      ledRed->on();
		  ledGreen->off();
		}
	}

	// Delete the Grove LED object
	ledGreen->off();
	ledRed->off();

	delete ledGreen;
	delete ledRed;
	return MRAA_SUCCESS;
}
