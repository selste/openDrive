#ifndef __openDrive_TYPES_H 
#define __openDrive_TYPES_H

#include <stdint.h>
#include "opendrive-config.h"


typedef struct {    
	uint32_t uxSourceID;
	unsigned char ucValue[OD_CMDLENGTH];	    
} CMDParser_CMDQueue_t;

typedef struct {
	uint32_t uxSourceID;
	unsigned char ucValue[OD_CMDLENGTH];	
} xUART2_SendQueue_t;


#endif /* end __openDrive_TYPES_H */
