/*
Copyright (c) 2025 Nikita Martynau (MIT license: https://opensource.org/license/mit)

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

// Thanks to Nikita Martynau da goat
#version 330

in vec3 Position;
in vec3 Normal;
in vec4 Color;

uniform mat4 ViewMatrix;
uniform mat4 ModelMat;
uniform mat4 ProjMat;
uniform vec3 SS_CameraPosition;
uniform vec3 LightPosition;

out vec3 camPosLocalSpace;
out vec3 sunPosLocalSpace;
out vec3 fragPosLocalSpace;

void main() 
{
    gl_Position = ProjMat * ViewMatrix * ModelMat * vec4(Position, 1.0);
    mat4 invModelMat = inverse(ModelMat);
    camPosLocalSpace = vec3(invModelMat * vec4(SS_CameraPosition, 1));
    sunPosLocalSpace = vec3(invModelMat * vec4(LightPosition, 1));
    fragPosLocalSpace = vec3(Position);
}