#include "globals.hpp"

#include <hyprland/src/debug/Log.hpp>
#include <hyprland/src/render/shaders/Shaders.hpp>
#include <regex>
#include <string>

#include <hyprland/src/debug/Log.hpp>
#include <hyprland/src/render/OpenGL.hpp>
#include <hyprland/src/render/Shader.hpp>
#include <string>
#include <unistd.h>

#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/config/ConfigManager.hpp>
#include <hyprland/src/desktop/Window.hpp>
#include <hyprland/src/render/Renderer.hpp>

// Do NOT change this function.
APICALL EXPORT std::string PLUGIN_API_VERSION() { return HYPRLAND_API_VERSION; }

inline CFunctionHook *g_pInitShadersHook = nullptr;
typedef bool (*origInitShaders)(CHyprOpenGLImpl *);

// we can't use these functions since they are static. So we'll have to copy
// them here. hopefully this does not destroy everything in the future :3 shader
// has #include "CM.glsl"
static void getCMShaderUniforms(SShader &shader) {
    shader.uniformLocations[SHADER_SKIP_CM] =
        glGetUniformLocation(shader.program, "skipCM");
    shader.uniformLocations[SHADER_SOURCE_TF] =
        glGetUniformLocation(shader.program, "sourceTF");
    shader.uniformLocations[SHADER_TARGET_TF] =
        glGetUniformLocation(shader.program, "targetTF");
    shader.uniformLocations[SHADER_SRC_TF_RANGE] =
        glGetUniformLocation(shader.program, "srcTFRange");
    shader.uniformLocations[SHADER_DST_TF_RANGE] =
        glGetUniformLocation(shader.program, "dstTFRange");
    shader.uniformLocations[SHADER_TARGET_PRIMARIES] =
        glGetUniformLocation(shader.program, "targetPrimaries");
    shader.uniformLocations[SHADER_MAX_LUMINANCE] =
        glGetUniformLocation(shader.program, "maxLuminance");
    shader.uniformLocations[SHADER_DST_MAX_LUMINANCE] =
        glGetUniformLocation(shader.program, "dstMaxLuminance");
    shader.uniformLocations[SHADER_DST_REF_LUMINANCE] =
        glGetUniformLocation(shader.program, "dstRefLuminance");
    shader.uniformLocations[SHADER_SDR_SATURATION] =
        glGetUniformLocation(shader.program, "sdrSaturation");
    shader.uniformLocations[SHADER_SDR_BRIGHTNESS] =
        glGetUniformLocation(shader.program, "sdrBrightnessMultiplier");
    shader.uniformLocations[SHADER_CONVERT_MATRIX] =
        glGetUniformLocation(shader.program, "convertMatrix");
}

// shader has #include "rounding.glsl"
static void getRoundingShaderUniforms(SShader &shader) {
    shader.uniformLocations[SHADER_TOP_LEFT] =
        glGetUniformLocation(shader.program, "topLeft");
    shader.uniformLocations[SHADER_FULL_SIZE] =
        glGetUniformLocation(shader.program, "fullSize");
    shader.uniformLocations[SHADER_RADIUS] =
        glGetUniformLocation(shader.program, "radius");
    shader.uniformLocations[SHADER_ROUNDING_POWER] =
        glGetUniformLocation(shader.program, "roundingPower");
}

static std::string patchShaderBorder() {
    std::string shader = SHADERS.at("border.frag");
    std::regex reIncludeRounding(R"(#include "rounding.glsl")");
    shader = std::regex_replace(shader, reIncludeRounding,
                                SHADERS.at("rounding.glsl"));

    std::regex reIncludeCm(R"(#include "CM.glsl")");
    shader = std::regex_replace(shader, reIncludeCm, SHADERS.at("CM.glsl"));

    std::regex re(R"(pixCoordOuter \-= fullSize \* 0.5 \- radiusOuter;)");
    shader = std::regex_replace(
        shader, re,
        "pixCoordOuter -= fullSize * 0.5 - radius + thick * (0.707 - 0.5);");

    return shader;
}

