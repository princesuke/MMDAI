/**

 Copyright (c) 2010-2014  hkrn

 All rights reserved.

 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.
 - Neither the name of the MMDAI project team nor the names of
   its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

#include "vpvl2/vpvl2.h"
#include "vpvl2/internal/MotionHelper.h"

#include "vpvl2/mvd/EffectKeyframe.h"
#include "vpvl2/mvd/EffectSection.h"

namespace vpvl2
{
namespace VPVL2_VERSION_NS
{
namespace mvd
{

#pragma pack(push, 1)

struct EffectSectionHeader {
    int32 reserved;
    int32 sizeOfKeyframe;
    int32 countOfKeyframes;
    int32 parameterSize;
    int32 parameterCount;
};

struct EffectParameter {
    int32 pid;
    int32 type;
};

#pragma pack(pop)

class EffectSection::PrivateContext : public BaseAnimationTrack {
public:
    PrivateContext()
    {
    }
    ~PrivateContext() {
    }
};

EffectSection::EffectSection(const Motion *motionRef)
    : BaseSection(motionRef),
      m_context(new PrivateContext())
{
}

EffectSection::~EffectSection()
{
    internal::deleteObject(m_context);
}

bool EffectSection::preparse(uint8 *&ptr, vsize &rest, Motion::DataInfo &info)
{
    EffectSectionHeader header;
    if (!internal::validateSize(ptr, sizeof(header), rest)) {
        return false;
    }
    internal::getData(ptr - sizeof(header), header);
    const int nparameters = header.parameterCount;
    const vsize parameterArraySize = nparameters * sizeof(int);
    if (!internal::validateSize(ptr, parameterArraySize, rest)) {
        return false;
    }
    if (!internal::validateSize(ptr, header.parameterSize - 8 * nparameters - 4, rest)) {
        return false;
    }
    const int nkeyframes = header.countOfKeyframes;
    const vsize reserved = header.sizeOfKeyframe - (EffectKeyframe::size() + parameterArraySize);
    for (int i = 0; i < nkeyframes; i++) {
        if (!EffectKeyframe::preparse(ptr, rest, reserved, info)) {
            return false;
        }
    }
    return true;
}

void EffectSection::read(const uint8 * /* data */)
{
}

void EffectSection::seek(const IKeyframe::TimeIndex &timeIndex)
{
    saveCurrentTimeIndex(timeIndex);
}

void EffectSection::write(uint8 * /* data */) const
{
}

vsize EffectSection::estimateSize() const
{
    return 0;
}

vsize EffectSection::countKeyframes() const
{
    return m_context->keyframes.count();
}

void EffectSection::update()
{
    updateKeyframes(m_context->keyframes);
}

void EffectSection::addKeyframe(IKeyframe *keyframe)
{
    m_context->keyframes.append(keyframe);
}

void EffectSection::removeKeyframe(IKeyframe *keyframe)
{
    m_context->keyframes.remove(keyframe);
}

void EffectSection::deleteKeyframe(IKeyframe *&keyframe)
{
    removeKeyframe(keyframe);
    internal::deleteObject(keyframe);
}

void EffectSection::getKeyframes(const IKeyframe::TimeIndex & /* timeIndex */,
                                 const IKeyframe::LayerIndex & /* layerIndex */,
                                 Array<IKeyframe *> & /* keyframes */) const
{
}

void EffectSection::getAllKeyframes(Array<IKeyframe *> &keyframes) const
{
    keyframes.copy(m_context->keyframes);
}

void EffectSection::setAllKeyframes(const Array<IKeyframe *> &value)
{
    release();
    m_context = new PrivateContext();
    const int nkeyframes = value.count();
    for (int i = 0; i < nkeyframes; i++) {
        IKeyframe *keyframe = value[i];
        if (keyframe && keyframe->type() == IKeyframe::kEffectKeyframe) {
            addKeyframe(keyframe);
        }
    }
}

void EffectSection::createFirstKeyframeUnlessFound()
{
    if (!findKeyframe(0, 0, 0)) {
        EffectKeyframe *keyframe = m_context->keyframes.append(new EffectKeyframe(m_motionRef));
        keyframe->setAddBlendEnable(false);
        keyframe->setLayerIndex(0);
        keyframe->setName(0);
        keyframe->setOpacity(1.0);
        keyframe->setScaleFactor(1.0);
        keyframe->setShadowEnable(true);
        keyframe->setTimeIndex(0);
        keyframe->setVisible(true);
        update();
    }
}

int EffectSection::countLayers(const IString * /* name */) const
{
    return 1;
}

IEffectKeyframe *EffectSection::findKeyframe(const IKeyframe::TimeIndex &timeIndex,
                                             const IString * /* name */,
                                             const IKeyframe::LayerIndex &layerIndex) const
{
    const PrivateContext::KeyframeCollection &keyframes = m_context->keyframes;
    const int nkeyframes = keyframes.count();
    for (int i = 0; i < nkeyframes; i++) {
        EffectKeyframe *keyframe = reinterpret_cast<EffectKeyframe *>(keyframes[i]);
        if (keyframe->timeIndex() == timeIndex && keyframe->layerIndex() == layerIndex) {
            return keyframe;
        }
    }
    return 0;
}

IEffectKeyframe *EffectSection::findKeyframeAt(int index) const
{
    const PrivateContext::KeyframeCollection &keyframes = m_context->keyframes;
    if (internal::checkBound(index, 0, keyframes.count())) {
        EffectKeyframe *keyframe = reinterpret_cast<EffectKeyframe *>(keyframes[index]);
        return keyframe;
    }
    return 0;
}

} /* namespace mvd */
} /* namespace VPVL2_VERSION_NS */
} /* namespace vpvl2 */
