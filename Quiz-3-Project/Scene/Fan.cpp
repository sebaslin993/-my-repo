#define LOG_TAG "Quiz3Fan"
#include "Fan.h"
#include "ShaderHelper.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "../third_party/tinyobjloader/tiny_obj_loader.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <fstream>
#include <sstream>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace {

constexpr int kBasePart = 0;
constexpr int kPolePart = 1;
constexpr int kHubPart = 2;
constexpr int kFirstBladePart = 3;
constexpr float kPi = 3.14159265358979323846f;
const glm::vec3 kHighlightColour(1.0f, 0.72f, 0.05f);

void AddVertex(std::vector<GLfloat>& vertices,
               float x, float y, float z,
               float nx, float ny, float nz)
{
    vertices.insert(vertices.end(), {x, y, z, nx, ny, nz});
}

void LogPickState(int part, bool highlighted)
{
    const char* state = highlighted ? "on" : "off";
    if (part == kBasePart) {
        LOGI("WindmillScene: Base picked - highlight %s", state);
    } else if (part == kPolePart) {
        LOGI("WindmillScene: Pole picked - highlight %s", state);
    } else if (part == kHubPart) {
        LOGI("WindmillScene: MotorEngine picked - highlight %s", state);
    } else if (part >= kFirstBladePart) {
        LOGI("WindmillScene: Blade%d picked - highlight %s",
             part - kFirstBladePart + 1, state);
    }
}
} // namespace

#ifdef PLATFORM_ANDROID
Fan::Fan(AAssetManager* manager) : assetManager(manager) { modelType = FanType; }
#else
Fan::Fan() { modelType = FanType; }
#endif

Fan::~Fan()
{
    DeleteMesh(cube);
    DeleteMesh(cylinder);
    DeleteMesh(sphere);
    if (program) glDeleteProgram(program);
}

void Fan::DeleteMesh(Mesh& mesh)
{
    if (mesh.vao) glDeleteVertexArrays(1, &mesh.vao);
    if (mesh.vbo) glDeleteBuffers(1, &mesh.vbo);
    if (mesh.ibo) glDeleteBuffers(1, &mesh.ibo);
    mesh = {};
}

void Fan::UploadMesh(Mesh& mesh, const std::vector<GLfloat>& vertices,
                     const std::vector<GLushort>& indices)
{
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(vertices.size() * sizeof(GLfloat)),
                 vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &mesh.ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(indices.size() * sizeof(GLushort)),
                 indices.data(), GL_STATIC_DRAW);

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(GLfloat), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat),
        reinterpret_cast<void*>(3 * sizeof(GLfloat)));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
    glBindVertexArray(0);

    mesh.indexCount = static_cast<GLsizei>(indices.size());
}

