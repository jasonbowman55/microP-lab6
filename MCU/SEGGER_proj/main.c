/*
File: Lab_6_JHB.c
Author: Josh Brake
Email: jbrake@hmc.edu
Date: 9/14/19
*/


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "main.h"

/////////////////////////////////////////////////////////////////
// Provided Constants and Functions
/////////////////////////////////////////////////////////////////

//Defining the web page in two chunks: everything before the current time, and everything after the current time
char* webpageStart = "<!DOCTYPE html><html><head><title>E155 Web Server Demo Webpage</title>\
	<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
	</head>\
	<body><h1>E155 Web Server Demo Webpage</h1>";
char* ledStr = "<p>LED Control:</p><form action=\"ledon\"><input type=\"submit\" value=\"Turn the LED on!\"></form>\
	<form action=\"ledoff\"><input type=\"submit\" value=\"Turn the LED off!\"></form>";

char* tempStr = "<p>Tempurature Resolution:</p><form action=\"8bit\"><input type=\"submit\" value=\"8 Bit\"></form>\
	<form action=\"9bit\"><input type=\"submit\" value=\"9 Bit\"></form>\
        <form action=\"10bit\"><input type=\"submit\" value=\"10 Bit\"></form>\
        <form action=\"11bit\"><input type=\"submit\" value=\"11 Bit\"></form>\
        <form action=\"12bit\"><input type=\"submit\" value=\"12 Bit\"></form>";

char* webpageEnd   = "</body></html>";

//determines whether a given character sequence is in a char array request, returning 1 if present, -1 if not present
int inString(char request[], char des[]) {
	if (strstr(request, des) != NULL) {return 1;}
	return -1;
}

int updateLEDStatus(char request[])
{
	int led_status = 0;
	// The request has been received. now process to determine whether to turn the LED on or off
	if (inString(request, "ledoff")==1) {
		digitalWrite(PB6, PIO_LOW);
		led_status = 0;
	}
	else if (inString(request, "ledon")==1) {
		digitalWrite(PB6, PIO_HIGH);
		led_status = 1;
	}

	return led_status;
}



/////////////////////////////////////////////////////////////////
// Solution Functions
/////////////////////////////////////////////////////////////////
// decond the tempurature to be displayed later ******
float decodeTemp(uint8_t lsb, uint8_t msb) {
  int8_t wholeNumberTemp = 0;
  if(msb>>7 == 1){                        //if the sign bit is 1
    wholeNumberTemp = -128+(msb & 0x7F);  //mask 0->6 and assign -128 starter max neg val starter
  } else {
    wholeNumberTemp = (msb & 0x7F);       //mask 0->6 resulting in 0 positive min neg val starter
  }
  

  //NOTE: liniearly extrapolate tempVal assignments from the 0x80 value found on the data sheet
  double tempVal = 0.0; //init double temp val
  if (lsb & 0x80) {         //if largest bit it is on
    tempVal += 0.5;     //add coresponding decimal value as found in data sheet
  }
  if (lsb & 0x40) {         //if next largest bit is on
    tempVal += 0.25;    //add coresponding decimal value as calculated
  }
  if (lsb & 0x20) {         //if next largest bit is on
    tempVal += 0.125;   //add coresponding decimal value as calculated
  }
  if (lsb & 0x10) {         //if next largest bit is on
    tempVal += 0.0625;  //add coresponding decimal value as calculated
  }

  return tempVal + (double)wholeNumberTemp; //once all bits are checked, return the 0->6 bit calculated decimal temp plus the logic for determining positive and negative (wholeNumberTemp_
}
//*****

float updateTemperature(char request[]) {
  uint8_t lsbTemp = 0;
  uint8_t msbTemp = 0;

  if (inString(request, "8bit")==1) {
    digitalWrite(PB1, 1);
    spiSendReceive(0x80);         //write address enable to configuration/status register
    spiSendReceive(0b11100000);   //8-bit configuration into configuration/status register 
    digitalWrite(PB1, 0);
  }
  else if (inString(request, "9bit")==1) {
    digitalWrite(PB1, 1);
    spiSendReceive(0x80);         //write address location to configuration/status register
    spiSendReceive(0b11100010);   //9-bit configuration into configuration/status register
    digitalWrite(PB1, 0);
  }
  else if (inString(request, "10bit")==1) {
    digitalWrite(PB1, 1);
    spiSendReceive(0x80);         //write address location to configuration/status register
    spiSendReceive(0b11100100);   //10-bit configuration into configuration/status register
    digitalWrite(PB1, 0);
  }
  else if (inString(request, "11bit")==1) {
    digitalWrite(PB1, 1);
    spiSendReceive(0x80);         //write address location to configuration/status register
    spiSendReceive(0b11100110);   //11-bit configuration into configuration/status register
    digitalWrite(PB1, 0);
  }
  else if (inString(request, "12bit")==1) {
    digitalWrite(PB1, 1);
    spiSendReceive(0x80);         //write address location to configuration/status register
    spiSendReceive(0b11101110);   //12-bit configuration into configuration/status register
    digitalWrite(PB1, 0);
  }
  
    digitalWrite(PB1, 1);
    spiSendReceive(0x01);           //read address location for tempurature LSB
    lsbTemp = spiSendReceive(0x00); //initializing so that you can receive later
    digitalWrite(PB1, 0);

    digitalWrite(PB1, 1);
    spiSendReceive(0x02);           //read address location for tempurature MSB
    msbTemp = spiSendReceive(0x00); //initializing so that you can receive later
    digitalWrite(PB1, 0);
    
    return decodeTemp(lsbTemp,msbTemp);
}

