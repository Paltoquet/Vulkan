#version 450

/* --------------------------- Varying --------------------------- */

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragTexCoord;
layout(location = 2) in vec3 worldPosition;

layout(location = 0) out vec4 outColor;

/* --------------------------- Uniforms --------------------------- */

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    float time;
} ubo;

layout(set = 1, binding = 1) uniform sampler3D texSampler3D;

layout(set = 1, binding = 3) uniform CloudData {
    vec4 worldCamera;
    vec4 worldLightPos;
    vec4 bboxMin;
    vec4 bboxMax;
    vec4 lightColor;
    vec4 lightAbsorption;
    vec4 densityTreshold;
    vec4 phaseParams;
    vec4 fogSpeed;
    float fogDensity;
} cloud;

/* --------------------------- Defines --------------------------- */

float nbSamples = 128.0;
float nbLightSamples = 32.0;
float darknessThreshold = 0.05;

// Returns (dstToBox, dstInsideBox). If ray misses box, dstInsideBox will be zero
vec2 rayBoxDist(vec3 bboxMin, vec3 bboxMax, vec3 origin, vec3 invRaydir) {
    // Adapted from: http://jcgt.org/published/0007/03/04/
    vec3 t0 = (bboxMin - origin) * invRaydir;
    vec3 t1 = (bboxMax - origin) * invRaydir;
    vec3 tmin = min(t0, t1);
    vec3 tmax = max(t0, t1);
    
    float dstA = max(max(tmin.x, tmin.y), tmin.z);
    float dstB = min(tmax.x, min(tmax.y, tmax.z));

    // CASE 1: ray intersects box from outside (0 <= dstA <= dstB)
    // dstA is dst to nearest intersection, dstB dst to far intersection

    // CASE 2: ray intersects box from inside (dstA < 0 < dstB)
    // dstA is the dst to intersection behind the ray, dstB is dst to forward intersection

    // CASE 3: ray misses box (dstA > dstB)

    float dstToBox = max(0, dstA);
    float dstInsideBox = max(0, dstB - dstToBox);
    return vec2(dstToBox, dstInsideBox);
}

float sample3DTexture(vec3 pos)
{
    //[-0.5, 0.5] -> [-1; 1]  -> [0; 2] -> [0; 1]
    pos = (pos * 2.0 + 1.0) / 2.0;
    float speed = 0.12;
    // Change depth value for adding a scrolling effect
    pos.z += cloud.fogSpeed.x * ubo.time;
    pos.z = mod(pos.z, 1.0);
    float noiseValue = texture(texSampler3D, pos).r;
    return noiseValue;
}

float lightMarch(vec3 position)
{
    float insideTreshold = cloud.densityTreshold.x;
    vec3 dirToLight = normalize(cloud.worldLightPos.xyz);
    float dstInsideBox = rayBoxDist(cloud.bboxMin.xyz, cloud.bboxMax.xyz, position, vec3(1.0) / dirToLight).y;

    float stepSize = dstInsideBox / nbLightSamples;
    float totalDensity = 0;
    
    for (int step = 0; step < nbLightSamples; step++) {
        position += dirToLight * stepSize;
        float density = sample3DTexture(position);
        density = density > insideTreshold ? density : 0.0;
        totalDensity += max(0, density * stepSize);
    }
    
    float transmittance = exp(-totalDensity / cloud.lightAbsorption.x);
    return transmittance;
}

// Henyey-Greenstein 
float hg(float a, float g) {
    float g2 = g*g;
    return (1-g2) / (4.0 * 3.1415 * pow(1.0 + g2-2.0 * g * (a), 1.5));
}

float phase(float a) {
    float blend = 0.5;
    float hgBlend = hg(a, cloud.phaseParams.x) * (1.0-blend) + hg(a,-cloud.phaseParams.y) * blend;
    //return hgBlend;
    return cloud.phaseParams.w + hgBlend * cloud.phaseParams.z;
}

