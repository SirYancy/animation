//
// Created by eric on 11/29/18.
//

#include <iostream>
#include "colladaloader.h"

char data_types[4][15] = {"float_array", "int_array", "bool_array", "Name_array"};

ColladaLoader::ColladaLoader(std::string filename) : filename(filename) {}

void ColladaLoader::ReadGeometries(std::vector<Geometry> *geometries) {
    XMLDocument doc;
    doc.LoadFile(filename.c_str());
    XMLElement *root = doc.RootElement();

    XMLElement *mesh, *triangles, *input, *source, *primitive;
    std::string sourceName;
    int primitiveCount, indicesCount;

    XMLElement *geometry = root->FirstChildElement("library_geometries")->FirstChildElement("geometry");

    while (geometry != nullptr) {
        Geometry data;
        data.name = geometry->Attribute("id");
        mesh = geometry->FirstChildElement("mesh");
        while (mesh != nullptr) {
            triangles = mesh->FirstChildElement("triangles");
            if (triangles == nullptr) {
                std::cerr << "We do not support the primitives in " << filename << std::endl;
                exit(1);
            }
            input = triangles->FirstChildElement("input");

            while (input != nullptr) {
                if (!strcmp(input->Attribute("semantic"), "VERTEX")) {
                    sourceName = std::string(
                            mesh->FirstChildElement("vertices")->FirstChildElement("input")->Attribute("source"));
                } else {
                    sourceName = std::string(input->Attribute("source"));
                }

                std::cout << "" << input->Attribute("semantic") << std::endl;

                sourceName = sourceName.erase(0, 1);
                source = mesh->FirstChildElement("source");

                while (source != nullptr) {
                    if (std::string(source->Attribute("id")) == sourceName) {
                        data.map[std::string(input->Attribute("semantic"))] = ReadElement(source);
                    }
                    source = source->NextSiblingElement("source");
                }
                input = input->NextSiblingElement("input");
            }

            primitive = mesh->FirstChildElement("triangles");
            if (primitive == nullptr) {
                std::cerr << "We do not support the primitives in " << filename << std::endl;
                exit(1);
            }
            primitive->QueryAttribute("count", &primitiveCount);
            indicesCount = primitiveCount * 3;

            data.index_count = indicesCount;
            std::cout << "Primitives: " << primitiveCount << std::endl <<
                      "Indices: " << indicesCount << std::endl;
            data.indices = (unsigned short *) malloc(indicesCount * sizeof(unsigned short));
            char *text = (char *) (primitive->FirstChildElement("p")->GetText());
            data.indices[0] = (unsigned short) atoi(strtok(text, " "));
            for (int index = 1; index < indicesCount; index++) {
                data.indices[index] = (unsigned short) atoi(strtok(NULL, " "));
            }
            mesh = mesh->FirstChildElement("mesh");
        }
        geometries->push_back(data);
        geometry = geometry->NextSiblingElement("geometry");
    }
}

SourceData ColladaLoader::ReadElement(XMLElement *element) {
    SourceData sourceData;
    XMLElement *array;
    char *text;
    unsigned int numVals, stride;
    int check;

    for (int i = 0; i < 4; i++) {
        array = element->FirstChildElement(data_types[i]);
        if (array != nullptr) {
            array->QueryUnsignedAttribute("count", &numVals);
            std::cout << "Values: " << numVals << std::endl;
            sourceData.size = numVals;

            check = element->FirstChildElement("technique_common")->FirstChildElement("accessor")
                    ->QueryUnsignedAttribute("stride", &stride);
            if (check != XML_NO_ATTRIBUTE)
                sourceData.stride = stride;
            else
                sourceData.stride = 1;

            text = (char *) (array->GetText());

            switch (i) {
                case 0:
                    sourceData.type = GL_FLOAT;
                    sourceData.size *= sizeof(float);
                    sourceData.data = malloc(numVals * sizeof(float));

                    ((float *) sourceData.data)[0] = atof(strtok(text, " "));
                    for (unsigned int index = 1; index < numVals; index++) {
                        ((float *) sourceData.data)[index] = atof(strtok(NULL, " "));
                    }
                    break;
                case 1:
                    sourceData.type = GL_INT;
                    sourceData.size *= sizeof(int);
                    sourceData.data = malloc(numVals * sizeof(int));

                    ((int *) sourceData.data)[0] = atoi(strtok(text, " "));
                    for (unsigned int index = 1; index < numVals; index++) {
                        ((int *) sourceData.data)[index] = atoi(strtok(NULL, " "));
                    }
                    break;
                default:
                    std::cout << "We do not support this type of mesh data " << std::endl;
                    break;
            }
        }
    }
    return sourceData;
}

void ColladaLoader::FreeGeometries(std::vector<Geometry> *geometries) {
    std::vector<Geometry>::iterator giterator;
    SourceMap::iterator miterator;

    for (giterator = geometries->begin(); giterator < geometries->end(); giterator++) {
        free(giterator->indices);
        for (miterator = giterator->map.begin(); miterator != giterator->map.end(); miterator++) {
            free((*miterator).second.data);
        }
        geometries->erase(giterator);
    }
}
