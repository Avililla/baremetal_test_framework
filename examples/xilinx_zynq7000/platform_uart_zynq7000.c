#include "bmt_platform_io.h"
#include "xuartps.h"
#include "xscutimer.h"
#include "xparameters.h"


#define TIMER_DEVICE_ID     XPAR_SCUTIMER_DEVICE_ID

static XScuTimer TimerInstance;


void bmt_platform_io_init(void) {

    XScuTimer_Config *TimerConfig = XScuTimer_LookupConfig(TIMER_DEVICE_ID);
    int Status = XScuTimer_CfgInitialize(&TimerInstance, TimerConfig, TimerConfig->BaseAddr);
    if (Status != XST_SUCCESS) {
        return;
    }
    XScuTimer_SetPrescaler(&TimerInstance, 0);
    XScuTimer_LoadTimer(&TimerInstance, 0xFFFFFFFF);
    XScuTimer_Start(&TimerInstance);
}

void bmt_platform_putchar(char c) {
    putchar(c);
}

void bmt_platform_puts(const char *str) {
	printf("%s", str);
}

uint32_t bmt_platform_get_msec_ticks(void) {
    uint32_t raw_ticks = XScuTimer_GetCounterValue(&TimerInstance);
    return 0xFFFFFFFF - raw_ticks;
}