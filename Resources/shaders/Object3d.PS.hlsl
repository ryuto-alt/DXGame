#include "object3d.hlsli"

struct Material
{
    float32_t4 color;
    int32_t enableLighting;
    float32_t4x4 uvTransform;
};

struct DirectionalLight
{
    float32_t4 color;
    float32_t3 direction;
    float intensity;
};

ConstantBuffer<Material> gMaterial : register(b0);
Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSample : register(s0);

ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    float4 transformedUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float32_t4 textureColor = gTexture.Sample(gSample, transformedUV.xy);
    
    // アルファ値が閾値未満の場合、そのピクセルを描画しない
    if (textureColor.a < 0.1f)
    {
        discard; // このピクセルの処理を中止
    }
    
    PixelShaderOutput output;
    
    // 正規化された法線を使用（念のため再度正規化）
    float3 normal = normalize(input.normal);
    
    if (gMaterial.enableLighting != 0)
    {
        // 半球ライティングを採用
        float3 lightDir = normalize(-gDirectionalLight.direction);
        float NdotL = dot(normal, lightDir);
        
        // 滑らかなシェーディングのための改良されたライティング計算
        float diffuse = saturate(NdotL * 0.5f + 0.5f); // 半球ライティング
        
        // リムライトを追加（輪郭を強調）
        float3 viewDir = float3(0.0f, 0.0f, -1.0f); // カメラ方向（適宜調整）
        float rim = 1.0f - saturate(dot(normal, viewDir));
        rim = pow(rim, 3.0f) * 0.3f; // リム効果を調整
        
        // 最終的なライティング計算
        float3 lighting = gDirectionalLight.color.rgb * diffuse * gDirectionalLight.intensity;
        lighting += rim * gDirectionalLight.color.rgb; // リムライトを追加
        
        output.color.rgb = gMaterial.color.rgb * textureColor.rgb * lighting;
        output.color.a = gMaterial.color.a * textureColor.a;
    }
    else
    {
        output.color = gMaterial.color * textureColor;
    }
    
    return output;
}