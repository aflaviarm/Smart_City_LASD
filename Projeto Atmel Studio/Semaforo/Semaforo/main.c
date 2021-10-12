/*
 * Semaforo.c
 *
 * Created: 11/07/2021 20:36:28
 * Author : Ana Flavia
 */ 

#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "nokia5110.h"
#include "prototipos.h"

////////////////////////////-----TIMER----////////////////////////////////////////

ISR(TIMER0_COMPA_vect){
	tempo_ms++;
	
	if((tempo_ms % 500) == 0){
		flag_500ms = 1;
	}
	if((tempo_ms % 5000) == 0){
		flag_5000ms = 1;
	}
}

ISR(PCINT1_vect){
	if(!(PINC & (1<<6))){
		switch(cursorY){ //testa em qual posição do lcd está o cursor para modificar o tempo da respectiva cor no semáforo
			case 0:
			if(flagModo == 0)
			flagModo = 1;
			else{
				flagModo = 0;
			}
			break;
			case 20:
			if(tempos.tempVerde > 1000){
				tempos.tempVerde -= 1000;
			}
			break;
			case 30:
			if(tempos.tempVermelho > 1000){
				tempos.tempVermelho -= 1000;
			}
			break;
			case 40:
			if(tempos.tempAmarelo > 1000){
				tempos.tempAmarelo -= 1000;
			}
			break;
			default:
			break;
		}
	
		lcd();
	}
}

ISR(PCINT2_vect){ //Mudança de estado do pino PD4
	if(!(PIND & (1<<5))){
		qtdCarros++;
	}
	
	if(!(PIND & (1<<6))){ //caso o botão da passagem de pedestres seja acionado
		if(j < 3) //se o semáforo estiver verde para os veículos
			j = 4; //o contador da animação do semáforo é setado para a posição do amarelo
	}
	
	if(flagModo == 0){
		if(!(PIND & (1<<4))){
			if(cursorY == 20){
				cursorY = 0;
			}
			else if(cursorY == 0){
				cursorY = 40;
			}
			else{
				cursorY -= 10;
			}
		}
		lcd();
	}
}

ISR(INT0_vect){ //Interrupção externa INT0, quando o botão é pressionado o tempo será aumentado
	switch(cursorY){ //testa em qual posição do lcd está o cursor para modificar o tempo da respectiva cor no semáforo 
		case 0:
			if(flagModo == 0)
				flagModo = 1;
			else{
				flagModo = 0;
			}
			break;
		case 20:
			if(tempos.tempVerde < 9000){
				tempos.tempVerde += 1000;
			}
			break;
		case 30:
			if(tempos.tempVermelho < 9000){
				tempos.tempVermelho += 1000;
			}
			break;
		case 40: 
			if(tempos.tempAmarelo < 9000){
				tempos.tempAmarelo += 1000;
			}
			break;
		default:
			break;
	}
	
	lcd();
}

