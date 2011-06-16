/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2011  hkrn                                    */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAI project team nor the names of     */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#ifndef VPVL_XMODEL_H_
#define VPVL_XMODEL_H_

#include "vpvl/PMDModel.h"
#include "vpvl/common.h"

namespace vpvl
{

typedef struct XModelUserData XModelUserData;
typedef btAlignedObjectArray<uint16_t> XModelIndexList;

class XModelMaterial {
public:
    XModelMaterial()
        : m_color(0.0f, 0.0f, 0.0f, 0.0f),
          m_specular(0.0f, 0.0f, 0.0f, 0.0f),
          m_emmisive(0.0f, 0.0f, 0.0f, 0.0f),
          m_textureName(0),
          m_power(0.0f)
    {
    }
    XModelMaterial(const btVector4 &color,
                   const btVector4 &specular,
                   btVector4 &emmisive,
                   char *textureName,
                   float power)
        : m_color(color),
          m_specular(specular),
          m_emmisive(emmisive),
          m_textureName(textureName),
          m_power(power)
    {
    }
    ~XModelMaterial() {
        m_color.setZero();
        m_specular.setZero();
        m_emmisive.setZero();
        m_textureName = 0;
        m_power = 0;
    }

    const btVector4 &color() const { return m_color; }
    const btVector4 &specular() const { return m_specular; }
    const btVector4 &emmisive() const { return m_emmisive; }
    const char *textureName() const { return m_textureName; }
    float power() const { return m_power; }

private:
    btVector4 m_color;
    btVector4 m_specular;
    btVector4 m_emmisive;
    char *m_textureName;
    float m_power;
};

/**
 * @file
 * @author hkrn
 *
 * @section DESCRIPTION
 *
 * XModel class represents a DirectX model.
 */

class XModel
{
public:
    enum StrideType
    {
        kVerticesStride,
        kNormalsStride,
        kTextureCoordsStride,
        kColorsStride,
        kIndicesStride
    };

    XModel(const uint8_t *data, size_t size);
    ~XModel();

    bool preparse();
    bool load();

    size_t stride(StrideType type) const;
    const void *verticesPointer() const;
    const void *normalsPointer() const;
    const void *textureCoordsPointer() const;
    const void *colorsPointer() const;

    const btAlignedObjectArray<btVector3> &vertices() const {
        return m_vertices;
    }
    const btAlignedObjectArray<btVector3> &normals() const {
        return m_normals;
    }
    const btAlignedObjectArray<btVector3> &textureCoords() const {
        return m_coords;
    }
    const btAlignedObjectArray<btVector4> &colors() const {
        return m_colors;
    }
    const XModelIndexList *indicesAt(uint32_t value) const {
        return m_indices[value];
    }
    const XModelMaterial *materialAt(uint32_t value) const {
        return m_materials[value];
    }
    uint32_t countMatreials() const {
        return m_materials.size() - 1;
    }
    XModelUserData *userData() const {
        return m_userData;
    }
    const uint8_t *data() const {
        return m_data;
    }
    size_t size() const {
        return m_size;
    }

    void setUserData(XModelUserData *value) {
        m_userData = value;
    }

private:
    btAlignedObjectArray<btVector3> m_vertices;
    btAlignedObjectArray<btVector3> m_normals;
    btAlignedObjectArray<btVector3> m_coords;
    btAlignedObjectArray<btVector4> m_colors;
    btAlignedObjectArray<XModelIndexList *> m_indices;
    btAlignedObjectArray<XModelMaterial *> m_materials;
    XModelUserData *m_userData;
    const uint8_t *m_data;
    const size_t m_size;
    char *m_buffer;
};

}

#endif
