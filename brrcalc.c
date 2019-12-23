#include <stdio.h>
#include <stdint.h>
int main(){
    int BaudRate=38400;
    int CLK=4000000;
    uint32_t BaudRate_Mantissa    = ((uint32_t)CLK / (BaudRate << 4));
    uint32_t BaudRate_Mantissa100 = (((uint32_t)CLK * 100) / (BaudRate << 4));
    /* Set the fraction of UART1DIV  */
    uint8_t BRR2 = (uint8_t)((uint8_t)(((BaudRate_Mantissa100 - (BaudRate_Mantissa * 100)) << 4) / 100) & (uint8_t)0x0F); 
    /* Set the MSB mantissa of UART1DIV  */
    BRR2 |= (uint8_t)((BaudRate_Mantissa >> 4) & (uint8_t)0xF0); 
    /* Set the LSB mantissa of UART1DIV  */
    uint8_t BRR1 = (uint8_t)BaudRate_Mantissa;           
    printf("BRR1 %d, BRR2 %d\n",BRR1,BRR2);
    return 0;
}
	
