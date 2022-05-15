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
float nbLightSamples = 58.0;
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

void main() {

    vec3 origin = cloud.worldCamera.xyz;
    vec3 rayDir = worldPosition - origin;
    rayDir = normalize(rayDir);

    vec3 invRayDir = vec3(1.0) / rayDir;
    vec2 boxDistance = rayBoxDist(cloud.bboxMin.xyz, cloud.bboxMax.xyz, origin, invRayDir);
    float cosAngle = dot(rayDir, cloud.worldLightPos.xyz);

    bool hasFoundIntersection = boxDistance.y > 0;
    if(!hasFoundIntersection){
        discard;
    }

    vec3 firstPoint = origin + rayDir * boxDistance.x;
    vec3 secondPoint = firstPoint + rayDir * boxDistance.y;

    vec3 direction = secondPoint - firstPoint;
    float totalDistance = length(direction);

    // March through volume:
    vec3 currentPosition;
    float distTravelled = 0.0;
    float transmittance = 1.0;
    vec3 lightDir;
    float stepSize = 1.0 / nbSamples;
    float accumulation = 0.0;
    float lightStepSize = 1.0 / nbLightSamples; 

    while (distTravelled < totalDistance) {
        currentPosition = firstPoint + rayDir * distTravelled;
        float density = cloud.phaseParams.x * sample3DTexture(currentPosition);

        float shadowValue = 0.0;
        lightDir = normalize(cloud.worldLightPos.xyz - currentPosition);
        vec3 invLightDir = 1.0 / lightDir;
        vec2 lightBoxDistance = rayBoxDist(cloud.bboxMin.xyz, cloud.bboxMax.xyz, currentPosition, invLightDir);
        vec3 lightSamplePoint = currentPosition;
        vec3 lightStepVector = lightDir * lightStepSize;
        vec3 lightLastPoint = currentPosition + lightDir * lightBoxDistance.y;
        float lightDistanceToTravel = length(lightLastPoint - currentPosition);
        float lightDistanceTravelled = 0.0;

        //if (density > cloud.densityTreshold.x) {
            while (lightDistanceTravelled < lightDistanceToTravel) {
                float lsample = 1.0;
                float maxCoord = max(max(abs(lightSamplePoint.x), abs(lightSamplePoint.y)), abs(lightSamplePoint.z));
                bool outSideBox = (maxCoord > 1.0);
                if(!outSideBox){
                    //float lsample = cloud.phaseParams.x * sample3DTexture(lightSamplePoint);
                    //shadowDist += lsample > cloud.phaseParams.y ? lsample : 0.0;

                    lsample = sample3DTexture(lightSamplePoint);
                }
                shadowValue += lsample;
                lightDistanceTravelled += lightStepSize;
                lightSamplePoint += lightStepVector;
            }

            float shadowTerm = exp(-shadowValue * cloud.lightAbsorption.x);
            float curdensity = density * stepSize;
            float absorbedLight = shadowTerm * curdensity;
            accumulation += absorbedLight * transmittance;
            //transmittance *= exp(-density * stepSize / cloud.fogDensity);
            transmittance *= 1.0 - curdensity;
        //}

        distTravelled += stepSize;
    }


    outColor.xyz = cloud.lightColor.xyz * accumulation;
    outColor.a = 1.0;

    /*while (distTravelled < totalDistance) {
        currentPosition = firstPoint + rayDir * distTravelled;
        float cursample = cloud.phaseParams.x * sample3DTexture(currentPosition);
        accumulation += cursample * stepSize;
        distTravelled += stepSize;
    }

    outColor = vec4(accumulation);*/

}