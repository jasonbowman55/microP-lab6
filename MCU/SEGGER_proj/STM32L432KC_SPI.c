// STM32L432KC_SPI.c
// Jason Bowman
// jbowman@hmc.edu
// 11-3-24
// This will configure the SPI communications for our micro controller

#include "STM32L432KC_SPI.h"

void initSPI(int br, int cpol, int cpha) {
  
  RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;           // enable the SYSCLK to go into the SPI peripheral

  SPI1->CR1 |= (0b111 << SPI_CR1_BR_Pos);       // change baud rate to be fspclk/256

  SPI1->CR1 |= _VAL2FLD(SPI_CR1_CPHA, cpha);    // seeting clock phase
  SPI1->CR1 |= _VAL2FLD(SPI_CR1_CPOL, cpol);    // setting clock phase

  SPI1->CR1 |= _VAL2FLD(SPI_CR1_LSBFIRST, 0b0); // configure to not send with LSB first
  SPI1->CR1 |= _VAL2FLD(SPI_CR1_CRCEN, 0b0);    // not sure, but needs to be off for correct function of SPI

  SPI1->CR1 |= _VAL2FLD(SPI_CR1_SSM, 0b1);      // this is needed for communications
  SPI1->CR1 |= _VAL2FLD(SPI_CR1_SSI, 0b1);      // this is needed for communications

  SPI1->CR1 |= _VAL2FLD(SPI_CR1_MSTR, 0b1);     // this configures the MCU to be the master

  SPI1->CR2 |= (0b0111 << SPI_CR2_DS_Pos);      // configure to use 8-bit SPI
  SPI1->CR2 |= _VAL2FLD(SPI_CR2_SSOE, 0b1);     // this enables the bit for correction function as requested by being in master mode
  SPI1->CR2 |= _VAL2FLD(SPI_CR2_FRF, 0b0);      // this bit is only writtedn when SPI is disabled
  SPI1->CR2 |= _VAL2FLD(SPI_CR2_FRXTH, 0b1);    // this sets the thershold of the RXFIFO

  SPI1->CR1 |= SPI_CR1_SPE;                     // enable SPI1 peripheral
}


char spiSendReceive(char send) {
  
  
  while(!(SPI1->SR & SPI_SR_TXE)); // mask SPI1 reg with TXE transfer buffer, waiting for it to not be empty
  

  volatile uint8_t* ptrDR = (volatile uint8_t*)&SPI1->DR;
  *ptrDR = send;

  while(!(SPI1->SR & SPI_SR_RXNE)); // wait while the mask is empty

  return SPI1->DR;                  // return what is in the data register
}