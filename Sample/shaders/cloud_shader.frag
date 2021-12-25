#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragTexCoord;
layout(location = 2) in vec3 cameraDirection;
layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler3D texSampler3D;

layout(binding = 3) uniform CloudData {
    vec4 worldCamera;
    vec3[24] planes;
} cloud;
/*
  Find the intersection of a plane and a line.
  Parameters:
  P0 : A point on the plane
  P01: Another point of the plane
  P02: Another point of the plane
  la: Line point a
  lb: Line point b
       .P01_________
       |            |
       |        lb  |
       |       /    |
    P0 .______/_____.P02
             /
            /la
  Returns a vector vec.xyzw where x is and indicator. x < 0 if there is no intersection or
  infinite intersections (line is on plane).
  Otherwise, the intersection point coordinates are returned in vec.yzw.
*/

bool planeLineIntersection(vec3 P0, vec3 P01, vec3 P02, vec3 la, vec3 lb, inout vec3 result) {
    // Many thanks to wikipedia
    // Reference:
    // https://en.wikipedia.org/wiki/Line%E2%80%93plane_intersection
    vec3 lab = lb - la;
    float det = - dot(lab, cross(P01,P02));

    if (abs(det) < 0.01) {
        return false;
    }

    float t = 1.0/det * dot(cross(P01, P02), la - P0);
    float u = 1.0/det * dot(cross(P02, -lab), la - P0);
    float v = 1.0/det * dot(cross(-lab, P01), la - P0);

    result = P0 + P01 * u + P02 * v;

    bool insideFace = u >= 0.0 && u <= 1.0 && v >= 0.0 && v <= 1.0;
    //bool insideSegment = t >= 0.0 && t <= 1.0; 

    return insideFace /*&& insideSegment*/;
}


void main() {
    float noiseValue = texture(texSampler3D, fragTexCoord).r;
    outColor = vec4(noiseValue, noiseValue, noiseValue, 1.0);
    //outColor = vec4(fragTexCoord.z, fragTexCoord.z, fragTexCoord.z, 1.0);
    //outColor = vec4(fragTexCoord.x, fragTexCoord.y, 0.0, 1.0);
}