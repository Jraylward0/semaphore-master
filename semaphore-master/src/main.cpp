#include <mbed.h>
#include <ucos_ii.h>
#include <display.h>


/*
*********************************************************************************************************
*                                            APPLICATION TASK AND MUTEX PRIORITIES
*********************************************************************************************************
*/

typedef enum {
  APP_TASK_LED1_PRIO = 4,
  APP_TASK_LED2_PRIO,
	APP_TASK_COUNT1_PRIO,
	APP_TASK_COUNT2_PRIO
} taskPriorities_t;

/*
*********************************************************************************************************
*                                            APPLICATION TASK STACKS
*********************************************************************************************************
*/

#define  APP_TASK_LED1_STK_SIZE              256
#define  APP_TASK_LED2_STK_SIZE              256
#define  APP_TASK_COUNT1_STK_SIZE            256
#define  APP_TASK_COUNT2_STK_SIZE            256

static OS_STK appTaskLED1Stk[APP_TASK_LED1_STK_SIZE];
static OS_STK appTaskLED2Stk[APP_TASK_LED2_STK_SIZE];
static OS_STK appTaskCOUNT1Stk[APP_TASK_COUNT1_STK_SIZE];
static OS_STK appTaskCOUNT2Stk[APP_TASK_COUNT2_STK_SIZE];

/*
*********************************************************************************************************
*                                            APPLICATION FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static void appTaskLED1(void *pdata);
static void appTaskLED2(void *pdata);
static void appTaskCOUNT1(void *pdata);
static void appTaskCOUNT2(void *pdata);

static void display(uint8_t id, uint32_t value);
static void progress(uint8_t id, uint32_t value);
/*
*********************************************************************************************************
*                                            GLOBAL VARIABLES
*********************************************************************************************************
*/
static Display* d = Display::theDisplay();
static bool flashing = false;
static uint32_t total = 0;
static uint32_t count1 = 0;
static uint32_t count2 = 0;

static OS_EVENT *lcdSem;

/*
*********************************************************************************************************
*                                            GLOBAL FUNCTION DEFINITIONS
*********************************************************************************************************
*/

int main() {

  /* Initialise the display */	
	d->fillScreen(WHITE);
	d->setTextColor(BLACK, WHITE);
	
  /* Initialise the OS */
  OSInit();                                                   

  /* Create the tasks */
  OSTaskCreate(appTaskLED1,                               
               (void *)0,
               (OS_STK *)&appTaskLED1Stk[APP_TASK_LED1_STK_SIZE - 1],
               APP_TASK_LED1_PRIO);
  
  OSTaskCreate(appTaskLED2,                               
               (void *)0,
               (OS_STK *)&appTaskLED2Stk[APP_TASK_LED2_STK_SIZE - 1],
               APP_TASK_LED2_PRIO);

  OSTaskCreate(appTaskCOUNT1,                               
               (void *)0,
               (OS_STK *)&appTaskCOUNT1Stk[APP_TASK_COUNT1_STK_SIZE - 1],
               APP_TASK_COUNT1_PRIO);

  OSTaskCreate(appTaskCOUNT2,                               
               (void *)0,
               (OS_STK *)&appTaskCOUNT2Stk[APP_TASK_COUNT2_STK_SIZE - 1],
               APP_TASK_COUNT2_PRIO);
							 
	lcdSem = OSSemCreate(1);

							 
  /* Start the OS */
  OSStart();                                                  
  
  /* Should never arrive here */ 
  return 0;      
}

/*
*********************************************************************************************************
*                                            APPLICATION TASK DEFINITIONS
*********************************************************************************************************
*/

static void appTaskLED1(void *pdata) {
  DigitalOut led1(LED1);
	
  /* Start the OS ticker -- must be done in the highest priority task */
  SysTick_Config(SystemCoreClock / OS_TICKS_PER_SEC);
  
  /* Task main loop */
  while (true) {
		if (flashing) {
      led1 = !led1;
		}
		OSTimeDlyHMSM(0,0,0,500);
  }
}


static void appTaskLED2(void *pdata) {
  DigitalOut led2(LED2);
	
  while (true) {
    OSTimeDlyHMSM(0,0,0,500);
		if (flashing) {
      led2 = !led2;
		}
  } 
}


static void appTaskCOUNT1(void *pdata) {  
	uint8_t status;
	
  while (true) {
		OSSemPend(lcdSem, 0, &status);
    count1 += 1;
    display(1, count1);
    total += 1;
		status = OSSemPost(lcdSem);
    if ((count1 + count2) != total) {
      flashing = true;
    }
		OSTimeDlyHMSM(0,0,0,2);
  } 
}


static void appTaskCOUNT2(void *pdata) {
	uint8_t status;
	
  while (true) {
		OSSemPend(lcdSem, 0, &status);
    count2 += 1;
    display(2, count2);
    total += 1;
		status = OSSemPost(lcdSem);		
    if ((count1 + count2) != total) {
      flashing = true;
    }
		OSTimeDlyHMSM(0,0,0,2);
  } 
}


static void display(uint8_t id, uint32_t value) {
    d->setCursor(2, id * d->getStringHeight("X"));
    d->printf("count%1d: %09d", id, value);
	  progress(id, value);
}

static void progress(uint8_t id, uint32_t value) {
	uint16_t x;
	uint16_t y;
	uint16_t const left = 200;
  uint16_t const right = 300;
  uint16_t ppos;	
	uint16_t height;
	uint16_t top;
	uint16_t bottom;
	uint16_t colour;
	
	ppos = left + ((value % 1000) / 10);	
	height = d->getStringHeight("X");
	top = id * height;
	bottom = (id + 1) * height - 2;
	if (id == 1) {
		colour = RED;
	}
	else {
		colour = GREEN;
	}
	for (x = left; x < ppos; x++) {
		for (y = top; y < bottom; y++) {
			d->drawPixel(x, y, colour);
		}
	}
	for (x = ppos; x < right; x++) {
		for (y = top; y < bottom; y++) {
			d->drawPixel(x, y, WHITE);
		}
	}
}

