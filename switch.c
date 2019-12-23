#include "includes.h"

#define DEBUG
#define HSI

#ifdef DEBUG
void UART_Send(const char *s) {
    char c;
    while ( (c = *s++) ) {
        while(!(UART1->SR & UART1_SR_TXE)) {}
        UART1_SendData8(c);
    }
}
void cout(const char c) {
    while(!(UART1->SR & UART1_SR_TXE)) {}
    UART1_SendData8(c);
}
void xcout(unsigned char i){
    int c;
    for(c=2;c>0;c--){
	char q=(i&0xF0)>>4;
	if(q<10){
	    cout('0'+q);
	}else{
	    cout('A'+q-10);
	}
	i<<=4;
    }
}
#else
#define UART_Send(x)
#endif

unsigned char *ring_pattern;
unsigned char rings[17][8]={
    {50,51,24,0,0,0,0,0},
    {100,101,200,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},

    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},

    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},

    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0}
};
struct alarm {
    uint8_t h,m,s,dow,pattern;
};
struct alarm alarms[16];

uint8_t th=0;
uint8_t tm=0;
uint8_t ts=0;
uint8_t dow=1;

void run_ring(){
    if(!ring_pattern) return;
    if(*ring_pattern){
        if(*ring_pattern&1){ //delay
            GPIO_WriteLow(GPIOD, GPIO_PIN_4);
            cout('d');
        }else{//ring
            GPIO_WriteHigh(GPIOD, GPIO_PIN_4);
            cout('r');
        }
        TIM1_SetCounter(*ring_pattern*4);
        TIM1_Cmd(ENABLE);
        ring_pattern++;
    }else{
        cout('e');
        GPIO_WriteLow(GPIOD, GPIO_PIN_4);
        TIM1_Cmd(DISABLE);
        ring_pattern=NULL;
    }
}

uint8_t rxp=0;
uint8_t rxbuf[9];
uint8_t rxto=0;
uint8_t handlerx=0;

void UART_RX(void) __interrupt(18) { 
    uint8_t c=UART1_ReceiveData8();
    if(c=='0' || c=='1'){
        rxbuf[0]<<=1;
        if(c=='1') rxbuf[0]|=1;
        rxp++;
        if((rxp&0x07) == 0){
            rxbuf[((rxp>>3)&0x07)]=rxbuf[0];
            rxbuf[0]=0;
        }
        if(rxp==48){
            rxp=0;
            rxbuf[0]=0;
            handlerx=1;
            /*
            xcout(rxbuf[1]); //cmd
            xcout(rxbuf[2]); //data[0]
            xcout(rxbuf[3]); //data[1]
            xcout(rxbuf[4]); //data[2]
            xcout(rxbuf[5]); //data[3]
            xcout(rxbuf[6]); //crc
            cout('\r');
            cout('\n');
            */
            


            rxto=0;
        }else
            rxto=2;
    }else if(c=='z'){
            cout('\r');
            cout('\n');
            xcout(dow);
            xcout(th);
            xcout(tm);
            xcout(ts);
            cout('\r');
            cout('\n');
    }else if(c=='a'){
        int i=0;
        cout('\r');
        cout('\n');
        for(;i<16;i++){
            xcout(i);
            cout(' ');
            xcout(alarms[i].dow);
            xcout(alarms[i].h);
            xcout(alarms[i].m);
            cout(' ');
            xcout(alarms[i].pattern);
            cout('\r');
            cout('\n');
        }
    }else{
        cout(c);
    }
}


void TIM1OVF(void) __interrupt(11) { //overflow
    if(TIM1_GetITStatus(TIM1_IT_UPDATE)==SET){
        TIM1_ClearITPendingBit(TIM1_IT_UPDATE);
    }
    GPIO_WriteLow(GPIOD, GPIO_PIN_3);
    run_ring();
}

void TIM2OVF(void) __interrupt(13) { //overflow
    if(TIM2_GetITStatus(TIM2_IT_UPDATE)==SET){
        TIM2_ClearITPendingBit(TIM2_IT_UPDATE);
    }
    ts++;
    if(ts>59){
        int i=0;
        tm++;
        ts=0;

        cout('\r');
        cout('\n');
        for(;i<16;i++){
            xcout(i);
            cout(' ');
            xcout(alarms[i].dow);
            xcout(alarms[i].h);
            xcout(alarms[i].m);
            cout(' ');
            xcout(alarms[i].pattern);
            if(alarms[i].h == th && alarms[i].m == tm){
                cout('t');
                if((1<<dow) && alarms[i].dow){
                    cout('+');
                    ring_pattern=rings[alarms[i].pattern];
                    run_ring();
                }
            }
            cout('\r');
            cout('\n');
        }

        if(tm>59){
            th++;
            tm=0;
            if(th>23){ 
                th=0;
                dow++;
                if(dow>=7)
                    dow=1;
            }
            if(!ring_pattern){
                ring_pattern=rings[1];
                run_ring();
            }
        }else{
            if(!ring_pattern){
                ring_pattern=rings[0];
                run_ring();
            }
        }

    }
    if(rxto){
        rxto--;
        if(!rxto){
                rxbuf[0]=0;
                rxp=0;
        }
    }
    GPIO_WriteReverse(GPIOD, GPIO_PIN_2);

    GPIO_WriteHigh(GPIOD, GPIO_PIN_3);
    cout('.');

}