int main(void) {

  double temp = 0;

  configureFlash();
  configureClock();

  //enable all GPIO peripherals
  gpioEnable(GPIO_PORT_A);
  gpioEnable(GPIO_PORT_B);
  gpioEnable(GPIO_PORT_C);

  // enable PB0 to be output to toggle the LED
  pinMode(PB6, GPIO_OUTPUT);
  
  // enable timer clk and initialize timer peripheral
  RCC->APB2ENR |= (RCC_APB2ENR_TIM15EN);
  initTIM(TIM15);
  
  // init USART at 125000 baud rate
  USART_TypeDef * USART = initUSART(USART1_ID, 125000);

  // enable SPI I/Os
  pinMode(PB3, GPIO_ALT); // SPI1_SCK
  pinMode(PB4, GPIO_ALT); // SPI1_MISO
  pinMode(PB5, GPIO_ALT); // SPI1_MOSI

  // Configure alternate function MUXs routing SPI lines correctly
  GPIOB->AFR[0] |= (0b101 << GPIO_AFRL_AFSEL3_Pos);   //AF5
  GPIOB->AFR[0] |= (0b101 << GPIO_AFRL_AFSEL4_Pos);   //AF5  **ALL Bs NOW**
  GPIOB->AFR[0] |= (0b101 << GPIO_AFRL_AFSEL5_Pos);   //AF5

  // CE enable SPI communitcation line
  pinMode(PB1, GPIO_OUTPUT);
  
  // initialize SPI peripheral with baud rate, polarity, and phase
  initSPI(9600, 0, 1);

  // enable SPI communication and write an initial 10-bit configureation
  digitalWrite(PB1, 1);
  spiSendReceive(0x80);  
  spiSendReceive(0b11100100);
  digitalWrite(PB1, 0);

  digitalWrite(PB1, 1);
  spiSendReceive(0x00);  
  spiSendReceive(0xAA);
  digitalWrite(PB1, 0);

  while(1) {
    /* Wait for ESP8266 to send a request.
    Requests take the form of '/REQ:<tag>\n', with TAG begin <= 10 characters.
    Therefore the request[] array must be able to contain 18 characters.
    */

    // Receive web request from the ESP
    char request[BUFF_LEN] = "                  "; // initialize to known value
    int charIndex = 0;
  
    // Keep going until you get end of line character
    while(inString(request, "\n") == -1) {
      // Wait for a complete request to be transmitted before processing
      while(!(USART->ISR & USART_ISR_RXNE));
      request[charIndex++] = readChar(USART);
    }

    // update the current tempurature value from the updateTempurature block
    float currentTemp = updateTemperature(request);

    // update integer with read LED value
    int led_status = updateLEDStatus(request);

    // ***** print the interface of the website *****
    char ledStatusStr[20];
    if (led_status == 1)
      sprintf(ledStatusStr,"LED ON");
    else if (led_status == 0)
      sprintf(ledStatusStr,"LED OFF");

    char temp_send[30];
    sprintf(temp_send,"Temperature: %f C", currentTemp);
    // **********

    // send webpage over UART..
    sendString(USART, webpageStart); // start the webpage
    sendString(USART, ledStr);       // send LED data to website
    sendString(USART, tempStr);      // send temp data to website

    //send temp and led strings to website
    sendString(USART, "<h2>Temperature Reading</h2>");
    sendString(USART, "<p>");
    sendString(USART, temp_send);
    sendString(USART, "</p>");

    sendString(USART, "<h2>LED Status</h2>");
    sendString(USART, "<p>");
    sendString(USART, ledStatusStr);
    sendString(USART, "</p>");

    sendString(USART, webpageEnd); // end the webpage
  }
}