bool Fan::LoadObjMesh(const char* fileName, Mesh& mesh)
{
    const std::string displayPath = std::string("Models/") + fileName;
    std::string source;

#ifdef PLATFORM_ANDROID
    // Gradle adds Quiz-3-Project/Models as an Android asset source root.
    source = ShaderHelper::loadAsset(assetManager, fileName);
#else
    const std::array<std::string, 3> searchPaths = {
        displayPath,
        std::string("assets/Models/") + fileName,
        std::string("../Models/") + fileName
    };
    for (const std::string& path : searchPaths) {
        std::ifstream input(path);
        if (!input.is_open()) continue;
        std::ostringstream buffer;
        buffer << input.rdbuf();
        source = buffer.str();
        break;
    }
#endif

    if (source.empty()) {
        LOGE("WindmillScene: unable to open %s", displayPath.c_str());
        return false;
    }
    LOGI("WindmillScene: opened %s", displayPath.c_str());

    tinyobj::ObjReaderConfig config;
    config.triangulate = true;
    config.vertex_color = false;

    tinyobj::ObjReader reader;
    if (!reader.ParseFromString(source, std::string(), config)) {
        if (!reader.Error().empty()) {
            LOGE("TinyObjLoaderHelper: %s", reader.Error().c_str());
        }
        return false;
    }
    if (!reader.Warning().empty()) {
        LOGI("TinyObjLoaderHelper: %s", reader.Warning().c_str());
    }

    const tinyobj::attrib_t& attributes = reader.GetAttrib();
    const std::vector<tinyobj::shape_t>& shapes = reader.GetShapes();
    const size_t positionCount = attributes.vertices.size() / 3;
    if (positionCount == 0 || shapes.empty()) return false;

    bool needsSmoothNormals = attributes.normals.empty();
    for (const tinyobj::shape_t& shape : shapes) {
        for (const tinyobj::index_t& index : shape.mesh.indices) {
            if (index.normal_index < 0) needsSmoothNormals = true;
        }
    }

    std::vector<glm::vec3> smoothNormals(positionCount, glm::vec3(0.0f));
    if (needsSmoothNormals) {
        for (const tinyobj::shape_t& shape : shapes) {
            const std::vector<tinyobj::index_t>& objIndices = shape.mesh.indices;
            for (size_t i = 0; i + 2 < objIndices.size(); i += 3) {
                const int ia = objIndices[i + 0].vertex_index;
                const int ib = objIndices[i + 1].vertex_index;
                const int ic = objIndices[i + 2].vertex_index;
                if (ia < 0 || ib < 0 || ic < 0) continue;

                const glm::vec3 a(
                    attributes.vertices[ia * 3 + 0],
                    attributes.vertices[ia * 3 + 1],
                    attributes.vertices[ia * 3 + 2]);
                const glm::vec3 b(
                    attributes.vertices[ib * 3 + 0],
                    attributes.vertices[ib * 3 + 1],
                    attributes.vertices[ib * 3 + 2]);
                const glm::vec3 c(
                    attributes.vertices[ic * 3 + 0],
                    attributes.vertices[ic * 3 + 1],
                    attributes.vertices[ic * 3 + 2]);
                const glm::vec3 faceNormal = glm::cross(b - a, c - a);
                smoothNormals[ia] += faceNormal;
                smoothNormals[ib] += faceNormal;
                smoothNormals[ic] += faceNormal;
            }
        }
        for (glm::vec3& normal : smoothNormals) {
            const float lengthSquared = glm::dot(normal, normal);
            normal = lengthSquared > 1.0e-8f
                ? glm::normalize(normal)
                : glm::vec3(0.0f, 1.0f, 0.0f);
        }
        LOGI("TinyObjLoaderHelper: no vn in file - computed %zu smooth normals",
             smoothNormals.size());
    }

    std::vector<GLfloat> vertices;
    std::vector<GLushort> indices;
    for (const tinyobj::shape_t& shape : shapes) {
        for (const tinyobj::index_t& index : shape.mesh.indices) {
            if (index.vertex_index < 0 ||
                static_cast<size_t>(index.vertex_index) >= positionCount ||
                indices.size() >= std::numeric_limits<GLushort>::max()) {
                LOGE("TinyObjLoaderHelper: invalid or oversized OBJ index");
                return false;
            }

            const size_t positionOffset =
                static_cast<size_t>(index.vertex_index) * 3;
            glm::vec3 normal;
            if (index.normal_index >= 0 &&
                static_cast<size_t>(index.normal_index * 3 + 2) <
                    attributes.normals.size()) {
                const size_t normalOffset =
                    static_cast<size_t>(index.normal_index) * 3;
                normal = glm::vec3(
                    attributes.normals[normalOffset + 0],
                    attributes.normals[normalOffset + 1],
                    attributes.normals[normalOffset + 2]);
            } else {
                normal = smoothNormals[index.vertex_index];
            }

            // The supplied Blender primitives span -1..1. Normalize them to
            // the -0.5..0.5 convention used by the existing hierarchy.
            AddVertex(
                vertices,
                attributes.vertices[positionOffset + 0] * 0.5f,
                attributes.vertices[positionOffset + 1] * 0.5f,
                attributes.vertices[positionOffset + 2] * 0.5f,
                normal.x, normal.y, normal.z);
            indices.push_back(static_cast<GLushort>(indices.size()));
        }
    }

    if (vertices.empty() || indices.empty()) return false;
    UploadMesh(mesh, vertices, indices);
    LOGI("WindmillScene: %s loaded via TinyObjLoaderHelper",
         displayPath.c_str());
    return true;
}
void Fan::CreateCubeMesh()
{
    const GLfloat data[][6] = {
        {-.5f,-.5f, .5f, 0, 0, 1}, { .5f,-.5f, .5f, 0, 0, 1},
        { .5f, .5f, .5f, 0, 0, 1}, {-.5f, .5f, .5f, 0, 0, 1},
        { .5f,-.5f,-.5f, 0, 0,-1}, {-.5f,-.5f,-.5f, 0, 0,-1},
        {-.5f, .5f,-.5f, 0, 0,-1}, { .5f, .5f,-.5f, 0, 0,-1},
        {-.5f,-.5f,-.5f,-1, 0, 0}, {-.5f,-.5f, .5f,-1, 0, 0},
        {-.5f, .5f, .5f,-1, 0, 0}, {-.5f, .5f,-.5f,-1, 0, 0},
        { .5f,-.5f, .5f, 1, 0, 0}, { .5f,-.5f,-.5f, 1, 0, 0},
        { .5f, .5f,-.5f, 1, 0, 0}, { .5f, .5f, .5f, 1, 0, 0},
        {-.5f, .5f, .5f, 0, 1, 0}, { .5f, .5f, .5f, 0, 1, 0},
        { .5f, .5f,-.5f, 0, 1, 0}, {-.5f, .5f,-.5f, 0, 1, 0},
        {-.5f,-.5f,-.5f, 0,-1, 0}, { .5f,-.5f,-.5f, 0,-1, 0},
        { .5f,-.5f, .5f, 0,-1, 0}, {-.5f,-.5f, .5f, 0,-1, 0}
    };
    const GLushort indexData[] = {
         0, 1, 2,  0, 2, 3,   4, 5, 6,  4, 6, 7,
         8, 9,10,  8,10,11,  12,13,14, 12,14,15,
        16,17,18, 16,18,19,  20,21,22, 20,22,23
    };
    std::vector<GLfloat> vertices(&data[0][0], &data[0][0] + 24 * 6);
    std::vector<GLushort> indices(indexData, indexData + 36);
    UploadMesh(cube, vertices, indices);
}

