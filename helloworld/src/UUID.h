#pragma once
#include <cstdint> // Necesario para usar uint64_t

// Definimos VroomUUID como un entero de 64 bits sin signo (unsigned long long)
typedef std::uint64_t VroomUUID;

class UUIDGen {
public:
	UUIDGen();
	~UUIDGen();

	// Método estático para poder llamarlo sin instanciar la clase: 
	// VroomUUID id = UUIDGen::GenerateUUID();
	static VroomUUID GenerateUUID();
};