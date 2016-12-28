attribute vec3 aVertexPosition;
attribute vec3 aVertexNormal;
attribute vec2 aTextureCoord;
attribute vec3 aVertexFrontColor;

uniform mat4 uMVMatrix;
uniform mat4 uPMatrix;
uniform mat3 uNMatrix;

varying vec3 vLightDirection[3];
varying vec3 vNormalDirection;
varying vec3 vVertexPosition;
varying vec3 vVertexFrontColor;

varying vec2 vTextureCoord;
uniform vec3 uPointLightingLocation[3];

void main(void) {
    gl_Position = uPMatrix * uMVMatrix * vec4(aVertexPosition, 1.0);

    vNormalDirection = normalize(uNMatrix * aVertexNormal);
    vVertexPosition = (uMVMatrix * vec4(aVertexPosition, 1.0)).xyz;
    for (int i = 0; i < 3; i++) {
		vLightDirection[i] = normalize(uPointLightingLocation[i] - (uMVMatrix * vec4((aVertexPosition), 1.0)).xyz);
	}
	vTextureCoord = aTextureCoord;
	vVertexFrontColor = aVertexFrontColor;
}