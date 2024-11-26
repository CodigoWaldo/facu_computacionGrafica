#ifndef BILLBOARD_H
#define BILLBOARD_H

#include "Model.hpp"

#include <memory>

class Model;

// Singletone instance representing a billboard.
// Simple quad on XY plane, with normals and texture coords.
struct Billboard {

public:
	static std::unique_ptr<Model>& getBillboard();
	
private:
	static std::unique_ptr<Model> m_billboard;
};


#endif
