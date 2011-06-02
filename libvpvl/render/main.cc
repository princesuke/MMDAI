#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <gl/gl.h>
#include <gl/glu.h>
#endif

#include <SDL.h>
#include <SDL_image.h>

#include <vpvl/vpvl.h>
#include <vpvl/internal/PMDModel.h>

struct vpvl::MaterialPrivate {
    GLuint primaryTextureID;
    GLuint secondTextureID;
};

struct vpvl::PMDModelPrivate {
    GLuint toonTextureID[vpvl::PMDModel::kSystemTextureMax];
    bool hasSingleSphereMap;
    bool hasMultipleSphereMap;
};

static const int g_width = 800;
static const int g_height = 600;

static const char *g_sysdir = "render/res/system";
static const char *g_modeldir = "render/res/lat";
static const char *g_modelname = "normal.pmd";

static bool InitializeSurface(SDL_Surface *&surface, int width, int height)
{
    SDL_WM_SetCaption("libvpvl render testing program", NULL);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    const SDL_VideoInfo *info = SDL_GetVideoInfo();
    if (!info) {
        fprintf(stderr, "Unable to get video info: %s", SDL_GetError());
        return false;
    }
    if ((surface = SDL_SetVideoMode(width, height, info->vfmt->BitsPerPixel, SDL_OPENGL)) == NULL) {
        fprintf(stderr, "Unable to init surface: %s", SDL_GetError());
        return false;
    }
    glClearStencil(0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GEQUAL, 0.05f);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
    return true;
}

static bool LoadTexture(const char *path, GLuint &textureID)
{
    static const GLfloat priority = 1.0f;
    SDL_Surface *surface = IMG_Load(path);
    if (surface) {
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        GLenum format, internal;
        if (surface->format->BitsPerPixel == 32) {
            format = GL_BGRA;
            internal = GL_RGBA8;
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        }
        else if (surface->format->BitsPerPixel == 24) {
            format = GL_BGR;
            internal = GL_RGB8;
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        }
        else {
            fprintf(stderr, "unknown image format: %s\n", path);
            SDL_FreeSurface(surface);
            return false;
        }
        SDL_LockSurface(surface);
        glTexImage2D(GL_TEXTURE_2D, 0, internal, surface->w, surface->h, 0, format, GL_UNSIGNED_BYTE, surface->pixels);
        SDL_UnlockSurface(surface);
        SDL_FreeSurface(surface);
        glPrioritizeTextures(1, &textureID, &priority);
        glBindTexture(GL_TEXTURE_2D, 0);
        return true;
    }
    else {
        fprintf(stderr, "failed loading %s: %s\n", path, IMG_GetError());
        return false;
    }
}

static bool LoadToonTexture(const char *system, const char *dir, const char *name, GLuint &textureID)
{
    char path[256];
    struct stat sb;
    snprintf(path, sizeof(path), "%s/%s", dir, name);
    if (!(stat(path, &sb) != -1 && S_ISREG(sb.st_mode))) {
        snprintf(path, sizeof(path), "%s/%s", system, name);
        if (!(stat(path, &sb) != -1 && S_ISREG(sb.st_mode))) {
            fprintf(stderr, "%s is not found, skipped...\n", path);
            return false;
        }
    }
    //fprintf(stderr, "%s\n", path);
    return LoadTexture(path, textureID);
}

static void LoadModelTextures(vpvl::PMDModel &model, const char *system, const char *dir)
{
    const vpvl::MaterialList materials = model.materials();
    const uint32_t nMaterials = materials.size();
    char path[256];
    GLuint textureID = 0;
    vpvl::PMDModelPrivate *modelPrivate = new vpvl::PMDModelPrivate;
    bool hasSingleSphere = false, hasMultipleSphere = false;
    for (uint32_t i = 0; i <nMaterials; i++) {
        vpvl::Material *material = materials[i];
        const char *primary = material->primaryTextureName();
        const char *second = material->secondTextureName();
        vpvl::MaterialPrivate *materialPrivate = new vpvl::MaterialPrivate;
        materialPrivate->primaryTextureID = 0;
        materialPrivate->secondTextureID = 0;
        if (*primary) {
            snprintf(path, sizeof(path), "%s/%s", dir, primary);
            if (LoadTexture(path, textureID))
                materialPrivate->primaryTextureID = textureID;
        }
        if (*second) {
            snprintf(path, sizeof(path), "%s/%s", dir, second);
            if (LoadTexture(path, textureID))
                materialPrivate->secondTextureID = textureID;
        }
        hasSingleSphere |= material->isSpherePrimary() && !material->isSphereAuxSecond();
        hasMultipleSphere |= material->isSphereAuxSecond();
        material->setPrivateData(materialPrivate);
    }
    modelPrivate->hasSingleSphereMap = hasSingleSphere;
    modelPrivate->hasMultipleSphereMap = hasMultipleSphere;
    if (LoadToonTexture(system, dir, "toon0.bmp", textureID))
        modelPrivate->toonTextureID[0] = textureID;
    for (uint32_t i = 0; i < vpvl::PMDModel::kSystemTextureMax - 1; i++) {
        const char *name = model.toonTexture(i);
        if (LoadToonTexture(system, dir, name, textureID))
            modelPrivate->toonTextureID[i + 1] = textureID;
    }
    model.setPrivateData(modelPrivate);
}

