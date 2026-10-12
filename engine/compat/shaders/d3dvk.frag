#version 450
// DX7 fixed-function pixel pipeline for the compat device: up to 3 texture
// stages with the same op/arg semantics as the software rasterizer
// (EvalStagesE/PixE in d3dsoft.h), then specular add, fog, and optional
// W-buffer depth write.
layout(location=0) noperspective in vec4  vCol;
layout(location=1) noperspective in vec4  vSpec;
layout(location=2) noperspective in vec2  vU0;
layout(location=3) noperspective in vec2  vU1;
layout(location=4) noperspective in vec2  vU2;
layout(location=5) noperspective in vec2  vUW0;
layout(location=6) noperspective in vec2  vUW1;
layout(location=7) noperspective in vec2  vUW2;
layout(location=8) noperspective in float vRHW;
layout(location=9) noperspective in float vZ;

layout(set=0,binding=0) uniform sampler2D tex0;
layout(set=0,binding=1) uniform sampler2D tex1;
layout(set=0,binding=2) uniform sampler2D tex2;

layout(location=0) out vec4 oCol;

// Push-constant layout shared with d3dvk.vert (VS reads only xform/zmisc).
layout(push_constant) uniform PC {
    vec4  xform;    // VS only
    vec4  zmisc;    // x: zbias(VS), y: fogStart, z: fogEnd
    vec4  fogCol;
    vec4  tfac;     // TEXTUREFACTOR normalized
    uvec4 cfg;      // x: nStage|persp<<4|spec<<5|fogT<<6|fogV<<7|ckey<<8|usew<<9
                    // y: ckeyLo rgb packed, z: ckeyHi rgb
    uvec4 stg0;     // x: cop|aop<<8|tci<<16|hasTex<<24
    uvec4 stg1;     // y: ca1|ca2<<8|aa1<<16|aa2<<24 (8b each: sel|comp|arep)
    uvec4 stg2;
} pc;

vec4 pick(uint ta, vec4 texC, vec4 cur, vec4 dif){
    uint sel = ta & 15u;
    vec4 v = sel==0u ? dif : sel==2u ? texC : sel==3u ? pc.tfac : cur;
    if((ta & 16u)!=0u) v.rgb = 1.0 - v.rgb;          // COMPLEMENT (rgb only)
    if((ta & 32u)!=0u) v.rgb = vec3(v.a);            // ALPHAREPLICATE
    return v;
}
vec2 stageUV(uint tci){
    if((pc.cfg.x & 16u)!=0u){                        // persp: u = (u*rhw)/rhw
        float w = vRHW!=0.0 ? vRHW : 1.0;
        if(tci==0u) return vUW0/w;
        if(tci==1u) return vUW1/w;
        return vUW2/w;
    }
    if(tci==0u) return vU0;
    if(tci==1u) return vU1;
    return vU2;
}
vec4 sampleTex(uint i, uint tci){
    vec2 uv = stageUV(tci);
    if(i==0u) return texture(tex0, uv);
    if(i==1u) return texture(tex1, uv);
    return texture(tex2, uv);
}

void main(){
    uint nStage = pc.cfg.x & 7u;
    vec4 dif = vCol;
    vec4 cur = dif;
    for(uint i=0u;i<3u;i++){
        if(i>=nStage) break;
        uvec4 s = i==0u ? pc.stg0 : i==1u ? pc.stg1 : pc.stg2;
        uint cop=s.x&255u, aop=(s.x>>8)&255u, tci=(s.x>>16)&3u;
        bool hasTex = (s.x>>24)!=0u;
        vec4 texC = vec4(1.0);
        if(hasTex){
            texC = sampleTex(i,tci);
            if((pc.cfg.x & 256u)!=0u){               // COLORKEYENABLE
                // Software key is a packed lexicographic compare:
                // (texC&0xffffff)>=lo && <=hi — replicate exactly.
                uvec3 q = uvec3(clamp(texC.rgb*255.0+0.5,0.0,255.0));
                uint packed = (q.r<<16)|(q.g<<8)|q.b;
                if(packed>=pc.cfg.y&&packed<=pc.cfg.z) texC.a=0.0;
            }
        }
        if(cop==13u){                                // BLENDTEXTUREALPHA
            vec4 a1 = pick(s.y&255u,texC,cur,dif);
            vec4 a2 = pick((s.y>>8)&255u,texC,cur,dif);
            cur.rgb = a1.rgb*(1.0-texC.a) + a2.rgb*texC.a;
        } else if(cop!=1u){                          // != DISABLE
            vec4 a1 = pick(s.y&255u,texC,cur,dif);
            vec4 a2 = pick((s.y>>8)&255u,texC,cur,dif);
            if(cop==2u)      cur.rgb = a1.rgb;                     // SELECTARG1
            else if(cop==3u) cur.rgb = a2.rgb;                     // SELECTARG2
            else if(cop==6u) cur.rgb = 4.0*a1.rgb*a2.rgb;          // MODULATE4X
            else if(cop==7u) cur.rgb = a1.rgb+a2.rgb;              // ADD
            else if(cop==8u) cur.rgb = a1.rgb+a2.rgb-0.5;          // ADDSIGNED
            else if(cop==10u)cur.rgb = a1.rgb-a2.rgb;              // SUBTRACT
            else             cur.rgb = a1.rgb*a2.rgb;              // MODULATE(2X)
        }
        uint aop_=aop;
        if(aop_!=1u){
            float aa1 = pick((s.y>>16)&255u,texC,cur,dif).a;
            float aa2 = pick((s.y>>24)&255u,texC,cur,dif).a;
            if(aop_==2u)      cur.a = aa1;
            else if(aop_==3u) cur.a = aa2;
            else if(aop_==6u) cur.a = 4.0*aa1*aa2;
            else if(aop_==7u) cur.a = aa1+aa2;
            else              cur.a = aa1*aa2;
        }
        cur = clamp(cur,0.0,1.0);
    }
    if((pc.cfg.x & 32u)!=0u)                         // SPECULARENABLE
        cur.rgb = min(cur.rgb + vSpec.rgb, 1.0);
    if((pc.cfg.x & 192u)!=0u){                       // fog table|vertex
        float f = -1.0;
        if((pc.cfg.x & 64u)!=0u){                    // table LINEAR
            float fs=pc.zmisc.y, fe=pc.zmisc.z;
            f = (fe-fs)!=0.0 ? (fe-vZ)/(fe-fs) : 0.0;
        } else
            f = vSpec.a;                             // vertex fog: spec alpha
        // Software skips fog when f<0 (z beyond fogEnd) — not a clamp.
        if(f>=0.0){
            f = min(f,1.0);
            cur.rgb = mix(pc.fogCol.rgb, cur.rgb, f);
        }
    }
#ifdef WBUF
    // ZENABLE==USEW: w-buffer via a dedicated pipeline variant — a conditional
    // gl_FragDepth store is not reliable (drivers may treat any FragDepth
    // write as unconditional), so the w-map is compile-time, selected by the
    // pipeline key's usew bit.
    {
        float w = vRHW>1e-20 ? 1.0/vRHW : 3.4e38;
        gl_FragDepth = w>3.3e37 ? 1.0 : w/(1.0+w);   // order-preserving map
    }
#endif
    oCol = cur;
}
