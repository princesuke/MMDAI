/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2013  hkrn                                    */
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

#ifndef VPVL2_PMD2_MODEL_H_
#define VPVL2_PMD2_MODEL_H_

#include "vpvl2/Common.h"
#include "vpvl2/IModel.h"

class btDiscreteDynamicsWorld;

namespace vpvl2
{

class IEncoding;
class IString;

namespace pmd2
{

class Bone;
class Joint;
class Label;
class Material;
class Morph;
class RigidBody;
class Vertex;

class VPVL2_API Model : public IModel
{
public:
    static const int kNameSize = 20;
    static const int kCommentSize = 256;
    static const int kCustomToonTextureNameSize = 100;
    static const int kMaxCustomToonTextures = 10;
    static const uint8_t *const kFallbackToonTextureName;

    struct DataInfo {
        IEncoding *encoding;
        ErrorType error;
        uint8_t *basePtr;
        uint8_t *namePtr;
        uint8_t *commentPtr;
        uint8_t *verticesPtr;
        size_t verticesCount;
        uint8_t *indicesPtr;
        size_t indicesCount;
        uint8_t *materialsPtr;
        size_t materialsCount;
        uint8_t *bonesPtr;
        size_t bonesCount;
        uint8_t *IKConstraintsPtr;
        size_t IKConstraintsCount;
        uint8_t *morphsPtr;
        size_t morphsCount;
        uint8_t *morphLabelsPtr;
        size_t morphLabelsCount;
        uint8_t *boneCategoryNamesPtr;
        size_t boneCategoryNamesCount;
        uint8_t *boneLabelsPtr;
        size_t boneLabelsCount;
        uint8_t *englishNamePtr;
        uint8_t *englishCommentPtr;
        uint8_t *englishBoneNamesPtr;
        uint8_t *englishFaceNamesPtr;
        uint8_t *englishBoneFramesPtr;
        uint8_t *customToonTextureNamesPtr;
        uint8_t *rigidBodiesPtr;
        size_t rigidBodiesCount;
        uint8_t *jointsPtr;
        size_t jointsCount;
    };

#pragma pack(push, 1)

    struct IKUnit {
        int16_t rootBoneID;
        int16_t targetBoneID;
        uint8_t nlinks;
        uint16_t niterations;
        float32_t angle;
    };

#pragma pack(pop)

    struct IKConstraint {
        IKUnit unit;
        Array<Bone *> effectors;
    };

    Model(IEncoding *encodingRef);
    ~Model();

    Type type() const { return kPMDModel; }
    const IString *name() const { return m_namePtr; }
    const IString *englishName() const { return m_englishNamePtr; }
    const IString *comment() const { return m_commentPtr; }
    const IString *englishComment() const { return m_englishCommentPtr; }
    bool isVisible() const { return m_visible && !btFuzzyZero(m_opacity); }
    ErrorType error() const;
    bool load(const uint8_t *data, size_t size);
    void save(uint8_t *data, size_t &written) const;
    size_t estimateSize() const;
    void resetAllVerticesTransform();
    void resetMotionState(btDiscreteDynamicsWorld *worldRef);
    void performUpdate();
    void joinWorld(btDiscreteDynamicsWorld *worldRef);
    void leaveWorld(btDiscreteDynamicsWorld *worldRef);
    IBone *findBone(const IString *value) const;
    IMorph *findMorph(const IString *value) const;
    int count(ObjectType value) const;
    void getBoneRefs(Array<IBone *> &value) const;
    void getLabelRefs(Array<ILabel *> &value) const;
    void getMaterialRefs(Array<IMaterial *> &value) const;
    void getMorphRefs(Array<IMorph *> &value) const;
    void getVertexRefs(Array<IVertex *> &value) const;
    IVertex::EdgeSizePrecision edgeScaleFactor(const Vector3 &cameraPosition) const;
    Vector3 worldPosition() const { return m_position; }
    Quaternion worldRotation() const { return m_rotation; }
    Scalar opacity() const { return m_opacity; }
    Scalar scaleFactor() const { return m_scaleFactor; }
    Vector3 edgeColor() const { return m_edgeColor; }
    Scalar edgeWidth() const { return m_edgeWidth; }
    Scene *parentSceneRef() const { return m_sceneRef; }
    IModel *parentModelRef() const { return 0; }
    IBone *parentBoneRef() const { return 0; }
    bool isPhysicsEnabled() const { return m_physicsEnabled; }
    void setName(const IString *value);
    void setEnglishName(const IString *value);
    void setComment(const IString *value);
    void setEnglishComment(const IString *value);
    void setWorldPosition(const Vector3 &value);
    void setWorldRotation(const Quaternion &value);
    void setOpacity(const Scalar &value);
    void setScaleFactor(const Scalar &value);
    void setEdgeColor(const Vector3 &value);
    void setEdgeWidth(const Scalar &value);
    void setParentSceneRef(Scene *value);
    void setParentModelRef(IModel * /* value */) {}
    void setParentBoneRef(IBone * /* value */) {}
    void setPhysicsEnable(bool value) { m_physicsEnabled = value; }

