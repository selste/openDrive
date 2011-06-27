#include <stdio.h>

#define NR_LEDS 8
static char LEDARR[NR_LEDS+1] = {0,};

static void vLEDArray_Out() {
	printf("LED : %s\n",LEDARR);
}

static void vLEDArray_Set(unsigned char c) {
	int i;
	for ( i = 0; i < NR_LEDS; i ++) {
		if (((c >> i) & 1UL) == 1) {
			LEDARR[i] = 'I';
		} else {
			LEDARR[i] = 'o';
		}
	}
	vLEDArray_Out();
}

static inline
void vLEDPort_Init(void) {
	vLEDArray_Set(0);
}


static inline
void vLEDPort_Toggle(uint32_t *counter) {
	vLEDArray_Set(1 << *counter);
	(*counter)++;
	
	if ( *counter > 8 ) {
		*counter = 0;
		vLEDArray_Set(*counter);
	}
}
