/*
 * prototipos.h
 *
 * Created: 26/09/2021 18:17:38
 *  Author: Ana Flavia
 */ 

#ifndef PROTOTIPOS_H_
#define PROTOTIPOS_H_

#define F_CPU 16000000UL
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1

typedef struct tempos_semaforo
{
	uint16_t tempAmarelo;
	uint16_t tempVerde;
	uint16_t tempVermelho;
} tempos_semaforo;

uint32_t flagModo = 0;
uint32_t flag_5000ms = 0;
uint32_t flag_500ms = 0;
uint32_t qtdCarros = 0;
uint32_t carros_min = 0;
uint32_t tempo_ms = 0;
uint32_t lux = 0;
float temperatura = 0;
int j = 0;

tempos_semaforo tempos = {.tempAmarelo = 1000, .tempVerde = 2000, .tempVermelho = 2000};
uint32_t cursorY = 40; //posição vertical inicial do cursor no lcd

int main(void);

void lcd();

void estima_carros(uint32_t *flag_disparo);

void leitura_sensores_ADC(uint32_t *flag_disparo);

void executa_timer(tempos_semaforo temp_sema);

#endif /* PROTOTIPOS_H_ */