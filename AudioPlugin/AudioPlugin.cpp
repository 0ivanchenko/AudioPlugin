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
	AudioEffect effect; // наследуем поля от AudioEffect
	float roomSize; // размер комнаты для реверберации
	float dampening; // уровень демпфирования
} Reverb;

typedef struct {
	AudioEffect effect; // наследуем поля от AudioEffect
	float delayTime; // время задержки в миллисекундах
	float feedback; // уровень обратной связи
} Delay;

typedef struct {
	AudioBuffer buffer; // аудио буффер с входными данными
	char* source; // источник аудиосигнала (например, файл)
} AudioInput;

typedef struct {
	AudioBuffer buffer; // аудио буффер с входными данными
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

// Инициализация аудио буффера
void initAudioBuffer(AudioBuffer* buffer, size_t size, int sampleRate) {
	buffer->data = (float*)malloc(size * sizeof(float));
	buffer->size = size;
	buffer->sampleRate = sampleRate;
}
// Освобождение памяти аудио буффера
void freeAudioBuffer(AudioBuffer* buffer) {
	free(buffer->data);
}
// Применение эффекта к аудиоданным
void applyEffect(AudioPlugin* plugin) {
	for (size_t i = 0; i < plugin->numEffects; i++) {
		plugin->effects[i]->process(&plugin->input.buffer, &plugin->output.buffer);
	}
}
// Реализация функции обработки реверберации
void processReverb(AudioBuffer* input, AudioBuffer* output) {

	// Параметры реверберации
	const size_t bufferSize = input->size;
	const float roomSize = 0.8f;
	const float dampening = 0.5f;

	// Временные буферы для реверберации
	float* delayBuffer1 = (float*)malloc(bufferSize * sizeof(float));
	float* delayBuffer2 = (float*)malloc(bufferSize * sizeof(float));

	// Инициализация буферов
	for (size_t i = 0; i < bufferSize; i++) {
		delayBuffer1[i] = 0.0f;
		delayBuffer2[i] = 0.0f;
	}

	// Основной цикл обработки
	for (size_t i = 0; i < bufferSize; i++) {
		// Вычисление индексов для задержек
		size_t delayIndex1 = (i + (size_t)(roomSize * input->sampleRate)) % bufferSize;
		size_t delayIndex2 = (i + (size_t)((roomSize + 0.1f) * input->sampleRate)) % bufferSize;

		// Вычисление выходного сигнала
		float delayedSample1 = delayBuffer1[delayIndex1];
		float delayedSample2 = delayBuffer2[delayIndex2];

		float outputSample = input->data[i] + dampening * (delayedSample1 + delayedSample2);

		// Обновление буферов задержки
		delayBuffer1[i] = input->data[i] + dampening * delayedSample1;
		delayBuffer2[i] = input->data[i] + dampening * delayedSample2;

		// Запись выходного сигнала
		output->data[i] = outputSample;
	}

	// Освобождение памяти
	free(delayBuffer1);
	free(delayBuffer2);
}

// Инициализация реверберации
void initReverb(Reverb* reverb, float roomSize, float dampening, float mix) {
	reverb->effect.mix = mix;
	reverb->roomSize = roomSize;
	reverb->dampening = dampening;
	reverb->effect.process = processReverb;

}

// Реализация функции обработки задержки
void processDelay(AudioBuffer* input, AudioBuffer* output) {

	// Параметры задержки
	const size_t bufferSize = input->size;
	const float delayTime = 500.0f; // Время задержки в миллисекундах
	const float feedback = 0.4f;

	// Временный буфер для задержки
	float* delayBuffer = (float*)malloc(bufferSize * sizeof(float));

	// Инициализация буфера
	for (size_t i = 0; i < bufferSize; i++) {
		delayBuffer[i] = 0.0f;
	}

	// Основной цикл обработки
	for (size_t i = 0; i < bufferSize; i++) {
		// Вычисление индекса для задержки
		size_t delayIndex = (i + (size_t)(delayTime * input->sampleRate / 1000.0f)) % bufferSize;

		// Вычисление выходного сигнала
		float delayedSample = delayBuffer[delayIndex];
		float outputSample = input->data[i] + feedback * delayedSample;

		// Обновление буфера задержки
		delayBuffer[i] = input->data[i] + feedback * delayedSample;

		// Запись выходного сигнала
		output->data[i] = outputSample;
	}

	// Освобождение памяти
	free(delayBuffer);
}
// Инициализация задержки
void initDelay(Delay* delay, float delayTime, float feedback, float mix) {
	delay->effect.mix = mix;
	delay->delayTime = delayTime;
	delay->feedback = feedback;
	delay->effect.process = processDelay;
}

int main() {

	setlocale(LC_ALL, "ru");

	// Инициализация аудиоплагина
	AudioPlugin plugin;
	initAudioBuffer(&plugin.input.buffer, 1024, 44100);
	initAudioBuffer(&plugin.output.buffer, 1024, 44100);

	// Инициализация реверберации и задержки
	Reverb reverb;
	initReverb(&reverb, 0.8f, 0.5f, 0.7f);

	Delay delay;
	initDelay(&delay, 500.0f, 0.4f, 0.6f);

	// Добавление эффектов в плагин
	AudioEffect* effects[] = { (AudioEffect*)&reverb, (AudioEffect*)&delay };
	plugin.effects = effects;
	plugin.numEffects = 2;

	// Применение эффектов
	applyEffect(&plugin);

	// Освобождение памяти
	freeAudioBuffer(&plugin.input.buffer);
	freeAudioBuffer(&plugin.output.buffer);
}