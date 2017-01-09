
attribute vec3 aVertexPosition;
attribute vec3 aVertexNormal;
attribute vec2 aTextureCoord;
attribute vec3 aVertexFrontColor;

uniform mat4 uMVMatrix;
uniform mat4 uPMatrix;
uniform mat3 uNMatrix;

varying vec4 fragcolor;

uniform float uMaterialShininess;

uniform vec3 uAmbientColor;
uniform vec3 uPointLightingLocation[3];
uniform float uPointEnabled[3];
uniform vec3 uPointLightingSpecularColor;
uniform vec3 uPointLightingDiffuseColor;

uniform sampler2D uSampler;
uniform int uTriangleColor;

void main(void) {
    //vPosition = uMVMatrix * vec4(aVertexPosition, 1.0);
    gl_Position = uPMatrix * uMVMatrix * vec4(aVertexPosition, 1.0);
    
    vec3 normal = normalize(uNMatrix * aVertexNormal);

    float specularLightWeighting = 0.0;
    
    vec3 eyeDirection = normalize(-(uMVMatrix * vec4(aVertexPosition, 1.0)).xyz);
    vec3 lightWeighting = vec3(0.0, 0.0, 0.0);

    for (int i = 0; i < 3; i++) {
        if (uPointEnabled[i] == 0.0)
            continue;
        vec3 lightDirection = normalize(uPointLightingLocation[i] - (uMVMatrix * vec4(aVertexPosition, 1.0)).xyz);
        vec3 reflectionDirection = reflect(-lightDirection, normal);
        specularLightWeighting = pow(max(dot(reflectionDirection, eyeDirection), 0.0), uMaterialShininess);

        float diffuseLightWeighting = max(dot(normal, lightDirection), 0.0);
        lightWeighting += uAmbientColor
            + uPointLightingSpecularColor * specularLightWeighting
            + uPointLightingDiffuseColor * diffuseLightWeighting;
    }

    vec4 fragmentColor;
    if (uTriangleColor == 0) {
        fragmentColor = texture2D(uSampler, vec2(aTextureCoord.s, aTextureCoord.t));
    } else {
        fragmentColor = vec4(aVertexFrontColor, 1.0);
    }
    fragcolor = vec4(fragmentColor.rgb * lightWeighting, fragmentColor.a);
}