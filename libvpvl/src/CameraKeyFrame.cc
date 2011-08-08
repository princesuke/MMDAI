/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn                                    */
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

#include "vpvl/vpvl.h"
#include "vpvl/internal/util.h"

namespace vpvl
{

#pragma pack(push, 1)

struct CameraKeyFrameChunk
{
    uint32_t frameIndex;
    float distance;
    float position[3];
    float angle[3];
    int8_t interpolationTable[24];
    uint32_t viewAngle;
    uint8_t noPerspective;
};

#pragma pack(pop)

struct InterpolationParameter
{
    btQuadWord x;
    btQuadWord y;
    btQuadWord z;
    btQuadWord rotation;
    btQuadWord distance;
    btQuadWord fovy;
};

CameraKeyFrame::CameraKeyFrame()
    : m_frameIndex(0),
      m_distance(0.0f),
      m_fovy(0.0f),
      m_position(0.0f, 0.0f, 0.0f),
      m_angle(0.0f, 0.0f, 0.0f),
      m_noPerspective(false),
      m_parameter(0)
{
    internal::zerofill(m_linear, sizeof(m_linear));
    internal::zerofill(m_interpolationTable, sizeof(m_interpolationTable));
    internal::zerofill(&m_parameter, sizeof(m_parameter));
    m_parameter = new InterpolationParameter();
}

CameraKeyFrame::~CameraKeyFrame()
{
    m_frameIndex = 0;
    m_distance = 0.0f;
    m_fovy = 0.0f;
    m_position.setZero();
    m_angle.setZero();
    m_noPerspective = false;
    for (int i = 0; i < 6; i++)
        delete[] m_interpolationTable[i];
    delete m_parameter;
    m_parameter = 0;
    internal::zerofill(m_linear, sizeof(m_linear));
    internal::zerofill(m_interpolationTable, sizeof(m_interpolationTable));
    internal::zerofill(&m_parameter, sizeof(m_parameter));
}

size_t CameraKeyFrame::stride()
{
    return sizeof(CameraKeyFrameChunk);
}

void CameraKeyFrame::read(const uint8_t *data)
{
    CameraKeyFrameChunk chunk;
    internal::copyBytes(reinterpret_cast<uint8_t *>(&chunk), data, sizeof(chunk));
    float *pos = chunk.position;
    float *angle = chunk.angle;

    m_frameIndex = static_cast<float>(chunk.frameIndex);
    m_fovy = static_cast<float>(chunk.viewAngle);
    m_noPerspective = chunk.noPerspective == 1;
#ifdef VPVL_COORDINATE_OPENGL
    m_distance = -chunk.distance;
    m_position.setValue(pos[0], pos[1], -pos[2]);
    m_angle.setValue(-degree(angle[0]), -degree(angle[1]), degree(angle[2]));
#else
    m_distance = chunk.distance;
    m_position.setValue(pos[0], pos[1], pos[2]);
    m_angle.setValue(degree(angle[0]), degree(angle[1]), degree(angle[2]));
#endif
    internal::copyBytes(reinterpret_cast<uint8_t *>(m_rawInterpolationTable),
                        reinterpret_cast<const uint8_t *>(chunk.interpolationTable),
                        sizeof(chunk.interpolationTable));
    setInterpolationTable(chunk.interpolationTable);
}

void CameraKeyFrame::write(uint8_t *data)
{
    CameraKeyFrameChunk chunk;
    chunk.frameIndex = static_cast<uint32_t>(m_frameIndex);
    chunk.viewAngle = static_cast<uint32_t>(m_fovy);
    chunk.noPerspective = m_noPerspective ? 1 : 0;
    chunk.position[0] = m_position.x();
    chunk.position[1] = m_position.y();
    chunk.angle[2] = radian(m_angle.z());
#ifdef VPVL_COORDINATE_OPENGL
    chunk.distance = -m_distance;
    chunk.angle[0] = -radian(m_angle.x());
    chunk.angle[1] = -radian(m_angle.y());
    chunk.position[2] = -m_position.z();
#else
    chunk.distance = m_distance;
    chunk.angle[0] = radian(m_angle.x());
    chunk.angle[1] = radian(m_angle.y());
    chunk.position[2] = m_position.z();
#endif
    internal::copyBytes(reinterpret_cast<uint8_t *>(chunk.interpolationTable),
                        reinterpret_cast<uint8_t *>(m_rawInterpolationTable),
                        sizeof(chunk.interpolationTable));
    internal::copyBytes(data, reinterpret_cast<const uint8_t *>(&chunk), sizeof(chunk));
}

void CameraKeyFrame::getInterpolationParameter(InterpolationType type, int8_t &x1, int8_t &x2, int8_t &y1, int8_t &y2)
{
    btQuadWord *w = getInterpolationParameterInternal(type);
    x1 = static_cast<int8_t>(w->x() * 127);
    x2 = static_cast<int8_t>(w->y() * 127);
    y1 = static_cast<int8_t>(w->z() * 127);
    y2 = static_cast<int8_t>(w->w() * 127);
}

void CameraKeyFrame::setInterpolationParameter(InterpolationType type, int8_t x1, int8_t x2, int8_t y1, int8_t y2)
{
    setInterpolationParameterInternal(type, x1, x2, y1, y2);
    int8_t table[kTableSize];
    internal::zerofill(table, sizeof(table));
    for (int i = 0; i < 4; i++) {
        // x1 => btQuadWord#x():0
        // x2 => btQuadWord#y():1
        // y1 => btQuadWord#z():2
        // y2 => btQuadWord#w():3
        table[i * 6 + 0] = m_parameter->x[i];
        table[i * 6 + 1] = m_parameter->y[i];
        table[i * 6 + 2] = m_parameter->z[i];
        table[i * 6 + 3] = m_parameter->rotation[i];
        table[i * 6 + 4] = m_parameter->distance[i];
        table[i * 6 + 5] = m_parameter->fovy[i];
    }
    internal::copyBytes(reinterpret_cast<uint8_t *>(m_rawInterpolationTable),
                        reinterpret_cast<const uint8_t *>(table), sizeof(table));
    setInterpolationTable(table);
}

void CameraKeyFrame::setInterpolationTable(const int8_t *table) {
    for (int i = 0; i < 6; i++)
        m_linear[i] = ((table[4 * i] == table[4 * i + 2]) && (table[4 * i + 1] == table[4 * i + 3])) ? true : false;
    for (int i = 0; i < 6; i++) {
        if (m_linear[i]) {
            m_interpolationTable[i] = 0;
            setInterpolationParameterInternal(static_cast<InterpolationType>(i), 0.0f, 0.0f, 0.0f, 0.0f);
            continue;
        }
        delete[] m_interpolationTable[i];
        m_interpolationTable[i] = new float[kTableSize + 1];
        float x1 = table[i * 4]     / 127.0f;
        float y1 = table[i * 4 + 2] / 127.0f;
        float x2 = table[i * 4 + 1] / 127.0f;
        float y2 = table[i * 4 + 3] / 127.0f;
        internal::buildInterpolationTable(x1, x2, y1, y2, kTableSize, m_interpolationTable[i]);
        setInterpolationParameterInternal(static_cast<InterpolationType>(i), x1, x2, y1, y2);
    }
}

void CameraKeyFrame::setInterpolationParameterInternal(InterpolationType type, float x1, float x2, float y1, float y2)
{
    btQuadWord *w = getInterpolationParameterInternal(type);
    w->setX(x1);
    w->setY(x2);
    w->setZ(y1);
    w->setW(y2);
}

btQuadWord *CameraKeyFrame::getInterpolationParameterInternal(InterpolationType type)
{
    switch (type) {
    case kX:
        return &m_parameter->x;
    case kY:
        return &m_parameter->y;
    case kZ:
        return &m_parameter->z;
    case kRotation:
        return &m_parameter->rotation;
    case kDistance:
        return &m_parameter->distance;
    case kFovy:
        return &m_parameter->fovy;
    default:
        assert(0);
    }
}

}
