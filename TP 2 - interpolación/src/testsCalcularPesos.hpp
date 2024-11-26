#ifndef TESTSCALCULARPESOS_HPP
#define TESTSCALCULARPESOS_HPP

#include <iostream>
#include "utils.hpp"

#define my_assert(cal,ret) if(!verificar(cal,ret)) {\
	std::cerr << "Falla el caso de prueba:\n   " << #cal << "\n   " << #ret << "\n   "\
	          << "peroRetorna (" << cal[0] << ", " << cal[1] << ", " << cal[2] << ")\n";\
	std::exit(1);\
}

struct PruebaCalcularPesos {
	bool verificar(Pesos calculados, Pesos correctos) {
		for(int k=0;k<3;++k)
			if (std::fabs(calculados[k]-correctos[k])>1e-5) return false;
		return true;
	}
	Pesos debeRetornar(float p1, float p2, float p3) {
		return Pesos{p1,p2,p3};
	}
	PruebaCalcularPesos() {
		constexpr double un_tercio = 1.0/3.0;
		glm::vec3 x0 = { -1.f, -1.f, 0.f };
		glm::vec3 x1 = { +1.f, -1.f, 0.f };
		glm::vec3 x2 = {  0.f, +1.f, 0.f };
		glm::vec3 x_fuera = {  +2.f, +1.f, 0.f };
		my_assert( calcularPesos (x0, x1, x2, x0),             debeRetornar (1.0, 0.0, 0.0));
		my_assert( calcularPesos (x0, x1, x2, x1),             debeRetornar (0.0, 1.0, 0.0));
		my_assert( calcularPesos (x0, x1, x2, x2),             debeRetornar (0.0, 0.0, 1.0));
		my_assert( calcularPesos (x0, x1, x2, (x0+x1)/2.f),    debeRetornar (0.5, 0.5, 0.0));
		my_assert( calcularPesos (x0, x1, x2, (x0+x1+x2)/3.f), debeRetornar (un_tercio, un_tercio, un_tercio));
		my_assert( calcularPesos (x0, x1, x2, x_fuera),        debeRetornar (-1.0,1.0,1.0));
	}
};

#endif
