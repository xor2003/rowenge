#version 450
// DX7 pre-transformed vertex (Rast7::Vert) -> Vulkan clip space.
// All varyings are noperspective: the software rasterizer interpolates
// every channel affinely and divides u*rhw by rhw per-pixel; the FS does
// the same so semantics match bit-for-bit.
layout(location=0) in vec2  aXY;
layout(location=1) in float aZ;
layout(location=2) in float aRHW;
layout(location=3) in vec4  aCol;     // B8G8R8A8_UNORM from 0xAARRGGBB
layout(location=4) in vec4  aSpec;
layout(location=5) in vec2  aUV0;
layout(location=6) in vec2  aUV1;
layout(location=7) in vec2  aUV2;

layout(push_constant) uniform PC {
    vec4 xform;     // xy: scale to NDC, zw: bias
    vec4 zmisc;     // x: zbias (normalized)
} pc;

layout(location=0) noperspective out vec4  vCol;
layout(location=1) noperspective out vec4  vSpec;
layout(location=2) noperspective out vec2  vU0;
layout(location=3) noperspective out vec2  vU1;
layout(location=4) noperspective out vec2  vU2;
layout(location=5) noperspective out vec2  vUW0;
layout(location=6) noperspective out vec2  vUW1;
layout(location=7) noperspective out vec2  vUW2;
layout(location=8) noperspective out float vRHW;
layout(location=9) noperspective out float vZ;      // un-biased z for fog

void main(){
    // zbias applied post-scale; clamped AFTER the add so biased z stays inside
    // the clip range (software clamps to the [0,zmax] integer instead).
    gl_Position = vec4(aXY.x*pc.xform.x + pc.xform.z,
                       aXY.y*pc.xform.y + pc.xform.w,
                       clamp(clamp(aZ,0.0,1.0) + pc.zmisc.x, 0.0, 1.0), 1.0);
    gl_PointSize = 1.0;
    vZ    = aZ;   // fog uses the raw interpolated z (software path)
    vCol  = aCol;  vSpec = aSpec;
    vU0 = aUV0; vU1 = aUV1; vU2 = aUV2;
    vUW0 = aUV0*aRHW; vUW1 = aUV1*aRHW; vUW2 = aUV2*aRHW;
    vRHW = aRHW;
}