static void UnloadModelTextures(const vpvl::PMDModel &model)
{
    const vpvl::MaterialList materials = model.materials();
    const uint32_t nMaterials = materials.size();
    for (uint32_t i = 0; i < nMaterials; i++) {
        vpvl::MaterialPrivate *data = materials[i]->privateData();
        glDeleteTextures(1, &data->primaryTextureID);
        glDeleteTextures(1, &data->secondTextureID);
        delete data;
    }
    vpvl::PMDModelPrivate *data = model.privateData();
    glDeleteTextures(vpvl::PMDModel::kSystemTextureMax, data->toonTextureID);
    delete data;
}

static void SetLighting(vpvl::PMDModel &model)
{
    btVector3 color(1.0f, 1.0f, 1.0f), direction(0.5f, 1.0f, 0.5f);
    btScalar diffuseValue, ambientValue, specularValue, lightIntensity = 0.6;

    // use MMD like cartoon
    diffuseValue = 0.2f;
    ambientValue = lightIntensity * 2.0f;
    specularValue = 0.4f;

    btVector3 diffuse = color * diffuseValue;
    btVector3 ambient = color * ambientValue;
    btVector3 specular = color * specularValue;
    diffuse.setW(1.0f);
    ambient.setW(1.0f);
    specular.setW(1.0f);

    glLightfv(GL_LIGHT0, GL_POSITION, static_cast<const btScalar *>(direction));
    glLightfv(GL_LIGHT0, GL_DIFFUSE, static_cast<const btScalar *>(diffuse));
    glLightfv(GL_LIGHT0, GL_AMBIENT, static_cast<const btScalar *>(ambient));
    glLightfv(GL_LIGHT0, GL_SPECULAR, static_cast<const btScalar *>(specular));
    model.setLightDirection(direction);
}

