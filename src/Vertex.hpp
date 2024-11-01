#pragma once

struct Vertex {
    float position[3];
    float textureCoords[2];
    float textureIndex; // i know this is stupid, but opengl does not likes anythyng else
};