#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragTexCoord;
layout(location = 2) in vec3 worldPosition;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler3D texSampler3D;
layout(binding = 3) uniform CloudData {
    vec4 worldCamera;
    vec4[18] planes;
} cloud;
/*
  Find the intersection of a plane and a line.
  Parameters:
  P0 : A point on the plane
  P01: Another point of the plane
  P02: Another point of the plane
  la: Line point a
  lb: Line point b
       .P1__________
       |            |
       |        lb  |
       |       /    |
    P0 .______/_____.P2
             /
            /la
  Returns a vector vec.xyzw where x is and indicator. x < 0 if there is no intersection or
  infinite intersections (line is on plane).
  Otherwise, the intersection point coordinates are returned in vec.yzw.
*/
bool planeLineIntersection(vec3 P0, vec3 P1, vec3 P2, vec3 la, vec3 lb, inout vec3 result) {
    // Many thanks to wikipedia
    // Reference:
    // https://en.wikipedia.org/wiki/Line%E2%80%93plane_intersection
    vec3 P01 = P1 - P0;
    vec3 P02 = P2 - P0;

    vec3 lab = lb - la;
    float det = - dot(lab, cross(P01,P02));

    if (abs(det) < 0.01) {
        return false;
    }

    float t = 1.0 / det * dot(cross(P01, P02), la - P0);
    float u = 1.0 / det * dot(cross(P02, -lab), la - P0);
    float v = 1.0 / det * dot(cross(-lab, P01), la - P0);

    result = P0 + P01 * u + P02 * v;

    bool insideFace = u >= 0.0 && u <= 1.0 && v >= 0.0 && v <= 1.0;
    //bool insideSegment = t >= 0.0 && t <= 1.0; 

    return insideFace /*&& insideSegment*/;
}


void main() {
    float noiseValue = texture(texSampler3D, fragTexCoord).r;

    int nbIntersections = 0;
    vec3 intersections[6];
    vec3 result;
    for(int i = 0; i < 6; i++) {
        vec3 P0 = cloud.planes[i * 3].xyz;
        vec3 P1 = cloud.planes[i * 3 + 1].xyz;
        vec3 P2 = cloud.planes[i * 3 + 2].xyz;
        bool foundIntersection = planeLineIntersection(P0, P1, P2, cloud.worldCamera.xyz, worldPosition, intersections[nbIntersections]);
        if(foundIntersection){
            nbIntersections++;
        }
    }

    if(nbIntersections != 2){
        discard;
    }


    vec3 first = intersections[0].y < intersections[1].y ? intersections[0] : intersections[1];
    vec3 second = intersections[0].y < intersections[1].y ? intersections[1] : intersections[0];
    vec3 direction = second - first;
    float nbSamples = 64;
    float insideTreshold = 0.6;
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    float opacity = 0.55;
    float accumulation = 0.0;
    float distanceOffset = length(direction) / nbSamples;

    for(int i = 0; i <= nbSamples; i++){
        float c = float(i) / nbSamples;
        vec3 p = first + c * direction;
        //[-0.5, 0.5] -> [-1; 1]  -> [0; 2] -> [0; 1]
        p = (p * 2.0 + 1.0) / 2.0;
        float noiseValue = texture(texSampler3D, p).r;
        if(noiseValue > insideTreshold){
            accumulation += distanceOffset;
        }
    }

    noiseValue = exp(-accumulation / opacity);
    outColor = vec4(noiseValue, noiseValue, noiseValue, 1.0);
    //outColor = vec4(fragTexCoord.z, fragTexCoord.z, fragTexCoord.z, 1.0);
    //outColor = vec4(fragTexCoord.x, fragTexCoord.y, 0.0, 1.0);
}