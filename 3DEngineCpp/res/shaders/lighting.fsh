varying vec2 texCoord0;
varying vec3 worldPos0;
varying mat3 axesInfoMatrix;

uniform sampler2D diffuse;
uniform sampler2D normalMap;


#include "lighting.glh"
