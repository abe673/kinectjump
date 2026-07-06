
#include "DepthRenderer.h"
#include <algorithm>
#include "rlgl.h"

using namespace kinect;

static const int w = DepthWidth;
static const int h = DepthHeight;

static const char* gray_fs = R"(
#version 330
in vec2 fragTexCoord;
out vec4 finalColor;
uniform sampler2D texture0;

void main()
{
    float v = texture(texture0, fragTexCoord).r;
    finalColor = vec4(v, v, v, 1.0);
}
)";

DepthRenderer::DepthRenderer() {}

DepthRenderer::~DepthRenderer()
{
    if (!initialized) return;
    UnloadTexture(depthTex);
    UnloadShader(grayShader);
    UnloadImage(image);
}


void DepthRenderer::Init()
{
    image = GenImageColor(w, h, BLACK);   // RGBA8
    depthTex = LoadTextureFromImage(image);
    grayShader = LoadShaderFromMemory(nullptr, gray_fs);

    initialized = true;
}


void DepthRenderer::Update(const kinect::Frame& frame)
{
    if (!initialized || !frame.hasDepth)
        return;

    const uint16_t* src = frame.depth.data.data();
    uint8_t* dst = static_cast<uint8_t*>(image.data); // RGBA buffer

    for (int i = 0; i < w * h; ++i)
    {
        uint16_t d = src[i] >> 3; // depth 13-bit
        uint8_t v = (d == 0) ? 0 : uint8_t(
            255 - std::min(d * 255 / 4500, 255)
        );

        // RGBA (WAJIB 4 channel)
        dst[i * 4 + 0] = v; // R
        dst[i * 4 + 1] = v; // G
        dst[i * 4 + 2] = v; // B
        dst[i * 4 + 3] = 255; // A
    }

    UpdateTexture(depthTex, dst);
}


void DepthRenderer::DrawSkeletons(const kinect::Frame& frame,int x, int y, float scale){
    if (!frame.hasSkeleton) return;

    for (const auto& sk : frame.skeletons)
    {
        if (!sk.tracked) continue;
        // bones
        for (const auto& bone : kinect::Bones){
            const auto& j0 = sk.get(bone.first);
            const auto& j1 = sk.get(bone.second);

            if (j0.state != TrackingState::Tracked ||
                j1.state != TrackingState::Tracked)
                continue;

            DrawLineEx(
                {x + j0.depthPosition.x * scale,y + j0.depthPosition.y * scale},
                {x + j1.depthPosition.x * scale,y + j1.depthPosition.y * scale}, 5, RED);
            }

        // joints
        for (const auto& joint : sk.joints)
        {
            if (joint.state != TrackingState::Tracked) continue;

            DrawCircle(
                x + joint.depthPosition.x * scale,
                y + joint.depthPosition.y * scale,
                3 * scale,
                GREEN
            );
        }
    }
}


void DepthRenderer::Draw(int x, int y, float scale)
{
    if (!initialized) return;
    BeginShaderMode(grayShader);
    DrawTextureEx(
        depthTex,
        { (float)x, (float)y },
        0.0f,
        scale,
        WHITE
    );
    EndShaderMode();
}