void lcd(){
	int fator_mult;
	
	unsigned char carros_min_string[4];
	sprintf(carros_min_string, "%u", carros_min);
	
	unsigned char tempAmarelo_string[2];	
	unsigned char tempVerde_string[2];
	unsigned char tempVermelho_string[2];
	sprintf(tempAmarelo_string, "%u", tempos.tempAmarelo/1000);
	sprintf(tempVerde_string, "%u", tempos.tempVerde/1000);
	sprintf(tempVermelho_string, "%u", tempos.tempVermelho/1000);
	
	unsigned char modo_string[2];
	modo_string[0] = (flagModo) ? 'A' : 'M'; modo_string[1]='\0';
	
	unsigned char lux_LDR[4];
	sprintf(lux_LDR, "%u", lux);
	
	unsigned char temperatura_string[5];
	fator_mult = (temperatura < 10) ? 1 : 10; //o valor pode ser multipicado por 1 caso seja um valor abaixo de 10
	sprintf(temperatura_string, "%u", (uint32_t)(temperatura*fator_mult)); //multiplica por um fator de multiplicação para não perder a primeira casa decimal
	temperatura_string[4] = temperatura_string[3]; //Desloca o número mais a esquerda
	temperatura_string[3] = temperatura_string[2]; //outro deslocamento
	temperatura_string[2] = '.'; //insere o ponto
	
	nokia_lcd_clear();		
	nokia_lcd_write_string("Modo", 1);
	nokia_lcd_set_cursor(30, 0);
	nokia_lcd_write_string(modo_string, 1);
	nokia_lcd_set_cursor(0, 10);
	nokia_lcd_write_string("------", 1);
	nokia_lcd_set_cursor(0, 20);
	nokia_lcd_write_string("T.Vd ", 1);
	nokia_lcd_write_string(tempVerde_string, 1);
	nokia_lcd_set_cursor(0, 30);
	nokia_lcd_write_string("T.Vm ", 1);
	nokia_lcd_write_string(tempVermelho_string, 1);
	nokia_lcd_set_cursor(0, 40);
	nokia_lcd_write_string("T.Am ", 1);
	nokia_lcd_write_string(tempAmarelo_string, 1);
	
	nokia_lcd_set_cursor(38, cursorY);
	nokia_lcd_write_char('<', 1);
	
	for(int i = 0; i < 42; i++){
		nokia_lcd_write_string("|", 1);
		nokia_lcd_set_cursor(43, i);
	}
	
	nokia_lcd_set_cursor(48, 0);
	nokia_lcd_write_string(temperatura_string, 1);
	nokia_lcd_set_cursor(72, 0);
	nokia_lcd_write_string(" C", 1);
	
	nokia_lcd_set_cursor(60, 10);
	nokia_lcd_write_string(lux_LDR, 1);
	nokia_lcd_set_cursor(60, 20);
	nokia_lcd_write_string("LUX", 1);
	
	nokia_lcd_set_cursor(65, 30);
	nokia_lcd_write_string(carros_min_string, 1);
	nokia_lcd_set_cursor(55, 40);
	nokia_lcd_write_string("c/min", 1);
	nokia_lcd_render();
}

void estima_carros(uint32_t *flag_disparo){	
	if(*flag_disparo){
		*flag_disparo = 0;
		
		if(flagModo){
			uint32_t carros_min_ant = carros_min;
			
			carros_min = qtdCarros*12;
			
			if(carros_min_ant != carros_min){
				tempos.tempVerde = (carros_min/60)*1000 + 1000;
				if(tempos.tempVerde > 9000){
					tempos.tempVerde = 9000;
				}
				tempos.tempVermelho = 9000 - (carros_min/60)*1000;
				if(tempos.tempVermelho > 32000){
					tempos.tempVermelho = 1000;
				}
				lcd();
			}
		}
		else{
			carros_min = 0;
		}
		
		qtdCarros = 0;
	}
}

void leitura_sensores_ADC(uint32_t *flag_disparo1){
	static uint8_t cont_canal = 0; //variável para seleção da leitura do canal
		
	uint32_t lux_ant = lux;
	float temperatura_ant = temperatura;
	
	if(*flag_disparo1){
		switch(cont_canal){
			case 0:
				lux = (1023000/ADC) - 1000;
				ADMUX = 0b01000001; // Muda para o canal 1
				break;
			case 1:
				temperatura = (((float)ADC)*50)/1023;
				ADMUX = 0b01000000; //Muda para o canal 0
				break;
		}
		
		if(cont_canal < 1){ //Atualização do contador do canal, como são 2 canais
			cont_canal++; //incrementa
		} else{
			cont_canal = 0; //senão volta pro canal 0
		}
		
		if(lux < 300){
			if((!(PIND & (1<<6))) || (qtdCarros > 0)){
				OCR2B = 255;
			}
			else{
				OCR2B = 77;
			}
		}
		else{
			OCR2B = 0;
		}
		
		*flag_disparo1 = 0;
		
		if((lux_ant != lux) || (temperatura_ant != temperatura))
			lcd();
	}
}