bool hkInitShaders(CHyprOpenGLImpl *thisptr) {
    if ((*(origInitShaders)g_pInitShadersHook->m_original)(thisptr) == false) {
        return false;
    }
    thisptr->m_shadersInitialized = false;

    GLuint prog;

    prog = thisptr->createProgram(thisptr->m_shaders->TEXVERTSRC,
                                  patchShaderBorder(), true);
    if (!prog)
        return false;

    thisptr->m_shaders->m_shBORDER1.program = prog;
    getCMShaderUniforms(thisptr->m_shaders->m_shBORDER1);
    getRoundingShaderUniforms(thisptr->m_shaders->m_shBORDER1);
    thisptr->m_shaders->m_shBORDER1.uniformLocations[SHADER_PROJ] =
        glGetUniformLocation(prog, "proj");
    thisptr->m_shaders->m_shBORDER1.uniformLocations[SHADER_THICK] =
        glGetUniformLocation(prog, "thick");
    thisptr->m_shaders->m_shBORDER1.uniformLocations[SHADER_POS_ATTRIB] =
        glGetAttribLocation(prog, "pos");
    thisptr->m_shaders->m_shBORDER1.uniformLocations[SHADER_TEX_ATTRIB] =
        glGetAttribLocation(prog, "texcoord");
    thisptr->m_shaders->m_shBORDER1.uniformLocations[SHADER_BOTTOM_RIGHT] =
        glGetUniformLocation(prog, "bottomRight");
    thisptr->m_shaders->m_shBORDER1
        .uniformLocations[SHADER_FULL_SIZE_UNTRANSFORMED] =
        glGetUniformLocation(prog, "fullSizeUntransformed");
    thisptr->m_shaders->m_shBORDER1.uniformLocations[SHADER_RADIUS_OUTER] =
        glGetUniformLocation(prog, "radiusOuter");
    thisptr->m_shaders->m_shBORDER1.uniformLocations[SHADER_GRADIENT] =
        glGetUniformLocation(prog, "gradient");
    thisptr->m_shaders->m_shBORDER1.uniformLocations[SHADER_GRADIENT2] =
        glGetUniformLocation(prog, "gradient2");
    thisptr->m_shaders->m_shBORDER1.uniformLocations[SHADER_GRADIENT_LENGTH] =
        glGetUniformLocation(prog, "gradientLength");
    thisptr->m_shaders->m_shBORDER1.uniformLocations[SHADER_GRADIENT2_LENGTH] =
        glGetUniformLocation(prog, "gradient2Length");
    thisptr->m_shaders->m_shBORDER1.uniformLocations[SHADER_ANGLE] =
        glGetUniformLocation(prog, "angle");
    thisptr->m_shaders->m_shBORDER1.uniformLocations[SHADER_ANGLE2] =
        glGetUniformLocation(prog, "angle2");
    thisptr->m_shaders->m_shBORDER1.uniformLocations[SHADER_GRADIENT_LERP] =
        glGetUniformLocation(prog, "gradientLerp");
    thisptr->m_shaders->m_shBORDER1.uniformLocations[SHADER_ALPHA] =
        glGetUniformLocation(prog, "alpha");
    thisptr->m_shaders->m_shBORDER1.createVao();

    thisptr->m_shadersInitialized = true;

    return true;
}

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
    PHANDLE = handle;

    const std::string HASH = __hyprland_api_get_hash();

    if (HASH != GIT_COMMIT_HASH) {
        HyprlandAPI::addNotification(
            PHANDLE, "[MyPlugin] Mismatched headers! Can't proceed.",
            CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
        throw std::runtime_error("[MyPlugin] Version mismatch");
    }

    std::string shader = patchShaderBorder();
    Debug::log(LOG, shader);

    static const auto METHODS =
        HyprlandAPI::findFunctionsByName(PHANDLE, "initShaders");
    g_pInitShadersHook = HyprlandAPI::createFunctionHook(
        handle, METHODS[0].address, (void *)&hkInitShaders);
    g_pInitShadersHook->hook();

    g_pHyprOpenGL->initShaders();

    return {"chamfer-borders-fix", "fixes chamfered borders", "revo", "0.1"};
}

APICALL EXPORT void PLUGIN_EXIT() {
    g_pInitShadersHook->unhook();

    g_pHyprOpenGL->initShaders();
}