static void DrawModel(const vpvl::PMDModel &model)
{
#ifndef VPVL_COORDINATE_OPENGL
    glPushMatrix();
    glScalef(1.0f, 1.0f, -1.0f);
    glCullFace(GL_FRONT);
#endif

    glActiveTexture(GL_TEXTURE0);
    glClientActiveTexture(GL_TEXTURE0);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    size_t stride = model.stride();
    glVertexPointer(3, GL_FLOAT, stride, model.verticesPointer());
    glNormalPointer(GL_FLOAT, stride, model.normalsPointer());
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, stride, model.textureCoordsPointer());
    const bool enableToon = true;
    // toon
    if (enableToon) {
        glActiveTexture(GL_TEXTURE1);
        glEnable(GL_TEXTURE_2D);
        glClientActiveTexture(GL_TEXTURE1);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        // shadow map
        if (false)
            glTexCoordPointer(2, GL_FLOAT, 0, 0);
        else
            glTexCoordPointer(2, GL_FLOAT, sizeof(btVector3), model.toonTextureCoordsPointer());
        glActiveTexture(GL_TEXTURE0);
        glClientActiveTexture(GL_TEXTURE0);
    }
    const vpvl::PMDModelPrivate *modelPrivate = model.privateData();
    bool hasSingleSphereMap = false, hasMultipleSphereMap = false;
    // first sphere map
    if (modelPrivate->hasSingleSphereMap) {
        glEnable(GL_TEXTURE_2D);
        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
        glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
        glDisable(GL_TEXTURE_2D);
        hasSingleSphereMap = true;
    }
    // second sphere map
    if (modelPrivate->hasMultipleSphereMap) {
        glActiveTexture(GL_TEXTURE2);
        glEnable(GL_TEXTURE_2D);
        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
        glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
        glDisable(GL_TEXTURE_2D);
        glActiveTexture(GL_TEXTURE0);
        hasMultipleSphereMap = true;
    }
    const vpvl::MaterialList materials = model.materials();
    const uint32_t nMaterials = materials.size();
    btVector4 average, ambient, diffuse, specular;
    uint16_t *indicesPtr = const_cast<uint16_t *>(model.indicesPointer());
    for (uint32_t i = 0; i <nMaterials; i++) {
        const vpvl::Material *material = materials[i];
        const vpvl::MaterialPrivate *materialPrivate = material->privateData();
        // toon
        const float alpha = material->alpha();
        if (enableToon) {
            average = material->averageColor();
            average.setW(average.w() * alpha);
            specular = material->specular();
            specular.setW(specular.w() * alpha);
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, static_cast<const GLfloat *>(average));
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, static_cast<const GLfloat *>(specular));
        }
        else {
            ambient = material->ambient();
            ambient.setW(ambient.w() * alpha);
            diffuse = material->diffuse();
            diffuse.setW(diffuse.w() * alpha);
            specular = material->specular();
            specular.setW(specular.w() * alpha);
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, static_cast<const GLfloat *>(ambient));
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, static_cast<const GLfloat *>(diffuse));
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, static_cast<const GLfloat *>(specular));
        }
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material->shiness());
        material->alpha() < 1.0f ? glDisable(GL_CULL_FACE) : glEnable(GL_CULL_FACE);
        glActiveTexture(GL_TEXTURE0);
        // has texture
        if (materialPrivate->primaryTextureID > 0) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, materialPrivate->primaryTextureID);
            if (hasSingleSphereMap) {
                // is sphere map
                if (material->isSpherePrimary() || material->isSphereAuxPrimary()) {
                    // is second sphere map
                    if (material->isSphereAuxPrimary())
                        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
                    glEnable(GL_TEXTURE_GEN_S);
                    glEnable(GL_TEXTURE_GEN_T);
                }
                else {
                    glDisable(GL_TEXTURE_GEN_S);
                    glDisable(GL_TEXTURE_GEN_T);
                }
            }
        }
        else {
            glDisable(GL_TEXTURE_2D);
        }
        // toon
        if (enableToon) {
            const GLuint textureID = modelPrivate->toonTextureID[material->toonID()];
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        if (hasMultipleSphereMap) {
            // second sphere
            glActiveTexture(GL_TEXTURE2);
            glEnable(GL_TEXTURE_2D);
            if (materialPrivate->secondTextureID > 0) {
                // is second sphere
                if (material->isSphereAuxSecond())
                    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
                else
                    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
                glBindTexture(GL_TEXTURE_2D, materialPrivate->secondTextureID);
                glEnable(GL_TEXTURE_GEN_S);
                glEnable(GL_TEXTURE_GEN_T);
            }
            else {
                glBindTexture(GL_TEXTURE_2D, 0);
            }
        }
        // draw
        const uint32_t nIndices = material->countIndices();
        // memo: 16: 18:eye 21 23
        //if (i != 23)
        glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_SHORT, indicesPtr);
        indicesPtr += nIndices;
        // is aux sphere map
        if (material->isSphereAuxPrimary()) {
            glActiveTexture(GL_TEXTURE0);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        }
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    // toon
    if (enableToon) {
        glClientActiveTexture(GL_TEXTURE0);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        // first sphere map
        if (hasSingleSphereMap) {
            glActiveTexture(GL_TEXTURE0);
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
        }
        glClientActiveTexture(GL_TEXTURE1);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        // second sphere map
        if (hasMultipleSphereMap) {
            glActiveTexture(GL_TEXTURE2);
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
        }
    }
    else {
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        // first sphere map
        if (hasSingleSphereMap) {
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
        }
        // second sphere map
        if (hasMultipleSphereMap) {
            glActiveTexture(GL_TEXTURE2);
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
        }
    }
    glActiveTexture(GL_TEXTURE0);
    // first or second sphere map
    if (hasSingleSphereMap || hasMultipleSphereMap) {
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);
    }
    // toon
    if (enableToon) {
        glActiveTexture(GL_TEXTURE1);
        glDisable(GL_TEXTURE_2D);
    }
    // second sphere map
    if (hasMultipleSphereMap) {
        glActiveTexture(GL_TEXTURE2);
        glDisable(GL_TEXTURE_2D);
    }
    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);

#ifndef VPVL_COORDINATE_OPENGL
    glPopMatrix();
    glCullFace(GL_BACK);
#endif
}