void Fan::CreateCylinderMesh()
{
    constexpr int slices = 32;
    std::vector<GLfloat> vertices;
    std::vector<GLushort> indices;

    // Side wall.
    for (int i = 0; i <= slices; ++i) {
        const float angle = 2.0f * kPi * static_cast<float>(i) / slices;
        const float x = 0.5f * std::cos(angle);
        const float z = 0.5f * std::sin(angle);
        const float nx = std::cos(angle);
        const float nz = std::sin(angle);
        AddVertex(vertices, x, -0.5f, z, nx, 0.0f, nz);
        AddVertex(vertices, x,  0.5f, z, nx, 0.0f, nz);
    }
    for (int i = 0; i < slices; ++i) {
        const GLushort a = static_cast<GLushort>(i * 2);
        const GLushort b = static_cast<GLushort>(a + 1);
        const GLushort c = static_cast<GLushort>(a + 2);
        const GLushort d = static_cast<GLushort>(a + 3);
        indices.insert(indices.end(), {a, b, c, b, d, c});
    }

    // Top cap.
    GLushort center = static_cast<GLushort>(vertices.size() / 6);
    AddVertex(vertices, 0.0f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f);
    GLushort rim = static_cast<GLushort>(vertices.size() / 6);
    for (int i = 0; i <= slices; ++i) {
        const float angle = 2.0f * kPi * static_cast<float>(i) / slices;
        AddVertex(vertices, 0.5f * std::cos(angle), 0.5f,
                  0.5f * std::sin(angle), 0.0f, 1.0f, 0.0f);
    }
    for (int i = 0; i < slices; ++i) {
        indices.insert(indices.end(), {
            center, static_cast<GLushort>(rim + i + 1),
            static_cast<GLushort>(rim + i)});
    }

    // Bottom cap.
    center = static_cast<GLushort>(vertices.size() / 6);
    AddVertex(vertices, 0.0f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f);
    rim = static_cast<GLushort>(vertices.size() / 6);
    for (int i = 0; i <= slices; ++i) {
        const float angle = 2.0f * kPi * static_cast<float>(i) / slices;
        AddVertex(vertices, 0.5f * std::cos(angle), -0.5f,
                  0.5f * std::sin(angle), 0.0f, -1.0f, 0.0f);
    }
    for (int i = 0; i < slices; ++i) {
        indices.insert(indices.end(), {
            center, static_cast<GLushort>(rim + i),
            static_cast<GLushort>(rim + i + 1)});
    }

    UploadMesh(cylinder, vertices, indices);
}

