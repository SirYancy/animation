//
// Created by eric on 11/29/18.
//

#include <iostream>
#include "colladaloader.h"

ColladaLoader::ColladaLoader(std::string filename) : filename(filename) {}

void ColladaLoader::ReadGeometry(Geometry *data) {
    XMLDocument doc;
    doc.LoadFile(filename.c_str());
    XMLElement *root = doc.RootElement();

    XMLElement *mesh, *vertices, *triangles, *input, *source;
    std::string sourceName;
    int primitiveCount, indicesCount;

    XMLElement *geometry = root->FirstChildElement("library_geometries")->FirstChildElement("geometry");

    data->name = geometry->Attribute("id");
    // Get Mesh Element
    mesh = geometry->FirstChildElement("mesh");

    // Get Triangles and Vertices Elements
    triangles = mesh->FirstChildElement("triangles");
    vertices = mesh->FirstChildElement("vertices");

    if (triangles == nullptr) {
        std::cerr << "We do not support the primitives in " << filename << std::endl;
        exit(1);
    }

    // First process Position Data
    input = GetInput("VERTEX", vertices);

    sourceName = std::string(input->Attribute("source"));
    sourceName = sourceName.erase(0,1);

    std::cout << input->Attribute("semantic") << " " << sourceName << std::endl;

    source = GetSource(sourceName, mesh);
    data->position = ReadElement(source);
    input->QueryAttribute("offset", &data->position.offset);

    // Process Normal Data
    input = GetInput("NORMAL", triangles);
    sourceName = std::string(input->Attribute("source"));
    sourceName = sourceName.erase(0, 1);

    std::cout << input->Attribute("semantic") << " " << sourceName << std::endl;

    source = GetSource(sourceName, mesh);
    data->normals = ReadElement(source);
    input->QueryAttribute("offset", &data->normals.offset);

    // TODO Process Texture Data

    triangles->QueryAttribute("count", &primitiveCount);

    indicesCount = primitiveCount * (data->normals.stride + data->position.stride); // TODO + texture stride

    auto *indices = (unsigned short*)malloc(indicesCount * sizeof(unsigned short));

    std::cout << "Primitives: " << primitiveCount << std::endl <<
              "Indices: " << indicesCount << std::endl;

    char *text = (char *) (triangles->FirstChildElement("p")->GetText());

    indices[0] = (unsigned short) atoi(strtok(text, " "));
    for (int index = 1; index < indicesCount; index++) {
        indices[index] = (unsigned short) atoi(strtok(NULL, " "));
    }

    BuildBuffer(indices, indicesCount, primitiveCount, data);

    printf("Geometry Loaded\n");
    free(indices);
}

SourceData ColladaLoader::ReadElement(XMLElement *element) {
    SourceData sourceData;
    XMLElement *array;
    char *text;
    unsigned int numVals, stride;
    int check;

    array = element->FirstChildElement("float_array");
    if (array != nullptr) {
        array->QueryUnsignedAttribute("count", &numVals);
        std::cout << "Values: " << numVals << std::endl;

        check = element->FirstChildElement("technique_common")->FirstChildElement("accessor")
                ->QueryUnsignedAttribute("stride", &stride);
        if (check != XML_NO_ATTRIBUTE)
            sourceData.stride = stride;
        else
            sourceData.stride = 1;

        text = (char *) (array->GetText());

        sourceData.type = GL_FLOAT;
        sourceData.size = numVals * sizeof(float);
        sourceData.data = malloc(numVals * sizeof(float));

        ((float *) sourceData.data)[0] = atof(strtok(text, " "));
        for (unsigned int index = 1; index < numVals; index++) {
            ((float *) sourceData.data)[index] = atof(strtok(NULL, " "));
        }
    }
    return sourceData;
}

void ColladaLoader::FreeGeometry(Geometry *g) {
    free(g->normals.data);
    free(g->position.data);
    free(g->texCoords.data);
    free(g->data);
    free(g);
}

XMLElement *ColladaLoader::GetSource(std::string sourceName, XMLElement *mesh) {
    XMLElement *source;
    source = mesh->FirstChildElement("source");
    while(source != nullptr) {
        if (std::string(source->Attribute("id")) == sourceName) {
            return source;
        }
        source = source->NextSiblingElement("source");
    }
}

XMLElement *ColladaLoader::GetInput(std::string semantic, XMLElement *element) {
    return nullptr;
}

void ColladaLoader::BuildBuffer(unsigned short *indices, int indicesCount, int primitiveCount, Geometry *g) {
    unsigned int normalOffset, positionOffset, texOffset; //TODO Do Textures

    positionOffset = g->position.offset;
    normalOffset = g->normals.offset;
    // TODO texOffset = g->texCoords.offset;

    g->data = (float *) malloc(indicesCount * sizeof(float));

    for(int i = 0; i < primitiveCount; i++){
        int index = i * (positionOffset + normalOffset); //TODO TEXTURE
        ((float*) g->data)[index] = ((float *)g->position.data)[indices[index+positionOffset]];
        ((float*) g->data)[index+1] = ((float *)g->position.data)[indices[index+positionOffset] + 1];
        ((float*) g->data)[index+2] = ((float *)g->position.data)[indices[index+positionOffset] + 2];
        ((float*) g->data)[index+3] = ((float *)g->normals.data)[indices[index+normalOffset]];
        ((float*) g->data)[index+4] = ((float *)g->normals.data)[indices[index+normalOffset] + 1];
        ((float*) g->data)[index+5] = ((float *)g->normals.data)[indices[index+normalOffset] + 2];
    }
}

