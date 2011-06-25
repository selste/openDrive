#ifndef _VBIT_H_
#define _VBIT_H_


//Macros
#define _VBIT_SET(arr,bit)			arr[(bit/8)] |= (1<<(bit%8))
#define _VBIT_CLEAR(arr,bit)		arr[(bit/8)] &= ~(1<<(bit%8))
#define _VBIT_IS_SET(arr,bit)		(arr[(bit/8)] & (1<<(bit%8)))
#define _VBIT_IS_CLEAR(arr,bit)		(!(arr[(bit/8)] & (1<<(bit%8))))

//Functions
void print_vbit(uint8_t *arr, uint8_t size);
uint8_t vbit_ext_ui8(uint8_t *vvar, uint8_t start, uint8_t end, uint8_t *result);
uint8_t vbit_ext_ui16(uint8_t *vvar, uint8_t start, uint8_t end, uint16_t *result);
uint8_t vbit_ext_ui32(uint8_t *vvar, uint8_t start, uint8_t end, uint32_t *result);



#endif