void Fan::CreateSphereMesh()
{
    constexpr int stacks = 16;
    constexpr int slices = 32;
    std::vector<GLfloat> vertices;
    std::vector<GLushort> indices;

    for (int stack = 0; stack <= stacks; ++stack) {
        const float phi = kPi * static_cast<float>(stack) / stacks;
        const float y = 0.5f * std::cos(phi);
        const float ring = 0.5f * std::sin(phi);
        for (int slice = 0; slice <= slices; ++slice) {
            const float theta = 2.0f * kPi * static_cast<float>(slice) / slices;
            const float x = ring * std::cos(theta);
            const float z = ring * std::sin(theta);
            const glm::vec3 normal = glm::normalize(glm::vec3(x, y, z));
            AddVertex(vertices, x, y, z, normal.x, normal.y, normal.z);
        }
    }

    for (int stack = 0; stack < stacks; ++stack) {
        for (int slice = 0; slice < slices; ++slice) {
            const GLushort a =
                static_cast<GLushort>(stack * (slices + 1) + slice);
            const GLushort b = static_cast<GLushort>(a + slices + 1);
            indices.insert(indices.end(), {
                a, static_cast<GLushort>(a + 1), b,
                static_cast<GLushort>(a + 1),
                static_cast<GLushort>(b + 1), b});
        }
    }

    UploadMesh(sphere, vertices, indices);
}

void Fan::InitModel()
{
#ifdef PLATFORM_ANDROID
    program = ShaderHelper::buildProgramFromAssets(
        assetManager, "shader/FanVertex.glsl", "shader/FanFragment.glsl");
#else
    program = ShaderHelper::buildProgramFromFile(
        "FanVertex.glsl", "FanFragment.glsl");
#endif
    if (!program) {
        LOGE("Fan: failed to build Phong shader program");
        return;
    }

    mvpLocation = glGetUniformLocation(program, "MODELVIEWPROJECTIONMATRIX");
    modelViewLocation = glGetUniformLocation(program, "MODELVIEWMATRIX");
    normalLocation = glGetUniformLocation(program, "NORMALMATRIX");
    partColorLocation = glGetUniformLocation(program, "PARTCOLOR");
    lightPositionLocation = glGetUniformLocation(program, "LIGHTPOSITIONVIEW");

    if (mvpLocation < 0 || modelViewLocation < 0 || normalLocation < 0 ||
        partColorLocation < 0 || lightPositionLocation < 0) {
        LOGE("Fan: required Phong uniforms are missing");
        glDeleteProgram(program);
        program = 0;
        return;
    }

    if (!LoadObjMesh("CubeWithNormal.obj", cube)) {
        CreateCubeMesh();
    }
    if (!LoadObjMesh("SemiHollowCylinder.obj", cylinder)) {
        CreateCylinderMesh();
    }
    if (!LoadObjMesh("IsoSphere.obj", sphere)) {
        CreateSphereMesh();
    }
    transform.TransformInit();
}

