#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

typedef struct {
	float* data; // указатель на массив данных аудиосигнала
	size_t size; // размер буфера (количество сэмплов)
	int sampleRate; // частота дискретизации
} AudioBuffer;

typedef struct {
	float mix; // коэффициент смешивания сухого и обработанного сигнала
	void (*process)(AudioBuffer* input, AudioBuffer* output); // функция для обработки входного аудиобуфера
} AudioEffect;

typedef struct {
	AudioBuffer effect; // наследуем поля от AudioEffect
	float roomSize; // размер комнаты для реверберации
	float dampening; // уровень демпфирования
} Reverb;

typedef struct {
	AudioBuffer effect; // наследуем поля от AudioEffect
	float delayTime; // время задержки в миллисекундах
	float feedback; // уровень обратной связи
} Delay;

typedef struct {
	AudioBuffer buffer; // аудио буфер с входными данными
	char* source; // источник аудиосигнала (например, файл)
} AudioInput;

typedef struct {
	AudioBuffer buffer; // аудио буфер с входными данными
	char* destination; // место сохранения или проигрывания аудиосигнала
} AudioOutput;

typedef struct {
	float gain; // усиление сигнала
	int bypass; // флаг включения/выключения эффекта
} PluginSettings;

typedef struct {
	AudioInput input; // аудиовход
	AudioOutput output; // аудиовыход
	AudioEffect** effects; // массив указателей на эффекты
	size_t numEffects; // количество эффектов
	PluginSettings settings; // настройки плагина
} AudioPlugin;
int main() {

	setlocale(LC_ALL, "ru");

}