void main() {

    float noiseValue = texture(texSampler3D, fragTexCoord).r;

    vec3 origin = cloud.worldCamera.xyz;
    vec3 rayDir = worldPosition - origin;
    rayDir = normalize(rayDir);

    vec3 invRayDir = vec3(1.0) / rayDir;
    vec2 boxDistance = rayBoxDist(cloud.bboxMin.xyz, cloud.bboxMax.xyz, origin, invRayDir);
    float cosAngle = dot(rayDir, cloud.worldLightPos.xyz);
    float phaseVal = phase(cosAngle);

    bool hasFoundIntersection = boxDistance.y > 0;
    if(!hasFoundIntersection){
        discard;
    }

    vec3 firstPoint = origin + rayDir * boxDistance.x;
    vec3 secondPoint = firstPoint + rayDir * boxDistance.y;

    vec3 direction = secondPoint - firstPoint;
    float totalDistance = length(direction);


    // March through volume:
    vec3 rayPos;
    float distTravelled = 0.0;
    float transmittance = 1.0;
    vec3 lightEnergy = vec3(0.0);
    float stepSize = 1.0 / nbSamples;
    float accumulation = 0.0;
    float insideTreshold = cloud.densityTreshold.x;

    float lightStepSize = 1.0 / nbLightSamples; 

    while (distTravelled < totalDistance) {
        rayPos = firstPoint + rayDir * distTravelled;
        float density = cloud.phaseParams.x * sample3DTexture(rayPos);
        
        /*if (density > cloud.densityTreshold.x) {
            float lightTransmittance = lightMarch(rayPos);
            //lightEnergy += vec3(density * stepSize * transmittance * lightTransmittance);
            //lightEnergy += vec3(density * stepSize * transmittance * lightTransmittance);
            //transmittance *= exp(-density * stepSize / cloud.fogDensity);
        
            // Exit early if T is close to zero as further samples won't affect the result much
            //if (transmittance < 0.01) {
            //    break;
            //}
            float noiseValue = sample3DTexture(rayPos);
            if(noiseValue > insideTreshold){
                accumulation += lightTransmittance * stepSize;
            }

        }*/


        float shadowDist = 0.0;
        vec3 lightVector;
        vec3 lightDir = normalize(cloud.worldLightPos.xyz - rayPos);
        vec3 invLightDir = 1.0 / lightDir;
        vec2 lightBoxDistance = rayBoxDist(cloud.bboxMin.xyz, cloud.bboxMax.xyz, rayPos, invLightDir);
        vec3 lightIntersection = rayPos + lightDir * lightBoxDistance.y;
        vec3 lightSamplePoint = rayPos;
        //vec3 lightStepVector = (lightIntersection - rayPos) / nbLightSamples;
        vec3 lightStepVector = lightDir * lightStepSize;

        if (density > cloud.densityTreshold.x) {
            for(int i = 0; i < nbLightSamples; i++){
                lightSamplePoint += lightStepVector;
                float maxCoord = max(max(abs(lightSamplePoint.x), abs(lightSamplePoint.y)), abs(lightSamplePoint.z));
                bool outSideBox = (maxCoord > 1.0);
                if(!outSideBox){
                    float lsample = sample3DTexture(lightSamplePoint);
                    shadowDist += lsample > cloud.phaseParams.y ? lsample : 0.0;
                }
            }

            float shadowTerm = exp(-shadowDist * cloud.lightAbsorption.x);
            float absorbedLight = shadowTerm * density;
            accumulation += absorbedLight * transmittance;
            //transmittance *= exp(-density * stepSize / cloud.fogDensity);
            float curdensity = density * stepSize;
            transmittance *= 1 - curdensity;
        }

        distTravelled += stepSize;
    }

    //float lightTransmittance = lightMarch(firstPoint + rayDir * 64.0 * stepSize);

    lightEnergy = clamp(lightEnergy, vec3(0.0), vec3(1.0));
    //outColor.xyz = cloud.lightColor.xyz * lightEnergy;
    outColor.xyz = cloud.lightColor.xyz * accumulation;
    //outColor.x = phaseVal;
    //outColor.xyz = vec3(lightTransmittance);
    outColor.a = 1.0;


    //noiseValue = exp(-accumulation / cloud.fogDensity);
    //noiseValue = clamp(noiseValue, 0.0, 1.0);
    //noiseValue = 1.0 - noiseValue;
    //outColor = vec4(noiseValue, noiseValue, noiseValue, 1.0);


    /*float accumulation = 0.0;
    float distanceOffset = length(direction) / nbSamples;

    for(int i = 0; i < nbSamples; i++){
        float c = float(i) / nbSamples;
        vec3 p = firstPoint + c * direction;
        //[-0.5, 0.5] -> [-1; 1]  -> [0; 2] -> [0; 1]
        p = (p * 2.0 + 1.0) / 2.0;
        float noiseValue = sample3DTexture(p);
        if(noiseValue > insideTreshold){
            accumulation += distanceOffset;
        }
    }

    noiseValue = exp(-accumulation / cloud.fogDensity);
    noiseValue = clamp(noiseValue, 0.0, 1.0);
    noiseValue = 1.0 - noiseValue;
    outColor = vec4(noiseValue, noiseValue, noiseValue, 1.0);*/
    //outColor = vec4(fragTexCoord.z, fragTexCoord.z, fragTexCoord.z, 1.0);
    //outColor = vec4(fragTexCoord.x, fragTexCoord.y, 0.0, 1.0);
}