int main(void) {
    CLK_HSECmd(ENABLE);
//    CLK_SYSCLKConfig(CLK_PRESCALER_HSEDIV1);
    //CLK_LSICmd(ENABLE);
    while (CLK_GetFlagStatus(CLK_FLAG_HSIRDY)==0);
    CLK_SYSCLKConfig(CLK_PRESCALER_CPUDIV1);
//    CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV4);
    CLK_ClockSwitchCmd(ENABLE);

    CLK_CCOConfig(CLK_OUTPUT_MASTER);
    CLK_CCOCmd(ENABLE);


    CLK_ClockSwitchConfig(CLK_SWITCHMODE_MANUAL, CLK_SOURCE_HSE, DISABLE, CLK_CURRENTCLOCKSTATE_DISABLE);
//    CLK_ClockSecuritySystemEnable ();
    CLK_HSICmd(DISABLE);

    //CLK_SYSCLKConfig(CLK_Prescaler_TypeDef CLK_Prescaler);


#ifdef DEBUG
    UART1_DeInit();
    //UART1_Init(208, 5, //1200
    UART1_Init(26, 0,  //9600
            UART1_WORDLENGTH_8D, UART1_STOPBITS_1, UART1_PARITY_NO,  
		UART1_SYNCMODE_CLOCK_DISABLE,     UART1_MODE_TXRX_ENABLE);
    UART1_Cmd(ENABLE);
    UART1_ITConfig(UART1_IT_RXNE_OR,ENABLE);
    UART_Send("Preved\r\n");
#endif

    GPIO_Init(GPIOD, GPIO_PIN_4, GPIO_MODE_OUT_PP_LOW_FAST); //Vibrator
    GPIO_Init(GPIOD, GPIO_PIN_3, GPIO_MODE_OUT_PP_HIGH_FAST); //Ind catode
    GPIO_Init(GPIOD, GPIO_PIN_2, GPIO_MODE_OUT_PP_HIGH_FAST); //Ind catode
/*
    GPIO_Init(GPIOA, GPIO_PIN_3, GPIO_MODE_OUT_PP_LOW_FAST); //Motor
    GPIO_WriteHigh(GPIOA, GPIO_PIN_3);
    GPIO_Init(GPIOB, GPIO_PIN_5, GPIO_MODE_OUT_PP_LOW_FAST); //LED
    GPIO_WriteHigh(GPIOB, GPIO_PIN_5);


    GPIO_Init(GPIOD, GPIO_PIN_4, GPIO_MODE_IN_PU_NO_IT); //DUMP Button
*/
    CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER1, ENABLE);
    CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER2, ENABLE);
    CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER4, DISABLE);
    CLK_PeripheralClockConfig(CLK_PERIPHERAL_I2C, DISABLE);
    CLK_PeripheralClockConfig(CLK_PERIPHERAL_SPI, DISABLE);
    CLK_PeripheralClockConfig(CLK_PERIPHERAL_ADC, DISABLE);

    TIM2_TimeBaseInit(TIM2_PRESCALER_256, 15625);
    TIM2_Cmd(ENABLE);
    TIM2_ITConfig(TIM2_IT_UPDATE, ENABLE);

    TIM1_TimeBaseInit(25000, TIM1_COUNTERMODE_DOWN, 256, 0);
    TIM1_ITConfig(TIM1_IT_UPDATE, ENABLE);
    
    enableInterrupts();
    ring_pattern=rings[1];
    run_ring();

    while(1){
        if(handlerx){
            if(!((rxbuf[1]+rxbuf[2]+rxbuf[3]+rxbuf[4]+rxbuf[5]+rxbuf[6])&0xff)){
                if(rxbuf[1]==0x81){ //set time
                    if(rxbuf[2]>0 && rxbuf[2]<8 && rxbuf[3]<24 && rxbuf[4]<60 && rxbuf[5]<60){
                        dow=rxbuf[2]; 
                        th=rxbuf[3]; 
                        tm=rxbuf[4]; 
                        ts=rxbuf[5]; 
                        cout('S');
                    }
                }else if(rxbuf[1]==0x82){ //test ring
                    ring_pattern=rings[rxbuf[5]];
                    run_ring();
                    cout('R');
                }else if(rxbuf[1]==0x83){ //set alarm
                    uint8_t alm=rxbuf[2]>>4;
                    alarms[alm].pattern=rxbuf[2]&0x0f;
                    alarms[alm].dow=rxbuf[3];
                    alarms[alm].h=rxbuf[4];
                    alarms[alm].m=rxbuf[5];
                    cout('A');
                }else if((rxbuf[1] & 0b11100000) == 0x20){ //set ring pattern
                    uint8_t pn=rxbuf[1]&7;
                    uint8_t nibble=rxbuf[1]&0x10 ? 4 : 0;
                    //xcout(pn);
                    //xcout(nibble);
                    rings[pn][0+nibble]=rxbuf[2];
                    rings[pn][1+nibble]=rxbuf[3];
                    rings[pn][2+nibble]=rxbuf[4];
                    rings[pn][3+nibble]=rxbuf[5];
                    cout('P');
                }
            }
            handlerx=0;
        }

        /*
        if(eesave==1){
            UART_Send("EES ");
            FLASH_Unlock(FLASH_MEMTYPE_DATA);
            //FLASH_ProgramWord(0x4010, w);
            if(FLASH_ReadByte(0x4001)!=cmode)
                FLASH_ProgramByte(0x4001, cmode);
            if(FLASH_ReadByte(0x4000)!=0x33)
                FLASH_ProgramByte(0x4000, 0x33);
            FLASH_Lock(FLASH_MEMTYPE_DATA);
            UART_Send("D\r\n");
            eesave=0;
        }*/
        __asm__("wfi");
    }
}