    bool preparse(const uint8_t *data, size_t size, DataInfo &info);
    void setVisible(bool value);
    void getAabb(Vector3 &min, Vector3 &max) const;
    void setAabb(const Vector3 &min, const Vector3 &max);

    float32_t version() const;
    void setVersion(float32_t value);
    IBone *createBone();
    ILabel *createLabel();
    IMaterial *createMaterial();
    IMorph *createMorph();
    IVertex *createVertex();
    IBone *findBoneAt(int value) const;
    ILabel *findLabelAt(int value) const;
    IMaterial *findMaterialAt(int value) const;
    IMorph *findMorphAt(int value) const;
    IVertex *findVertexAt(int value) const;
    void addBone(IBone *value);
    void addLabel(ILabel *value);
    void addMaterial(IMaterial *value);
    void addMorph(IMorph *value);
    void addVertex(IVertex *value);
    void removeBone(IBone *value);
    void removeLabel(ILabel *value);
    void removeMaterial(IMaterial *value);
    void removeMorph(IMorph *value);
    void removeVertex(IVertex *value);

    const PointerArray<Vertex> &vertices() const { return m_vertices; }
    const Array<int> &indices() const { return m_indices; }
    const PointerArray<Material> &materials() const { return m_materials; }
    const PointerArray<Bone> &bones() const { return m_bones; }
    const PointerArray<Morph> &morphs() const { return m_morphs; }
    const PointerArray<Label> &labels() const { return m_labels; }
    const PointerArray<RigidBody> &rigidBodies() const { return m_rigidBodies; }
    const PointerArray<Joint> &joints() const { return m_joints; }

    void getIndexBuffer(IIndexBuffer *&indexBuffer) const;
    void getStaticVertexBuffer(IStaticVertexBuffer *&staticBuffer) const;
    void getDynamicVertexBuffer(IDynamicVertexBuffer *&dynamicBuffer,
                                const IIndexBuffer *indexBuffer) const;
    void getMatrixBuffer(IMatrixBuffer *&matrixBuffer,
                         IDynamicVertexBuffer *dynamicBuffer,
                         const IIndexBuffer *indexBuffer) const;

private:
    void release();
    void parseNamesAndComments(const DataInfo &info);
    void parseVertices(const DataInfo &info);
    void parseIndices(const DataInfo &info);
    void parseMaterials(const DataInfo &info);
    void parseBones(const DataInfo &info);
    void parseIKConstraints(const DataInfo &info);
    void parseMorphs(const DataInfo &info);
    void parseLabels(const DataInfo &info);
    void parseCustomToonTextures(const DataInfo &info);
    void parseRigidBodies(const DataInfo &info);
    void parseJoints(const DataInfo &info);

    Scene *m_sceneRef;
    IEncoding *m_encodingRef;
    IString *m_namePtr;
    IString *m_englishNamePtr;
    IString *m_commentPtr;
    IString *m_englishCommentPtr;
    PointerArray<Vertex> m_vertices;
    Array<int> m_indices;
    PointerArray<Material> m_materials;
    PointerArray<Bone> m_bones;
    PointerArray<IKConstraint> m_constraints;
    PointerArray<Morph> m_morphs;
    PointerArray<Label> m_labels;
    PointerArray<RigidBody> m_rigidBodies;
    PointerArray<Joint> m_joints;
    PointerArray<IString> m_customToonTextures;
    Array<Bone *> m_sortedBoneRefs;
    Hash<HashString, IBone *> m_name2boneRefs;
    Hash<HashString, IMorph *> m_name2morphRefs;
    DataInfo m_info;
    Vector3 m_position;
    Quaternion m_rotation;
    Scalar m_opacity;
    Scalar m_scaleFactor;
    Vector3 m_edgeColor;
    Vector3 m_aabbMax;
    Vector3 m_aabbMin;
    Scalar m_edgeWidth;
    bool m_hasEnglish;
    bool m_visible;
    bool m_physicsEnabled;
};

} /* namespace pmd2 */
} /* namespace vpvl2 */

#endif
