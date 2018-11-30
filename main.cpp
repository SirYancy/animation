#include <iostream>
#include "collada/colladaloader.h"

int main() {
    std::cout << "Hello, World!" << std::endl;

    ColladaLoader *loader = new ColladaLoader("../models/cowboy.dae");
    auto *geometries = new std::vector<Geometry>();
    loader->ReadGeometries(geometries);
    loader->FreeGeometries(geometries);
    return 0;
}