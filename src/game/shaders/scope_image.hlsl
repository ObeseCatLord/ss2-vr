// Native Poly Bump VS remains installed: diffuse UV is TEXCOORD3 and its
// COLOR0 alpha includes fade/fog in both VS families. Native PP2 instead
// consumes TEXCOORD2.w; main is not a replacement for that family's alpha.
// The opaque entry is used only after live source and ordered-draw admission.
sampler2D sceneImage : register(s0);
sampler2D stockDiffuse : register(s3);
// Map the actual native post-c8/c9 UV into the proper optic camera's frame.
float4 uvToOpticX : register(c0);
float4 uvToOpticY : register(c1);
float4 uvToOpticZ : register(c2);
float4 eyeInOptic : register(c3);
// xy = (0.5/(M*tanHalfFovX), -0.5/(M*tanHalfFovY)); zw = (0.5,0.5).
float4 imageLookup : register(c4);
// x = admitted aperture visibility.
float4 presentation : register(c5);
// xy = frozen native aim in source UV; zw = reticle half-width/length (zero disables).
float4 reticleParameters : register(c6);

float3 scopeImageColor(float2 uv) {
    float3 q = float3(uv,1);
    float3 aperturePosition = float3(dot(q,uvToOpticX.xyz),dot(q,uvToOpticY.xyz),dot(q,uvToOpticZ.xyz));
    float3 ray = aperturePosition-eyeInOptic.xyz;
    float forward = -ray.z;
    float2 lookup = ray.xy/max(forward,0.000001)*imageLookup.xy+imageLookup.zw;
    // Invalid/edge-of-source pixels are opaque dark glass, not transparent holes
    // that could lose original gun occlusion. The original triangle aperture,
    // depth/stencil, alpha-test and blend remain native.
    float visible = step(0.000001,forward)*step(0,lookup.x)*step(lookup.x,1)*
                    step(0,lookup.y)*step(lookup.y,1)*saturate(presentation.x);
    float2 center = abs(lookup-reticleParameters.xy);
    float reticle = saturate(step(center.x,reticleParameters.z)*step(center.y,reticleParameters.w)+
                            step(center.y,reticleParameters.z)*step(center.x,reticleParameters.w))*
                    step(0.000001,reticleParameters.z);
    return tex2D(sceneImage,lookup).rgb*(1-reticle)*visible;
}

// Experimental PP-only alpha path; exact sampling/arithmetic and material
// association remain required before any native replacement.
float4 main(float2 uv : TEXCOORD3, float4 nativeColor : COLOR0) : COLOR0 {
    float alpha = tex2D(stockDiffuse,uv).a*nativeColor.a;
    return float4(scopeImageColor(uv),alpha);
}

// Admitted first slice: additional cap-only RGB pass after original native
// opaque draw. Alpha writes MUST be masked by the eventual native adapter;
// alpha/blend/stencil/fog must be disabled, depth equal with no depth writes.
// No native alpha interpolant or diffuse sampler participates in this entry.
float4 opaque(float2 uv : TEXCOORD3) : COLOR0 {
    return float4(scopeImageColor(uv),0);
}
