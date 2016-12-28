#extension GL_OES_standard_derivatives : enable

precision mediump float;

varying vec4 vFragcolor;
varying vec3 vLightDirection[3];
varying vec3 vNormalDirection;
varying vec3 vVertexPosition;

uniform float uMaterialShininess;

uniform vec3 uAmbientColor;
uniform vec3 uPointLightingSpecularColor;
uniform vec3 uPointLightingDiffuseColor;
uniform float uPointEnabled[3];


void main(void) {
	vec3 dx = dFdx(vVertexPosition);
	vec3 dy = dFdy(vVertexPosition);
	vec3 tn = normalize(cross(dx, dy));

	float specularLightWeighting = 0.0;
    
    vec3 eyeDirection = normalize(-vVertexPosition.xyz);
    vec3 lightWeighting = vec3(0.0, 0.0, 0.0);

    for (int i = 0; i < 3; i++) {
        if (uPointEnabled[i] == 0.0)
            continue;
        vec3 reflectionDirection = reflect(-vLightDirection[i], tn);

        specularLightWeighting = pow(max(dot(reflectionDirection, eyeDirection), 0.0), uMaterialShininess);
    
        float diffuseLightWeighting = max(dot(tn, vLightDirection[i]), 0.0);
        lightWeighting += uAmbientColor
        + uPointLightingSpecularColor * specularLightWeighting
        + uPointLightingDiffuseColor * diffuseLightWeighting;
    }

	gl_FragColor = vec4(vFragcolor.rgb * lightWeighting, vFragcolor.a);
}