void Fan::SetViewProjection(const glm::mat4& view, const glm::mat4& projection)
{
    viewMatrix = view;
    projectionMatrix = projection;
}

void Fan::SetSpeed(float newSpeed)
{
    speed = std::clamp(newSpeed, 0.0f, 20.0f);
}

void Fan::DrawMesh(const Mesh& mesh, const glm::vec3& colour, int partId)
{
    const glm::mat4 model = *transform.TransformGetModelMatrix();
    if (partId >= 0 && partId < kPartCount) {
        partMatrices[partId] = model;
        partMatrixValid[partId] = true;
    }

    glm::mat4* mvp = transform.TransformGetModelViewProjectionMatrix();
    glm::mat4* modelView = transform.TransformGetModelViewMatrix();
    glm::mat3 normalMatrix;
    transform.TransformGetNormalMatrix(&normalMatrix);

    const bool highlighted = partId >= 0 && partId < kPartCount &&
        (highlightedPartMask.load(std::memory_order_relaxed) &
         (1u << partId)) != 0;
    const glm::vec3 finalColour = highlighted ? kHighlightColour : colour;

    glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(*mvp));
    glUniformMatrix4fv(modelViewLocation, 1, GL_FALSE, glm::value_ptr(*modelView));
    glUniformMatrix3fv(normalLocation, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    glUniform3fv(partColorLocation, 1, glm::value_ptr(finalColour));
    glBindVertexArray(mesh.vao);
    glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_SHORT, nullptr);
}

void Fan::Render()
{
    if (!program || !cube.vao || !cylinder.vao || !sphere.vao) return;

    spinAngle = std::fmod(spinAngle + speed * 0.2f, 360.0f);
    assemblyAngle = std::fmod(assemblyAngle + 0.25f, 360.0f);
    partMatrixValid.fill(false);

    glUseProgram(program);

    transform.TransformSetMatrixMode(VIEW_MATRIX);
    transform.TransformLoadMatrix(&viewMatrix);
    transform.TransformSetMatrixMode(PROJECTION_MATRIX);
    transform.TransformLoadMatrix(&projectionMatrix);
    transform.TransformSetMatrixMode(MODEL_MATRIX);
    transform.TransformLoadIdentity();
    // Rotate the complete fan around the pole while the blades spin independently.
    transform.TransformTranslate(0.42f, 0.0f, 0.0f);
    transform.TransformRotate(glm::radians(assemblyAngle), 0.0f, 1.0f, 0.0f);
    transform.TransformTranslate(-0.42f, 0.0f, 0.0f);

    const glm::vec3 lightWorld(4.0f, 7.0f, 6.0f);
    const glm::vec3 lightView =
        glm::vec3(viewMatrix * glm::vec4(lightWorld, 1.0f));
    glUniform3fv(lightPositionLocation, 1, glm::value_ptr(lightView));

    // Light gray pedestal, matching the supplied nearest/farthest screenshots.
    transform.TransformPushMatrix();
    transform.TransformTranslate(0.42f, -1.80f, 0.0f);
    transform.TransformScale(1.35f, 0.25f, 0.85f);
    DrawMesh(cube, glm::vec3(0.58f, 0.60f, 0.64f), kBasePart);
    transform.TransformPopMatrix();

    // Silver cylindrical support.
    transform.TransformPushMatrix();
    transform.TransformTranslate(0.42f, -0.65f, 0.0f);
    transform.TransformScale(0.32f, 2.30f, 0.32f);
    DrawMesh(cylinder, glm::vec3(0.55f, 0.57f, 0.62f), kPolePart);
    transform.TransformPopMatrix();

    // The motor/axle runs along Z, perpendicular to the blade disc.
    // Its rear end sits over the pole and its front end meets the hub.
    transform.TransformPushMatrix();
    transform.TransformTranslate(0.42f, 0.48f, 0.35f);
    transform.TransformScale(0.46f, 0.46f, 1.15f);
    DrawMesh(cube, glm::vec3(0.52f, 0.54f, 0.58f), -1);
    transform.TransformPopMatrix();

    // Twenty thin orange-brown blades form the reference's dense radial fan.
    for (int i = 0; i < kBladeCount; ++i) {
        transform.TransformPushMatrix();
        transform.TransformTranslate(0.42f, 0.55f, 0.82f);
        transform.TransformRotate(
            glm::radians(spinAngle + i * (360.0f / kBladeCount)),
            0.0f, 0.0f, 1.0f);
        transform.TransformTranslate(0.0f, 0.82f, 0.0f);
        transform.TransformScale(0.16f, 1.30f, 0.09f);
        DrawMesh(cube, glm::vec3(0.72f, 0.28f, 0.07f),
                 kFirstBladePart + i);
        transform.TransformPopMatrix();
    }

    // Rounded gold hub sits in front of the blades, as in the screenshots.
    transform.TransformPushMatrix();
    transform.TransformTranslate(0.42f, 0.55f, 1.00f);
    transform.TransformScale(0.55f, 0.55f, 0.46f);
    DrawMesh(sphere, glm::vec3(0.72f, 0.45f, 0.02f), kHubPart);
    transform.TransformPopMatrix();

    glBindVertexArray(0);
    glUseProgram(0);
}

