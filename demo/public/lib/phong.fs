// <script id="per-fragment-lighting-fs" type="x-shader/x-fragment">
precision mediump float;


varying vec2 vTextureCoord;

varying vec3 vLightDirection[3];
varying vec3 vNormalDirection;
varying vec3 vVertexPosition;
varying vec3 vVertexFrontColor;

uniform float uMaterialShininess;

uniform vec3 uAmbientColor;
uniform vec3 uPointLightingSpecularColor;
uniform vec3 uPointLightingDiffuseColor;
uniform float uPointEnabled[3];

uniform sampler2D uSampler;

uniform int uTriangleColor;

void main(void) {
	vec3 tn = vNormalDirection;

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
	vec4 fragmentColor;
    if (uTriangleColor == 0) {
        fragmentColor = texture2D(uSampler, vec2(vTextureCoord.s, vTextureCoord.t));
    } else {
        fragmentColor = vec4(vVertexFrontColor, 1.0);
    }
	gl_FragColor = vec4(fragmentColor.rgb * lightWeighting, fragmentColor.a);
}
// </script>