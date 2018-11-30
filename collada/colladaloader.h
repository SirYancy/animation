//
// Created by eric on 11/29/18.
//

#ifndef MAZE_COLLADALOADER_H
#define MAZE_COLLADALOADER_H

#include <string>
#include <vector>
#include <glad.h>
#include <map>
#include "../tinyxml/tinyxml2.h"

using namespace tinyxml2;

struct SourceData {
    GLenum type;
    unsigned int size;
    unsigned int stride;
    void *data;
};

typedef std::map<std::string, SourceData> SourceMap;

struct Geometry{
    std::string name;
    SourceMap map;
    int index_count;
    unsigned short *indices;
};

class ColladaLoader {

private:
    std::string filename;

    SourceData ReadElement(XMLElement *);


public:
    explicit ColladaLoader(std::string filename);

    void ReadGeometries(std::vector<Geometry>*);

    void FreeGeometries(std::vector<Geometry>*);
};

#endif //MAZE_COLLADALOADER_H
