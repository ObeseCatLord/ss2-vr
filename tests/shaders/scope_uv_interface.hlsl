// Mod-owned offline interface fixture. Never installed or used to replace a VS.
float4 uvRow0 : register(c8);
float4 uvRow1 : register(c9);
struct Input {
    float4 position : TEXCOORD0;
    float4 normal : TEXCOORD1;
    float4 tangent : TEXCOORD2;
    float4 uv : TEXCOORD3;
};
struct Output {
    float4 position : POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD3;
};
Output main(Input input) {
    Output result;
    result.position=input.position;
    result.color=input.normal+input.tangent;
    result.uv=float2(dot(input.uv,uvRow0),dot(input.uv,uvRow1));
    return result;
}