void executa_timer(tempos_semaforo temp_sema){
	static uint32_t tempo_inicio = 0;
	
	const uint16_t estados[9] = {0b000001111, 0b000000111, 0b000000011, 0b00000001, 0b100000000, 0b011110000, 0b001110000, 0b000110000, 0b000010000};
	
	PORTB = estados[j] & 0b011111111;
	if(estados[j] & 0b100000000)
		PORTD |= 0b10000000;
		
	else
		PORTD &= 0b01111111;
		
	if(j <= 3){
		PORTD &= 0b11111110;
		if((tempo_ms - tempo_inicio) >= (temp_sema.tempVerde/4)){
			j++;
			tempo_inicio += (temp_sema.tempVerde/4);
		}
	}
	else{
		if (j <= 4){
			if((tempo_ms - tempo_inicio) >= (temp_sema.tempAmarelo)){
				j++;
				tempo_inicio += (temp_sema.tempAmarelo);
			}
		}
		else{
			if (j <= 8){
				PORTD |= 0b00000001;
				if((tempo_ms - tempo_inicio) >= (temp_sema.tempVermelho/4)){
					j++;
					tempo_inicio += (temp_sema.tempVermelho/4);
				}
			}
			else{
				j = 0;
				tempo_inicio = tempo_ms;
			}
		}
	}
}

int main(void)
{
	/* Replace with your application code */
	DDRB = 0b11111111; //habilita todos os 8 pinos de PB como saída
	DDRC = 0b0111100; //habilita PC0 e PC1 como entrada e o restante dos pinos de PC como saída
	DDRD = 0b10001011; //habilita os pinos PD7, PD3, PD1, PD0 como saída e deixa o restante como entrada
	PORTD |= 0b01110100; //habilita o resistor de pull-up dos pinos PD2, PD4, PD5 e PD6
	PORTC |= 0b1000000; //habilita o resistor de pull-up do pino PC6
	
	//configuração das interrupções
	EICRA = 0b00001010; // interrupção externa INT0 e INT1 na borda de descida
	EIMSK = 0b00000011; //habilita as interrupções externas INT0 INT1
	PCICR = 0b0000110; //Interrupção externa dos pinos D e C
	PCMSK2 = 0b01110000; //habilita as interrupções externas dos pinos PD4, PD6 e PD5
	PCMSK1 = 0b1000000; //habilita as interrupções externas dos pinos PC6
	
	TCCR0A = 0b00000010; //habilita o moto CTC do TC0
	TCCR0B = 0b00000011; //liga o TC0 com prescaler = 64
	OCR0A = 249; //ajusta o comparador para o TC0 contar até 249
	TIMSK0 = 0b00000010; //habilita a interrupção na igualdade de comparação com OCR0A. A interrupção irá ocorrer a cada 1ms, pois 1ms = 64*(249+1)/16*10^6
	
	TCCR2A = 0b00100011;
	TCCR2B = 0b00000100;
	ICR1 = 255;
	
	//Configuração do ADC
	ADMUX = 0b01000000; //VCC como ref, canal 0
	ADCSRA = 0b11100111; //habilita o A0, modo de conversão contínua, prescaler = 128
	ADCSRB = 0b00000000; //modo de conversão contínua
	DIDR0 = 0b00000000; //desabilita pino PC01 como entrada digital
	
	
	//Configuração da USART
	UBRR0H = (unsigned char) (MYUBRR>>8);
	UBRR0L = (unsigned char) MYUBRR;
	UCSR0B = (1<<RXCIE0)|(1<<RXEN0)|(1<<TXEN0);
	UCSR0C = (3<<UCSZ00);
	
	sei(); //habilita todas as interrupções
	
	//manipulando o lcd
	nokia_lcd_init();
	lcd();
	
	while (1)
	{
		executa_timer(tempos);
		estima_carros(&flag_5000ms);
		leitura_sensores_ADC(&flag_500ms);
	}
}
