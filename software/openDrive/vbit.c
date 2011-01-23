
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"


void print_vbit(uint8_t *arr, uint8_t size) {
	
	for(int i=size-1; i>=0; i--) {
		debug_printf("%c",arr[i]);
	}

}

uint8_t vbit_ext_ui8(uint8_t *vvar, uint8_t start, uint8_t end, uint8_t *result) {

	uint8_t abyte, mask;
	uint8_t bytes_cnt = (end / 8 -  start / 8 + 1);

	int imin, imax, move;
	for(int i=0; i<bytes_cnt; i++) {

		abyte = start/8 + i;
		move = abyte * 8 - start; // - = >>; + = <<;
									
		
		imin = abyte * 8;
		imax = abyte * 8 + 8-1;

		mask = 0xFF;
		mask &= (0xFF << (((start>imin)?start:imin) - imin));
		mask &= (0xFF >> (imax - ((end<imax)?end:imax)));

		
		if(move < 0) {
			*result += (vvar[abyte] & mask) >> abs(move);
		} else {
			*result += (vvar[abyte] & mask) << move;
		}
		
	}

	return 0;

}

uint8_t vbit_ext_ui16(uint8_t *vvar, uint8_t start, uint8_t end, uint16_t *result) {

	uint8_t abyte, mask;
	uint8_t bytes_cnt = (end / 8 -  start / 8 + 1);

	int imin, imax, move;
	for(int i=0; i<bytes_cnt; i++) {

		abyte = start/8 + i;
		move = abyte * 8 - start; // - = >>; + = <<;
									
		
		imin = abyte * 8;
		imax = abyte * 8 + 8-1;

		mask = 0xFF;
		mask &= (0xFF << (((start>imin)?start:imin) - imin));
		mask &= (0xFF >> (imax - ((end<imax)?end:imax)));

		
		if(move < 0) {
			*result += (vvar[abyte] & mask) >> abs(move);
		} else {
			*result += (vvar[abyte] & mask) << move;
		}
		
	}

	return 0;

}

uint8_t vbit_ext_ui32(uint8_t *vvar, uint8_t start, uint8_t end, uint16_t *result) {

	uint8_t abyte, mask;
	uint8_t bytes_cnt = (end / 8 -  start / 8 + 1);

	int imin, imax, move;
	for(int i=0; i<bytes_cnt; i++) {

		abyte = start/8 + i;
		move = abyte * 8 - start; // - = >>; + = <<;
									
		
		imin = abyte * 8;
		imax = abyte * 8 + 8-1;

		mask = 0xFF;
		mask &= (0xFF << (((start>imin)?start:imin) - imin));
		mask &= (0xFF >> (imax - ((end<imax)?end:imax)));

		
		if(move < 0) {
			*result += (vvar[abyte] & mask) >> abs(move);
		} else {
			*result += (vvar[abyte] & mask) << move;
		}
		
	}

	return 0;

}



