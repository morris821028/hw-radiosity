precision mediump float;

attribute vec3 aVertexPosition;
attribute vec3 aVertexNormal;
attribute vec2 aTextureCoord;
attribute vec3 aVertexFrontColor;

uniform mat4 uMVMatrix;
uniform mat4 uPMatrix;
uniform mat3 uNMatrix;

varying vec4 vFragcolor;
varying vec3 vNormalDirection;
varying vec3 vVertexPosition;
varying vec3 vLightDirection[3];

uniform vec3 uPointLightingLocation[3];
uniform int uTriangleColor;
uniform sampler2D uSampler;

void main(void) {
    gl_Position = uPMatrix * uMVMatrix * vec4(aVertexPosition, 1.0);

    vNormalDirection = normalize(uNMatrix * aVertexNormal);
    vVertexPosition = (uMVMatrix * vec4(aVertexPosition, 1.0)).xyz;

    for (int i = 0; i < 3; i++) {
		vLightDirection[i] = normalize(uPointLightingLocation[i]-(uMVMatrix * vec4((aVertexPosition), 1.0)).xyz);
	}

    vec4 fragmentColor;
    if (uTriangleColor == 0) {
    	fragmentColor = texture2D(uSampler, vec2(aTextureCoord.s, aTextureCoord.t));
    } else {
    	fragmentColor = vec4(aVertexFrontColor, 1.0);
    }
    vFragcolor = fragmentColor;
}