static void DrawModelEdge(const vpvl::PMDModel &model)
{
#ifdef VPVL_COORDINATE_OPENGL
    glCullFace(GL_FRONT);
#else
    glPushMatrix();
    glScalef(1.0f, 1.0f, -1.0f);
    glCullFace(GL_BACK);
#endif

    const float alpha = 1.0f;
    const btVector4 color(0.0f, 0.0f, 0.0f, alpha);

    glDisable(GL_LIGHTING);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(btVector3), model.edgeVerticesPointer());
    glColor4fv(static_cast<const btScalar *>(color));
    glDrawElements(GL_TRIANGLES, model.edgeIndicesCount(), GL_UNSIGNED_SHORT, model.edgeIndicesPointer());
    glDisableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_LIGHTING);

#ifdef VPVL_COORDINATE_OPENGL
    glCullFace(GL_BACK);
#else
    glPopMatrix();
    glCullFace(GL_FRONT);
#endif
}

static void DrawModelShadow(const vpvl::PMDModel &model)
{
    glDisable(GL_CULL_FACE);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, model.stride(), model.verticesPointer());
    glDrawElements(GL_TRIANGLES, model.indices().size(), GL_UNSIGNED_SHORT, model.indicesPointer());
    glDisableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_CULL_FACE);
}

static void DrawSurface(vpvl::PMDModel &model, int width, int height)
{
    model.updateRootBone();
    model.updateMotion();
    model.updateSkins();
    const double ratio = static_cast<double>(width) / height;
    float matrix[16];
    btTransform mv;
    btQuaternion q(0.0f, 0.0f, 0.0f, 1.0f);
    q.setEulerZYX(0.0f, 0.0f, 0.0f);
    mv.setIdentity();
    mv.setRotation(q);
    mv.setOrigin(mv * -btVector3(0.0f, 15.0f, 0.0f) - btVector3(0.0f, 0.0f, 30.0f));
    mv.getOpenGLMatrix(matrix);
    // initialize
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(16.0, ratio, 0.5, 8000.0);
    glMatrixMode(GL_MODELVIEW);
    glClearColor(0.0f, 0.0f, 0.25f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    // initialize matrices
    glLoadMatrixf(matrix);
    // initialize rendering states
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 1, ~0);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
    glColorMask(0, 0, 0, 0);
    glDepthMask(0);
    glDisable(GL_DEPTH_TEST);
    glPushMatrix();
    // render shadow
    DrawModelShadow(model);
    glPopMatrix();
    glColorMask(1, 1, 1, 1);
    glDepthMask(1);
    glStencilFunc(GL_EQUAL, 2, ~0);
    glDisable(GL_STENCIL_TEST);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    // render model and edge
    DrawModel(model);
    DrawModelEdge(model);
    SDL_GL_SwapBuffers();
}

static bool PollEvents()
{
    SDL_Event event;
    SDLKey key;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            return true;
        case SDL_KEYDOWN:
            key = event.key.keysym.sym;
            if (key == SDLK_ESCAPE || key == SDLK_q)
                return true;
            break;
        default:
            break;
        }
    }
    return false;
}

static void FileSlurp(const char *path, char *&data, size_t &size) {
    FILE *fp = fopen(path, "rb");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        data = new char[size];
        fread(data, size, 1, fp);
        fclose(fp);
    }
    else {
        fprintf(stderr, "failed loading the model: %s\n", strerror(errno));
    }
}

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
        return -1;
    }
    atexit(SDL_Quit);
    if (IMG_Init(IMG_INIT_JPG|IMG_INIT_PNG|IMG_INIT_TIF) < 0) {
        fprintf(stderr, "Unable to init SDL_image: %s\n", IMG_GetError());
        return -1;
    }
    atexit(IMG_Quit);

    SDL_Surface *surface;
    char *data = 0, path[256];
    size_t size;
    snprintf(path, sizeof(path), "%s/%s", g_modeldir, g_modelname);
    FileSlurp(path, data, size);
    vpvl::PMDModel model(data, size);
    if (!InitializeSurface(surface, g_width, g_height)) {
        delete data;
        return -1;
    }

    if (!model.parse()) {
        fprintf(stderr, "failed parsing the model\n");
        delete data;
        return -1;
    }
    SetLighting(model);
    LoadModelTextures(model, g_sysdir, g_modeldir);

    while (true) {
        if (PollEvents())
            break;
        DrawSurface(model, g_width, g_height);
    }

    UnloadModelTextures(model);
    SDL_FreeSurface(surface);
    delete data;

    return 0;
}