void Fan::Resize(int, int)
{
    // Scene3D owns the shared perspective projection.
}

bool Fan::RayHitsPart(const glm::vec3& origin, const glm::vec3& direction,
                      const glm::mat4& partMatrix, float& distance) const
{
    const glm::mat4 inverseModel = glm::inverse(partMatrix);
    const glm::vec3 localOrigin =
        glm::vec3(inverseModel * glm::vec4(origin, 1.0f));
    const glm::vec3 localDirection =
        glm::vec3(inverseModel * glm::vec4(direction, 0.0f));

    float tMin = -std::numeric_limits<float>::infinity();
    float tMax = std::numeric_limits<float>::infinity();

    for (int axis = 0; axis < 3; ++axis) {
        const float o = localOrigin[axis];
        const float d = localDirection[axis];

        if (std::abs(d) < 1.0e-6f) {
            if (o < -0.5f || o > 0.5f) return false;
            continue;
        }

        float t1 = (-0.5f - o) / d;
        float t2 = ( 0.5f - o) / d;
        if (t1 > t2) std::swap(t1, t2);
        tMin = std::max(tMin, t1);
        tMax = std::min(tMax, t2);
        if (tMin > tMax) return false;
    }

    const float hitT = (tMin >= 0.0f) ? tMin : tMax;
    if (hitT < 0.0f) return false;

    const glm::vec3 worldHit = origin + direction * hitT;
    distance = glm::length(worldHit - origin);
    return true;
}

bool Fan::Pick(const glm::vec3& rayOrigin, const glm::vec3& rayDirection)
{
    int nearestPart = -1;
    float nearestDistance = std::numeric_limits<float>::max();

    for (int part = 0; part < kPartCount; ++part) {
        if (!partMatrixValid[part]) continue;
        float distance = 0.0f;
        if (RayHitsPart(rayOrigin, rayDirection, partMatrices[part], distance) &&
            distance < nearestDistance) {
            nearestDistance = distance;
            nearestPart = part;
        }
    }

    // The pole can occlude parts behind it, but it never highlights.
    if (nearestPart < 0 || nearestPart == kPolePart) return false;

    const unsigned int partBit = 1u << nearestPart;
    const unsigned int previousMask = highlightedPartMask.fetch_xor(
        partBit, std::memory_order_relaxed);
    const bool nowHighlighted = (previousMask & partBit) == 0;

    LogPickState(nearestPart, nowHighlighted);
    return true;
}
