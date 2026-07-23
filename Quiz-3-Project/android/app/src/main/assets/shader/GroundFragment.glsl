#version 300 es

precision highp float;

in vec3 vWorldPosition;
in vec3 vPositionView;
in vec3 vNormalView;

uniform vec3 LIGHTPOSITIONVIEW;

out vec4 FragmentColor;

float checker(vec2 worldXZ, float tileSize)
{
    vec2 cell = floor(worldXZ / tileSize);
    return mod(cell.x + cell.y, 2.0);
}

void main()
{
    float parity = checker(vWorldPosition.xz, 0.65);
    vec3 material = mix(
        vec3(0.080, 0.090, 0.110),
        vec3(0.85, 0.87, 0.90),
        parity);

    vec3 normal = normalize(vNormalView);
    vec3 lightDirection = normalize(LIGHTPOSITIONVIEW - vPositionView);
    vec3 viewDirection = normalize(-vPositionView);
    vec3 reflectedDirection = reflect(-lightDirection, normal);

    float diffuseAmount = max(dot(normal, lightDirection), 0.0);
    float specularAmount = 0.0;
    if (diffuseAmount > 0.0) {
        specularAmount = pow(
            max(dot(viewDirection, reflectedDirection), 0.0), 24.0);
    }

    vec3 ambient = material * 0.28;
    vec3 diffuse = material * diffuseAmount * 0.72;
    vec3 specular = vec3(1.0) * specularAmount * 0.30;
    FragmentColor = vec4(ambient + diffuse + specular, 1.0);
}
