#include <math.h>
#include "ex03/main.h"
#include "gui/bt_blackangle64_16.h"
#include "macros.h"

// demo module declarations

#define NUM_DEMOS 4

static void demoGUI();
static void demoWelcome();
static void demoGraph();
static void demoScribble();

static void touchScreenError();
static void initGUI();

DemoFn demos[] = { demoGUI, demoWelcome, demoGraph, demoScribble };
uint32_t demoID = 0;

static TS_StateTypeDef touchState;
__IO uint32_t isPressed = 0;

static SpriteSheet dialSheet = {
		.pixels = bt_blackangle64_16,
		.spriteWidth = 64,
		.spriteHeight = 64,
		.numSprites = 16
};

static GUIElement *bt;

int main() {
	CPU_CACHE_Enable();
	HAL_Init();
	SystemClock_Config();

	BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);
	BSP_LCD_Init();
	if (BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize()) == TS_OK) {
		BSP_LCD_LayerDefaultInit(LTDC_ACTIVE_LAYER, LCD_FRAME_BUFFER);
		BSP_LCD_SelectLayer(LTDC_ACTIVE_LAYER);

		initGUI();

		while (1) {
			isPressed = 0;
			demos[demoID]();
			demoID = (demoID + 1) % NUM_DEMOS;
			while (!isPressed) {
				BSP_TS_GetState(&touchState);
				if (touchState.touchDetected) {
					do {
						BSP_TS_GetState(&touchState);
						HAL_Delay(10);
					} while (touchState.touchDetected);
					break;
				}
				HAL_Delay(10);
			}
		}
	} else {
		touchScreenError();
	}
	return 0;
}

static void initGUI() {
	bt = guiDialButton(0, "Volume", 10, 10, 0.0f, 0.025f, &dialSheet);
}

static void demoGUI() {
	BSP_LCD_SetFont(&UI_FONT);
	BSP_LCD_SetBackColor(0);
	BSP_LCD_Clear(UI_BG_COLOR);
	BSP_LCD_SetTextColor(UI_TEXT_COLOR);

	// force button redraw
	bt->state |= GUI_DIRTY;

	while (!isPressed) {
		BSP_TS_GetState(&touchState);
		bt->handler(bt, &touchState);
		bt->render(bt);
		HAL_Delay(10);
	}
}

static void demoWelcome() {
	BSP_LCD_SetFont(&UI_FONT);
	BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
	BSP_LCD_Clear(LCD_COLOR_WHITE);
	BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
	BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() / 2 - 8,
			(uint8_t *) "STM32F746G", CENTER_MODE);
	BSP_LCD_SetTextColor(LCD_COLOR_DARKBLUE);
	BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() / 2 + 8,
			(uint8_t *) "Welcome!", CENTER_MODE);
}

static void demoGraph() {
	BSP_LCD_Clear(LCD_COLOR_BLACK);
	uint16_t w = BSP_LCD_GetXSize();
	uint16_t h = BSP_LCD_GetYSize() / 2;
	for (uint16_t x = 0; x < w; x++) {
		float theta = (float) x / w * 4.0f * M_PI;
		float y = sinf(theta);
		y += 0.5f * sinf(2.0f * theta);
		y += 0.3333f * sinf(3.0f * theta);
		y += 0.25f * sinf(4.0f * theta);
		BSP_LCD_DrawPixel(x, (uint16_t) (h + h * y * 0.6f), LCD_COLOR_CYAN);
	}
}

static void demoScribble() {
	static uint32_t cols[] = { LCD_COLOR_RED, LCD_COLOR_GREEN, LCD_COLOR_BLUE,
	LCD_COLOR_YELLOW, LCD_COLOR_MAGENTA };
	uint16_t width = BSP_LCD_GetXSize();
	uint16_t height = BSP_LCD_GetYSize();
	BSP_LCD_Clear(LCD_COLOR_WHITE);
	while (!isPressed) {
		BSP_TS_GetState(&touchState);
		if (touchState.touchDetected) {
			for (uint8_t i = 0; i < MIN(touchState.touchDetected, 5); i++) {
				BSP_LCD_SetTextColor(cols[i]);
				BSP_LCD_FillCircle(CLAMP(touchState.touchX[i], 6, width - 6),
						CLAMP(touchState.touchY[i], 6, height - 6), 5);
			}
		}
		HAL_Delay(10);
	}
}

static void touchScreenError() {
	BSP_LCD_SetFont(&LCD_DEFAULT_FONT);
	BSP_LCD_SetBackColor(LCD_COLOR_RED);
	BSP_LCD_Clear(LCD_COLOR_RED);
	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() / 2 - 24,
			(uint8_t *) "Touchscreen error!", CENTER_MODE);
	while (1) {
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (KEY_BUTTON_PIN == GPIO_Pin) {
		while (BSP_PB_GetState(BUTTON_KEY) != RESET) {
		}
		isPressed = 1;
	}
}
