#include "UUID.h"
#include <random> // Librería estándar, ¡funciona sin instalar nada!

UUIDGen::UUIDGen() {
}

UUIDGen::~UUIDGen() {
}

VroomUUID UUIDGen::GenerateUUID() {
	// 1. Obtenemos una semilla aleatoria del hardware del ordenador
	static std::random_device rd;

	// 2. Inicializamos el generador (Mersenne Twister 64-bit) con esa semilla
	//    'static' hace que el generador se cree una sola vez y se reutilice (más rápido)
	static std::mt19937_64 generator(rd());

	// 3. Definimos una distribución uniforme que cubra todo el rango posible de números
	static std::uniform_int_distribution<VroomUUID> distribution;

	// 4. Generamos y devolvemos el número
	return distribution(generator);
}