#version 300 es

precision highp float;

in vec3 vPositionView;
in vec3 vNormalView;

uniform vec3 PARTCOLOR;
uniform vec3 LIGHTPOSITIONVIEW;

out vec4 FragmentColor;

void main()
{
    vec3 normal = normalize(vNormalView);
    vec3 lightDirection = normalize(LIGHTPOSITIONVIEW - vPositionView);
    vec3 viewDirection = normalize(-vPositionView);
    vec3 reflectedDirection = reflect(-lightDirection, normal);

    float diffuseAmount = max(dot(normal, lightDirection), 0.0);
    float specularAmount = 0.0;
    if (diffuseAmount > 0.0) {
        specularAmount = pow(
            max(dot(viewDirection, reflectedDirection), 0.0), 32.0);
    }

    vec3 ambient = PARTCOLOR * 0.32;
    vec3 diffuse = PARTCOLOR * diffuseAmount * 0.68;
    vec3 specular = vec3(1.0) * specularAmount * 0.65;
    FragmentColor = vec4(ambient + diffuse + specular, 1.0);
}
