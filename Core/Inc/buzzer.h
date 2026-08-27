#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>

void BUZZER_Init(void);
void BUZZER_On(void);
void BUZZER_Off(void);
void BUZZER_Beep(uint16_t ms);
void BUZZER_Play(uint16_t frequency, uint16_t duration);
void BUZZER_Update(void);

#endif
