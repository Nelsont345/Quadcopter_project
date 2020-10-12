/*------------------------------------------------------------------
 *  uart.c -- configures uart
 *
 *  I. Protonotarios
 *  Embedded Software Lab
 *
 *  July 2016
 *------------------------------------------------------------------
 */

#include "in4073.h"

bool txd_available = true;
bool start_flag = false;
uint8_t b_counter = 0;
uint16_t p_counter = 0;
uint16_t data[20];


void uart_put(uint8_t byte)
{
	NVIC_DisableIRQ(UART0_IRQn);

	if (txd_available) {txd_available = false; NRF_UART0->TXD = byte;}
	else enqueue(&tx_queue, byte);

	NVIC_EnableIRQ(UART0_IRQn);
}

// Reroute printf
int _write(int file, const char * p_char, int len)
{
	int i;
	for (i = 0; i < len; i++)
	{
		uart_put(*p_char++);
	}
	return len;
}


void UART0_IRQHandler(void)
{       intr_start_time = get_time_us();
	if (NRF_UART0->EVENTS_RXDRDY != 0)
	{       

		NRF_UART0->EVENTS_RXDRDY  = 0;
<<<<<<< HEAD
		printf("get data %lu \n",NRF_UART0->RXD);
=======
		//printf("get data %lu",NRF_UART0->RXD);
>>>>>>> Liang
		uint8_t k = NRF_UART0->RXD;
		
		if (b_counter == 0 && k == 0xFF)
		{
			b_counter++;
			//printf("get packet %u",p_counter);
		}
		else if (b_counter>0)
		{
			data[b_counter-1] = k;
<<<<<<< HEAD
			printf("get dataff%d %d",b_counter, k);
			b_counter++;
			
			if(b_counter==6)
			{
				command c = {data[0],data[1],data[2],data[3],data[4]};
				myenqueue( &myrx_queue, c);
				printf("%d",myrx_queue.count);
				b_counter = 0;
				p_counter++;
				//still need to check the packets
				printf("end packet\n");
=======
			//printf("get data%d %d",b_counter, k);
			b_counter++;
			if(b_counter==14)
			{
				//enqueue the command
				command c =
{data[0],data[1],(data[2]<<8)+data[3],(data[4]<<8)+data[5],(data[6]<<8)+data[7],(data[8]<<8)+data[9],data[10],data[11],data[12], intr_start_time};
				c_enqueue( &c_rx_queue, c);
				//printf("%d",c_rx_queue.count);
				b_counter = 0;
				p_counter++;
				//send ack
				uart_put(255);
				uart_put(c.frame);
				//still need to check the packets by CRC
				//printf("end packet\n");
>>>>>>> Liang
			}
		}
                // loop_time = get_time_us() - start_time;
	}

	if (NRF_UART0->EVENTS_TXDRDY != 0)
	{
		NRF_UART0->EVENTS_TXDRDY = 0;
		if (tx_queue.count) NRF_UART0->TXD = dequeue(&tx_queue);
		else txd_available = true;
	}

	if (NRF_UART0->EVENTS_ERROR != 0)
	{
		NRF_UART0->EVENTS_ERROR = 0;
		printf("uart error: %lu\n", NRF_UART0->ERRORSRC);
	}
        intr_stop_time = get_time_us();
        tot_intr_time += (intr_stop_time - intr_start_time);
}

void uart_init(void)
{
	init_queue(&rx_queue); // Initialize receive queue
<<<<<<< HEAD
	myinit_queue(&myrx_queue);
=======
	c_init_queue(&c_rx_queue);
>>>>>>> Liang
	init_queue(&tx_queue); // Initialize transmit queue

	nrf_gpio_cfg_output(TX_PIN_NUMBER);
	nrf_gpio_cfg_input(RX_PIN_NUMBER, NRF_GPIO_PIN_NOPULL); 
	NRF_UART0->PSELTXD = TX_PIN_NUMBER;
	NRF_UART0->PSELRXD = RX_PIN_NUMBER;
	NRF_UART0->BAUDRATE        = (UART_BAUDRATE_BAUDRATE_Baud115200 << UART_BAUDRATE_BAUDRATE_Pos);

	NRF_UART0->ENABLE           = (UART_ENABLE_ENABLE_Enabled << UART_ENABLE_ENABLE_Pos);
	NRF_UART0->EVENTS_RXDRDY    = 0;
	NRF_UART0->EVENTS_TXDRDY    = 0;
	NRF_UART0->TASKS_STARTTX    = 1;
	NRF_UART0->TASKS_STARTRX    = 1;

	NRF_UART0->INTENCLR = 0xffffffffUL;
	NRF_UART0->INTENSET = 	(UART_INTENSET_RXDRDY_Set << UART_INTENSET_RXDRDY_Pos) |
                          	(UART_INTENSET_TXDRDY_Set << UART_INTENSET_TXDRDY_Pos) |
                          	(UART_INTENSET_ERROR_Set << UART_INTENSET_ERROR_Pos);

	NVIC_ClearPendingIRQ(UART0_IRQn);
	NVIC_SetPriority(UART0_IRQn, 3); // either 1 or 3, 3 being low. (sd present)
	NVIC_EnableIRQ(UART0_IRQn);
}
