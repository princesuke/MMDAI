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

#include "BaseMotionTrack.h"
#include "BoneKeyframeRefObject.h"
#include "BoneMotionTrack.h"
#include "BoneRefObject.h"
#include "ModelProxy.h"
#include "MotionProxy.h"
#include "Util.h"

#include <QtCore>
#include <vpvl2/vpvl2.h>
#include <vpvl2/extensions/qt/String.h>

using namespace vpvl2;
using namespace vpvl2::extensions;

BoneKeyframeRefObject::BoneKeyframeRefObject(BoneMotionTrack *trackRef, IBoneKeyframe *data)
    : BaseKeyframeRefObject(trackRef->parentMotion()),
      m_parentTrackRef(trackRef),
      m_keyframe(data)
{
    Q_ASSERT(m_parentTrackRef);
    Q_ASSERT(m_keyframe);
}

BoneKeyframeRefObject::~BoneKeyframeRefObject()
{
    if (isDeleteable()) {
        delete m_keyframe;
    }
    m_parentTrackRef = 0;
    m_keyframe = 0;
}

QJsonValue BoneKeyframeRefObject::toJson() const
{
    QJsonObject v = BaseKeyframeRefObject::toJson().toObject(), i;
    v.insert("localTranslation", Util::toJson(localTranslation()));
    v.insert("localOrientation", Util::toJson(localOrientation()));
    addInterpolationParameterToJson("translationX", IBoneKeyframe::kBonePositionX, i);
    addInterpolationParameterToJson("translationY", IBoneKeyframe::kBonePositionX, i);
    addInterpolationParameterToJson("translationZ", IBoneKeyframe::kBonePositionX, i);
    addInterpolationParameterToJson("orientation", IBoneKeyframe::kBonePositionX, i);
    v.insert("interpolation", i);
    return v;
}

QVector4D BoneKeyframeRefObject::interpolationParameter(int type) const
{
    Q_ASSERT(m_keyframe);
    QuadWord value;
    m_keyframe->getInterpolationParameter(static_cast<IBoneKeyframe::InterpolationType>(type), value);
    return QVector4D(value.x(), value.y(), value.z(), value.w());
}

void BoneKeyframeRefObject::setInterpolationParameter(int type, const QVector4D &value)
{
    Q_ASSERT(m_keyframe);
    QuadWord v(value.x(), value.y(), value.z(), value.w());
    m_keyframe->setInterpolationParameter(static_cast<IBoneKeyframe::InterpolationType>(type), v);
}

BaseMotionTrack *BoneKeyframeRefObject::parentTrack() const
{
    return m_parentTrackRef;
}

BoneRefObject *BoneKeyframeRefObject::parentBone() const
{
    if (ModelProxy *modelProxy = parentMotion()->parentModel()) {
        return modelProxy->findBoneByName(name());
    }
    return 0;
}

IKeyframe *BoneKeyframeRefObject::baseKeyframeData() const
{
    return data();
}

QObject *BoneKeyframeRefObject::opaque() const
{
    return parentBone();
}

QString BoneKeyframeRefObject::name() const
{
    Q_ASSERT(m_keyframe);
    return Util::toQString(m_keyframe->name());
}

void BoneKeyframeRefObject::setName(const QString &value)
{
    Q_ASSERT(m_keyframe);
    if (!Util::equalsString(value, m_keyframe->name())) {
        qt::String s(value);
        m_keyframe->setName(&s);
    }
}

QVector3D BoneKeyframeRefObject::localTranslation() const
{
    Q_ASSERT(m_keyframe);
    return Util::fromVector3(m_keyframe->localTranslation());
}

void BoneKeyframeRefObject::setLocalTranslation(const QVector3D &value)
{
    Q_ASSERT(m_keyframe);
    if (!qFuzzyCompare(value, localTranslation())) {
        m_keyframe->setLocalTranslation(Util::toVector3(value));
        emit localTranslationChanged();
    }
}

QQuaternion BoneKeyframeRefObject::localOrientation() const
{
    Q_ASSERT(m_keyframe);
    return Util::fromQuaternion(m_keyframe->localOrientation());
}

void BoneKeyframeRefObject::setLocalOrientation(const QQuaternion &value)
{
    Q_ASSERT(m_keyframe);
    if (!qFuzzyCompare(value, localOrientation())) {
        m_keyframe->setLocalOrientation(Util::toQuaternion(value));
        emit localRotationChanged();
    }
}

QVector3D BoneKeyframeRefObject::localEulerOrientation() const
{
    Q_ASSERT(m_keyframe);
    Scalar yaw, pitch, roll;
    Matrix3x3 matrix(m_keyframe->localOrientation());
    matrix.getEulerZYX(yaw, pitch, roll);
    return QVector3D(qRadiansToDegrees(roll), qRadiansToDegrees(pitch), qRadiansToDegrees(yaw));
}

void BoneKeyframeRefObject::setLocalEulerOrientation(const QVector3D &value)
{
    Q_ASSERT(m_keyframe);
    if (!qFuzzyCompare(localEulerOrientation(), value)) {
        Quaternion rotation(Quaternion::getIdentity());
        rotation.setEulerZYX(qDegreesToRadians(value.z()), qDegreesToRadians(value.y()), qDegreesToRadians(value.x()));
        setLocalOrientation(Util::fromQuaternion(rotation));
    }
}

IBoneKeyframe *BoneKeyframeRefObject::data() const
{
    return m_keyframe;
}

void BoneKeyframeRefObject::addInterpolationParameterToJson(const QString &key, int type, QJsonObject &v) const
{
    const QVector4D &v2 = interpolationParameter(type);
    QJsonObject i;
    i.insert("x1", v2.x());
    i.insert("y1", v2.y());
    i.insert("x2", v2.z());
    i.insert("y2", v2.w());
    v.insert(key, i);
}
