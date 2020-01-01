/*

    This file was automatically generated.
    https://github.com/CharlesCarley/FileTools

    By    : TableDecompiler.exe
    From  : API/multibody.bullet(BULLETf-v288)
    On    : Wed Jan 01 03:03:13 PM

*/
#ifndef _BulletFile_h_
#define _BulletFile_h_

namespace BulletFile
{
#pragma region Forward
    struct PointerArray;
    struct btPhysicsSystem;
    struct ListBase;
    struct btVector3FloatData;
    struct btVector3DoubleData;
    struct btQuaternionFloatData;
    struct btQuaternionDoubleData;
    struct btMatrix3x3FloatData;
    struct btMatrix3x3DoubleData;
    struct btTransformFloatData;
    struct btTransformDoubleData;
    struct btBvhSubtreeInfoData;
    struct btOptimizedBvhNodeFloatData;
    struct btOptimizedBvhNodeDoubleData;
    struct btQuantizedBvhNodeData;
    struct btQuantizedBvhFloatData;
    struct btQuantizedBvhDoubleData;
    struct btCollisionShapeData;
    struct btStaticPlaneShapeData;
    struct btConvexInternalShapeData;
    struct btPositionAndRadius;
    struct btMultiSphereShapeData;
    struct btIntIndexData;
    struct btShortIntIndexData;
    struct btShortIntIndexTripletData;
    struct btCharIndexTripletData;
    struct btMeshPartData;
    struct btStridingMeshInterfaceData;
    struct btTriangleMeshShapeData;
    struct btScaledTriangleMeshShapeData;
    struct btCompoundShapeChildData;
    struct btCompoundShapeData;
    struct btCylinderShapeData;
    struct btConeShapeData;
    struct btCapsuleShapeData;
    struct btTriangleInfoData;
    struct btTriangleInfoMapData;
    struct btPersistentManifoldDoubleData;
    struct btPersistentManifoldFloatData;
    struct btGImpactMeshShapeData;
    struct btConvexHullShapeData;
    struct btCollisionObjectDoubleData;
    struct btCollisionObjectFloatData;
    struct btContactSolverInfoDoubleData;
    struct btContactSolverInfoFloatData;
    struct btDynamicsWorldDoubleData;
    struct btDynamicsWorldFloatData;
    struct btRigidBodyFloatData;
    struct btRigidBodyDoubleData;
    struct btConstraintInfo1;
    struct btTypedConstraintFloatData;
    struct btTypedConstraintData;
    struct btTypedConstraintDoubleData;
    struct btPoint2PointConstraintFloatData;
    struct btPoint2PointConstraintDoubleData2;
    struct btPoint2PointConstraintDoubleData;
    struct btHingeConstraintDoubleData;
    struct btHingeConstraintFloatData;
    struct btHingeConstraintDoubleData2;
    struct btConeTwistConstraintDoubleData;
    struct btConeTwistConstraintData;
    struct btGeneric6DofConstraintData;
    struct btGeneric6DofConstraintDoubleData2;
    struct btGeneric6DofSpringConstraintData;
    struct btGeneric6DofSpringConstraintDoubleData2;
    struct btGeneric6DofSpring2ConstraintData;
    struct btGeneric6DofSpring2ConstraintDoubleData2;
    struct btSliderConstraintData;
    struct btSliderConstraintDoubleData;
    struct btGearConstraintFloatData;
    struct btGearConstraintDoubleData;
    struct SoftBodyMaterialData;
    struct SoftBodyNodeData;
    struct SoftBodyLinkData;
    struct SoftBodyFaceData;
    struct SoftBodyTetraData;
    struct SoftRigidAnchorData;
    struct SoftBodyConfigData;
    struct SoftBodyPoseData;
    struct SoftBodyClusterData;
    struct btSoftBodyJointData;
    struct btSoftBodyFloatData;
    struct btMultiBodyLinkDoubleData;
    struct btMultiBodyLinkFloatData;
    struct btMultiBodyDoubleData;
    struct btMultiBodyFloatData;
    struct btMultiBodyLinkColliderFloatData;
    struct btMultiBodyLinkColliderDoubleData;
#pragma endregion

// Pointers that have references to no known
// struct need to be declared as some type of handle.
// This should be a struct handle class so that it can be
// recompiled. struct XXX {int unused; }
#pragma region MissingStructures

    struct btRigidBodyData
    {
        int missing;
    };

#pragma endregion

// Independent structures:
// The member declarations only contain references to other
// structures via a pointer (or only atomic types); Therefore,
// declaration order does not matter as long as any pointer
// reference is forwardly declared.
#pragma region Independent
    struct PointerArray
    {
        int   m_size;
        int   m_capacity;
        void *m_data;
    };

    struct ListBase
    {
        void *first;
        void *last;
    };

    struct btVector3FloatData
    {
        float m_floats[4];
    };

    struct btVector3DoubleData
    {
        double m_floats[4];
    };

    struct btQuaternionFloatData
    {
        float m_floats[4];
    };

    struct btQuaternionDoubleData
    {
        double m_floats[4];
    };

    struct btBvhSubtreeInfoData
    {
        int   m_rootNodeIndex;
        int   m_subtreeSize;
        short m_quantizedAabbMin[3];
        short m_quantizedAabbMax[3];
    };

    struct btQuantizedBvhNodeData
    {
        short m_quantizedAabbMin[3];
        short m_quantizedAabbMax[3];
        int   m_escapeIndexOrTriangleIndex;
    };

    struct btCollisionShapeData
    {
        char *m_name;
        int   m_shapeType;
        char  m_padding[4];
    };

    struct btIntIndexData
    {
        int m_value;
    };

    struct btShortIntIndexData
    {
        short m_value;
        char  m_pad[2];
    };

    struct btShortIntIndexTripletData
    {
        short m_values[3];
        char  m_pad[2];
    };

    struct btCharIndexTripletData
    {
        char m_values[3];
        char m_pad;
    };

    struct btMeshPartData
    {
        btVector3FloatData *        m_vertices3f;
        btVector3DoubleData *       m_vertices3d;
        btIntIndexData *            m_indices32;
        btShortIntIndexTripletData *m_3indices16;
        btCharIndexTripletData *    m_3indices8;
        btShortIntIndexData *       m_indices16;
        int                         m_numTriangles;
        int                         m_numVertices;
    };

    struct btTriangleInfoData
    {
        int   m_flags;
        float m_edgeV0V1Angle;
        float m_edgeV1V2Angle;
        float m_edgeV2V0Angle;
    };

    struct btTriangleInfoMapData
    {
        int *               m_hashTablePtr;
        int *               m_nextPtr;
        btTriangleInfoData *m_valueArrayPtr;
        int *               m_keyArrayPtr;
        float               m_convexEpsilon;
        float               m_planarEpsilon;
        float               m_equalVertexThreshold;
        float               m_edgeDistanceThreshold;
        float               m_zeroAreaThreshold;
        int                 m_nextSize;
        int                 m_hashTableSize;
        int                 m_numValues;
        int                 m_numKeys;
        char                m_padding[4];
    };

    struct btContactSolverInfoDoubleData
    {
        double m_tau;
        double m_damping;
        double m_friction;
        double m_timeStep;
        double m_restitution;
        double m_maxErrorReduction;
        double m_sor;
        double m_erp;
        double m_erp2;
        double m_globalCfm;
        double m_splitImpulsePenetrationThreshold;
        double m_splitImpulseTurnErp;
        double m_linearSlop;
        double m_warmstartingFactor;
        double m_maxGyroscopicForce;
        double m_singleAxisRollingFrictionThreshold;
        int    m_numIterations;
        int    m_solverMode;
        int    m_restingContactRestitutionThreshold;
        int    m_minimumSolverBatchSize;
        int    m_splitImpulse;
        char   m_padding[4];
    };

    struct btContactSolverInfoFloatData
    {
        float m_tau;
        float m_damping;
        float m_friction;
        float m_timeStep;
        float m_restitution;
        float m_maxErrorReduction;
        float m_sor;
        float m_erp;
        float m_erp2;
        float m_globalCfm;
        float m_splitImpulsePenetrationThreshold;
        float m_splitImpulseTurnErp;
        float m_linearSlop;
        float m_warmstartingFactor;
        float m_maxGyroscopicForce;
        float m_singleAxisRollingFrictionThreshold;
        int   m_numIterations;
        int   m_solverMode;
        int   m_restingContactRestitutionThreshold;
        int   m_minimumSolverBatchSize;
        int   m_splitImpulse;
        char  m_padding[4];
    };

    struct btConstraintInfo1
    {
        int m_numConstraintRows;
        int nub;
    };

    struct btTypedConstraintFloatData
    {
        btRigidBodyFloatData *m_rbA;
        btRigidBodyFloatData *m_rbB;
        char *                m_name;
        int                   m_objectType;
        int                   m_userConstraintType;
        int                   m_userConstraintId;
        int                   m_needsFeedback;
        float                 m_appliedImpulse;
        float                 m_dbgDrawSize;
        int                   m_disableCollisionsBetweenLinkedBodies;
        int                   m_overrideNumSolverIterations;
        float                 m_breakingImpulseThreshold;
        int                   m_isEnabled;
    };

    struct btTypedConstraintData
    {
        btRigidBodyData *m_rbA;
        btRigidBodyData *m_rbB;
        char *           m_name;
        int              m_objectType;
        int              m_userConstraintType;
        int              m_userConstraintId;
        int              m_needsFeedback;
        float            m_appliedImpulse;
        float            m_dbgDrawSize;
        int              m_disableCollisionsBetweenLinkedBodies;
        int              m_overrideNumSolverIterations;
        float            m_breakingImpulseThreshold;
        int              m_isEnabled;
    };

    struct btTypedConstraintDoubleData
    {
        btRigidBodyDoubleData *m_rbA;
        btRigidBodyDoubleData *m_rbB;
        char *                 m_name;
        int                    m_objectType;
        int                    m_userConstraintType;
        int                    m_userConstraintId;
        int                    m_needsFeedback;
        double                 m_appliedImpulse;
        double                 m_dbgDrawSize;
        int                    m_disableCollisionsBetweenLinkedBodies;
        int                    m_overrideNumSolverIterations;
        double                 m_breakingImpulseThreshold;
        int                    m_isEnabled;
        char                   padding[4];
    };

    struct SoftBodyMaterialData
    {
        float m_linearStiffness;
        float m_angularStiffness;
        float m_volumeStiffness;
        int   m_flags;
    };

    struct SoftBodyLinkData
    {
        SoftBodyMaterialData *m_material;
        int                   m_nodeIndices[2];
        float                 m_restLength;
        int                   m_bbending;
    };

    struct SoftBodyConfigData
    {
        int   m_aeroModel;
        float m_baumgarte;
        float m_damping;
        float m_drag;
        float m_lift;
        float m_pressure;
        float m_volume;
        float m_dynamicFriction;
        float m_poseMatch;
        float m_rigidContactHardness;
        float m_kineticContactHardness;
        float m_softContactHardness;
        float m_anchorHardness;
        float m_softRigidClusterHardness;
        float m_softKineticClusterHardness;
        float m_softSoftClusterHardness;
        float m_softRigidClusterImpulseSplit;
        float m_softKineticClusterImpulseSplit;
        float m_softSoftClusterImpulseSplit;
        float m_maxVolume;
        float m_timeScale;
        int   m_velocityIterations;
        int   m_positionIterations;
        int   m_driftIterations;
        int   m_clusterIterations;
        int   m_collisionFlags;
    };

#pragma endregion

// Dependent structures:
// The member declarations have references to other
// structures without a pointer; Therefore, declaration order DOES matter.
// If a structure has a non pointer member structure, then that structure
// must be visible before defining the structure that uses it.
#pragma region Dependent
    struct btMultiBodyFloatData
    {
        btVector3FloatData          m_baseWorldPosition;
        btQuaternionFloatData       m_baseWorldOrientation;
        btVector3FloatData          m_baseLinearVelocity;
        btVector3FloatData          m_baseAngularVelocity;
        btVector3FloatData          m_baseInertia;
        float                       m_baseMass;
        int                         m_numLinks;
        char *                      m_baseName;
        btMultiBodyLinkFloatData *  m_links;
        btCollisionObjectFloatData *m_baseCollider;
    };

    struct btMultiBodyDoubleData
    {
        btVector3DoubleData          m_baseWorldPosition;
        btQuaternionDoubleData       m_baseWorldOrientation;
        btVector3DoubleData          m_baseLinearVelocity;
        btVector3DoubleData          m_baseAngularVelocity;
        btVector3DoubleData          m_baseInertia;
        double                       m_baseMass;
        int                          m_numLinks;
        char                         m_padding[4];
        char *                       m_baseName;
        btMultiBodyLinkDoubleData *  m_links;
        btCollisionObjectDoubleData *m_baseCollider;
    };

    struct btMultiBodyLinkFloatData
    {
        btQuaternionFloatData       m_zeroRotParentToThis;
        btVector3FloatData          m_parentComToThisPivotOffset;
        btVector3FloatData          m_thisPivotToThisComOffset;
        btVector3FloatData          m_jointAxisTop[6];
        btVector3FloatData          m_jointAxisBottom[6];
        btVector3FloatData          m_linkInertia;
        btVector3FloatData          m_absFrameTotVelocityTop;
        btVector3FloatData          m_absFrameTotVelocityBottom;
        btVector3FloatData          m_absFrameLocVelocityTop;
        btVector3FloatData          m_absFrameLocVelocityBottom;
        int                         m_dofCount;
        float                       m_linkMass;
        int                         m_parentIndex;
        int                         m_jointType;
        float                       m_jointPos[7];
        float                       m_jointVel[6];
        float                       m_jointTorque[6];
        int                         m_posVarCount;
        float                       m_jointDamping;
        float                       m_jointFriction;
        float                       m_jointLowerLimit;
        float                       m_jointUpperLimit;
        float                       m_jointMaxForce;
        float                       m_jointMaxVelocity;
        char *                      m_linkName;
        char *                      m_jointName;
        btCollisionObjectFloatData *m_linkCollider;
        char *                      m_paddingPtr;
    };

    struct btMultiBodyLinkDoubleData
    {
        btQuaternionDoubleData       m_zeroRotParentToThis;
        btVector3DoubleData          m_parentComToThisPivotOffset;
        btVector3DoubleData          m_thisPivotToThisComOffset;
        btVector3DoubleData          m_jointAxisTop[6];
        btVector3DoubleData          m_jointAxisBottom[6];
        btVector3DoubleData          m_linkInertia;
        btVector3DoubleData          m_absFrameTotVelocityTop;
        btVector3DoubleData          m_absFrameTotVelocityBottom;
        btVector3DoubleData          m_absFrameLocVelocityTop;
        btVector3DoubleData          m_absFrameLocVelocityBottom;
        double                       m_linkMass;
        int                          m_parentIndex;
        int                          m_jointType;
        int                          m_dofCount;
        int                          m_posVarCount;
        double                       m_jointPos[7];
        double                       m_jointVel[6];
        double                       m_jointTorque[6];
        double                       m_jointDamping;
        double                       m_jointFriction;
        double                       m_jointLowerLimit;
        double                       m_jointUpperLimit;
        double                       m_jointMaxForce;
        double                       m_jointMaxVelocity;
        char *                       m_linkName;
        char *                       m_jointName;
        btCollisionObjectDoubleData *m_linkCollider;
        char *                       m_paddingPtr;
    };

    struct btSoftBodyJointData
    {
        void *             m_bodyA;
        void *             m_bodyB;
        btVector3FloatData m_refs[2];
        float              m_cfm;
        float              m_erp;
        float              m_split;
        int                m_delete;
        btVector3FloatData m_relPosition[2];
        int                m_bodyAtype;
        int                m_bodyBtype;
        int                m_jointType;
        int                m_pad;
    };

    struct SoftBodyTetraData
    {
        btVector3FloatData    m_c0[4];
        SoftBodyMaterialData *m_material;
        int                   m_nodeIndices[4];
        float                 m_restVolume;
        float                 m_c1;
        float                 m_c2;
        int                   m_pad;
    };

    struct SoftBodyFaceData
    {
        btVector3FloatData    m_normal;
        SoftBodyMaterialData *m_material;
        int                   m_nodeIndices[3];
        float                 m_restArea;
    };

    struct SoftBodyNodeData
    {
        SoftBodyMaterialData *m_material;
        btVector3FloatData    m_position;
        btVector3FloatData    m_previousPosition;
        btVector3FloatData    m_velocity;
        btVector3FloatData    m_accumulatedForce;
        btVector3FloatData    m_normal;
        float                 m_inverseMass;
        float                 m_area;
        int                   m_attach;
        int                   m_pad;
    };

    struct btGearConstraintDoubleData
    {
        btTypedConstraintDoubleData m_typeConstraintData;
        btVector3DoubleData         m_axisInA;
        btVector3DoubleData         m_axisInB;
        double                      m_ratio;
    };

    struct btGearConstraintFloatData
    {
        btTypedConstraintFloatData m_typeConstraintData;
        btVector3FloatData         m_axisInA;
        btVector3FloatData         m_axisInB;
        float                      m_ratio;
        char                       m_padding[4];
    };

    struct btPoint2PointConstraintDoubleData
    {
        btTypedConstraintData m_typeConstraintData;
        btVector3DoubleData   m_pivotInA;
        btVector3DoubleData   m_pivotInB;
    };

    struct btPoint2PointConstraintDoubleData2
    {
        btTypedConstraintDoubleData m_typeConstraintData;
        btVector3DoubleData         m_pivotInA;
        btVector3DoubleData         m_pivotInB;
    };

    struct btPoint2PointConstraintFloatData
    {
        btTypedConstraintData m_typeConstraintData;
        btVector3FloatData    m_pivotInA;
        btVector3FloatData    m_pivotInB;
    };

    struct btDynamicsWorldFloatData
    {
        btContactSolverInfoFloatData m_solverInfo;
        btVector3FloatData           m_gravity;
    };

    struct btDynamicsWorldDoubleData
    {
        btContactSolverInfoDoubleData m_solverInfo;
        btVector3DoubleData           m_gravity;
    };

    struct btPersistentManifoldFloatData
    {
        btVector3FloatData          m_pointCacheLocalPointA[4];
        btVector3FloatData          m_pointCacheLocalPointB[4];
        btVector3FloatData          m_pointCachePositionWorldOnA[4];
        btVector3FloatData          m_pointCachePositionWorldOnB[4];
        btVector3FloatData          m_pointCacheNormalWorldOnB[4];
        btVector3FloatData          m_pointCacheLateralFrictionDir1[4];
        btVector3FloatData          m_pointCacheLateralFrictionDir2[4];
        float                       m_pointCacheDistance[4];
        float                       m_pointCacheAppliedImpulse[4];
        float                       m_pointCacheCombinedFriction[4];
        float                       m_pointCacheCombinedRollingFriction[4];
        float                       m_pointCacheCombinedSpinningFriction[4];
        float                       m_pointCacheCombinedRestitution[4];
        int                         m_pointCachePartId0[4];
        int                         m_pointCachePartId1[4];
        int                         m_pointCacheIndex0[4];
        int                         m_pointCacheIndex1[4];
        int                         m_pointCacheContactPointFlags[4];
        float                       m_pointCacheAppliedImpulseLateral1[4];
        float                       m_pointCacheAppliedImpulseLateral2[4];
        float                       m_pointCacheContactMotion1[4];
        float                       m_pointCacheContactMotion2[4];
        float                       m_pointCacheContactCFM[4];
        float                       m_pointCacheCombinedContactStiffness1[4];
        float                       m_pointCacheContactERP[4];
        float                       m_pointCacheCombinedContactDamping1[4];
        float                       m_pointCacheFrictionCFM[4];
        int                         m_pointCacheLifeTime[4];
        int                         m_numCachedPoints;
        int                         m_companionIdA;
        int                         m_companionIdB;
        int                         m_index1a;
        int                         m_objectType;
        float                       m_contactBreakingThreshold;
        float                       m_contactProcessingThreshold;
        int                         m_padding;
        btCollisionObjectFloatData *m_body0;
        btCollisionObjectFloatData *m_body1;
    };

    struct btPersistentManifoldDoubleData
    {
        btVector3DoubleData          m_pointCacheLocalPointA[4];
        btVector3DoubleData          m_pointCacheLocalPointB[4];
        btVector3DoubleData          m_pointCachePositionWorldOnA[4];
        btVector3DoubleData          m_pointCachePositionWorldOnB[4];
        btVector3DoubleData          m_pointCacheNormalWorldOnB[4];
        btVector3DoubleData          m_pointCacheLateralFrictionDir1[4];
        btVector3DoubleData          m_pointCacheLateralFrictionDir2[4];
        double                       m_pointCacheDistance[4];
        double                       m_pointCacheAppliedImpulse[4];
        double                       m_pointCacheCombinedFriction[4];
        double                       m_pointCacheCombinedRollingFriction[4];
        double                       m_pointCacheCombinedSpinningFriction[4];
        double                       m_pointCacheCombinedRestitution[4];
        int                          m_pointCachePartId0[4];
        int                          m_pointCachePartId1[4];
        int                          m_pointCacheIndex0[4];
        int                          m_pointCacheIndex1[4];
        int                          m_pointCacheContactPointFlags[4];
        double                       m_pointCacheAppliedImpulseLateral1[4];
        double                       m_pointCacheAppliedImpulseLateral2[4];
        double                       m_pointCacheContactMotion1[4];
        double                       m_pointCacheContactMotion2[4];
        double                       m_pointCacheContactCFM[4];
        double                       m_pointCacheCombinedContactStiffness1[4];
        double                       m_pointCacheContactERP[4];
        double                       m_pointCacheCombinedContactDamping1[4];
        double                       m_pointCacheFrictionCFM[4];
        int                          m_pointCacheLifeTime[4];
        int                          m_numCachedPoints;
        int                          m_companionIdA;
        int                          m_companionIdB;
        int                          m_index1a;
        int                          m_objectType;
        double                       m_contactBreakingThreshold;
        double                       m_contactProcessingThreshold;
        int                          m_padding;
        btCollisionObjectDoubleData *m_body0;
        btCollisionObjectDoubleData *m_body1;
    };

    struct btCompoundShapeData
    {
        btCollisionShapeData      m_collisionShapeData;
        btCompoundShapeChildData *m_childShapePtr;
        int                       m_numChildShapes;
        float                     m_collisionMargin;
    };

    struct btStridingMeshInterfaceData
    {
        btMeshPartData *   m_meshPartsPtr;
        btVector3FloatData m_scaling;
        int                m_numMeshParts;
        char               m_padding[4];
    };

    struct btPositionAndRadius
    {
        btVector3FloatData m_pos;
        float              m_radius;
    };

    struct btConvexInternalShapeData
    {
        btCollisionShapeData m_collisionShapeData;
        btVector3FloatData   m_localScaling;
        btVector3FloatData   m_implicitShapeDimensions;
        float                m_collisionMargin;
        int                  m_padding;
    };

    struct btStaticPlaneShapeData
    {
        btCollisionShapeData m_collisionShapeData;
        btVector3FloatData   m_localScaling;
        btVector3FloatData   m_planeNormal;
        float                m_planeConstant;
        char                 m_pad[4];
    };

    struct btQuantizedBvhDoubleData
    {
        btVector3DoubleData           m_bvhAabbMin;
        btVector3DoubleData           m_bvhAabbMax;
        btVector3DoubleData           m_bvhQuantization;
        int                           m_curNodeIndex;
        int                           m_useQuantization;
        int                           m_numContiguousLeafNodes;
        int                           m_numQuantizedContiguousNodes;
        btOptimizedBvhNodeDoubleData *m_contiguousNodesPtr;
        btQuantizedBvhNodeData *      m_quantizedContiguousNodesPtr;
        int                           m_traversalMode;
        int                           m_numSubtreeHeaders;
        btBvhSubtreeInfoData *        m_subTreeInfoPtr;
    };

    struct btQuantizedBvhFloatData
    {
        btVector3FloatData           m_bvhAabbMin;
        btVector3FloatData           m_bvhAabbMax;
        btVector3FloatData           m_bvhQuantization;
        int                          m_curNodeIndex;
        int                          m_useQuantization;
        int                          m_numContiguousLeafNodes;
        int                          m_numQuantizedContiguousNodes;
        btOptimizedBvhNodeFloatData *m_contiguousNodesPtr;
        btQuantizedBvhNodeData *     m_quantizedContiguousNodesPtr;
        btBvhSubtreeInfoData *       m_subTreeInfoPtr;
        int                          m_traversalMode;
        int                          m_numSubtreeHeaders;
    };

    struct btOptimizedBvhNodeDoubleData
    {
        btVector3DoubleData m_aabbMinOrg;
        btVector3DoubleData m_aabbMaxOrg;
        int                 m_escapeIndex;
        int                 m_subPart;
        int                 m_triangleIndex;
        char                m_pad[4];
    };

    struct btOptimizedBvhNodeFloatData
    {
        btVector3FloatData m_aabbMinOrg;
        btVector3FloatData m_aabbMaxOrg;
        int                m_escapeIndex;
        int                m_subPart;
        int                m_triangleIndex;
        char               m_pad[4];
    };

    struct btMatrix3x3DoubleData
    {
        btVector3DoubleData m_el[3];
    };

    struct btMatrix3x3FloatData
    {
        btVector3FloatData m_el[3];
    };

    struct btPhysicsSystem
    {
        PointerArray m_collisionShapes;
        PointerArray m_collisionObjects;
        PointerArray m_constraints;
    };

    struct btMultiSphereShapeData
    {
        btConvexInternalShapeData m_convexInternalShapeData;
        btPositionAndRadius *     m_localPositionArrayPtr;
        int                       m_localPositionArraySize;
        char                      m_padding[4];
    };

    struct btTriangleMeshShapeData
    {
        btCollisionShapeData        m_collisionShapeData;
        btStridingMeshInterfaceData m_meshInterface;
        btQuantizedBvhFloatData *   m_quantizedFloatBvh;
        btQuantizedBvhDoubleData *  m_quantizedDoubleBvh;
        btTriangleInfoMapData *     m_triangleInfoMap;
        float                       m_collisionMargin;
        char                        m_pad3[4];
    };

    struct btScaledTriangleMeshShapeData
    {
        btTriangleMeshShapeData m_trimeshShapeData;
        btVector3FloatData      m_localScaling;
    };

    struct btTransformFloatData
    {
        btMatrix3x3FloatData m_basis;
        btVector3FloatData   m_origin;
    };

    struct btCompoundShapeChildData
    {
        btTransformFloatData  m_transform;
        btCollisionShapeData *m_childShape;
        int                   m_childShapeType;
        float                 m_childMargin;
    };

    struct btCylinderShapeData
    {
        btConvexInternalShapeData m_convexInternalShapeData;
        int                       m_upAxis;
        char                      m_padding[4];
    };

    struct btConeShapeData
    {
        btConvexInternalShapeData m_convexInternalShapeData;
        int                       m_upIndex;
        char                      m_padding[4];
    };

    struct btCapsuleShapeData
    {
        btConvexInternalShapeData m_convexInternalShapeData;
        int                       m_upAxis;
        char                      m_padding[4];
    };

    struct btGImpactMeshShapeData
    {
        btCollisionShapeData        m_collisionShapeData;
        btStridingMeshInterfaceData m_meshInterface;
        btVector3FloatData          m_localScaling;
        float                       m_collisionMargin;
        int                         m_gimpactSubType;
    };

    struct btConvexHullShapeData
    {
        btConvexInternalShapeData m_convexInternalShapeData;
        btVector3FloatData *      m_unscaledPointsFloatPtr;
        btVector3DoubleData *     m_unscaledPointsDoublePtr;
        int                       m_numUnscaledPoints;
        char                      m_padding3[4];
    };

    struct btTransformDoubleData
    {
        btMatrix3x3DoubleData m_basis;
        btVector3DoubleData   m_origin;
    };

    struct btCollisionObjectFloatData
    {
        void *                m_broadphaseHandle;
        void *                m_collisionShape;
        btCollisionShapeData *m_rootCollisionShape;
        char *                m_name;
        btTransformFloatData  m_worldTransform;
        btTransformFloatData  m_interpolationWorldTransform;
        btVector3FloatData    m_interpolationLinearVelocity;
        btVector3FloatData    m_interpolationAngularVelocity;
        btVector3FloatData    m_anisotropicFriction;
        float                 m_contactProcessingThreshold;
        float                 m_deactivationTime;
        float                 m_friction;
        float                 m_rollingFriction;
        float                 m_contactDamping;
        float                 m_contactStiffness;
        float                 m_restitution;
        float                 m_hitFraction;
        float                 m_ccdSweptSphereRadius;
        float                 m_ccdMotionThreshold;
        int                   m_hasAnisotropicFriction;
        int                   m_collisionFlags;
        int                   m_islandTag1;
        int                   m_companionId;
        int                   m_activationState1;
        int                   m_internalType;
        int                   m_checkCollideWith;
        int                   m_collisionFilterGroup;
        int                   m_collisionFilterMask;
        int                   m_uniqueId;
    };

    struct btRigidBodyFloatData
    {
        btCollisionObjectFloatData m_collisionObjectData;
        btMatrix3x3FloatData       m_invInertiaTensorWorld;
        btVector3FloatData         m_linearVelocity;
        btVector3FloatData         m_angularVelocity;
        btVector3FloatData         m_angularFactor;
        btVector3FloatData         m_linearFactor;
        btVector3FloatData         m_gravity;
        btVector3FloatData         m_gravity_acceleration;
        btVector3FloatData         m_invInertiaLocal;
        btVector3FloatData         m_totalForce;
        btVector3FloatData         m_totalTorque;
        float                      m_inverseMass;
        float                      m_linearDamping;
        float                      m_angularDamping;
        float                      m_additionalDampingFactor;
        float                      m_additionalLinearDampingThresholdSqr;
        float                      m_additionalAngularDampingThresholdSqr;
        float                      m_additionalAngularDampingFactor;
        float                      m_linearSleepingThreshold;
        float                      m_angularSleepingThreshold;
        int                        m_additionalDamping;
    };

    struct btCollisionObjectDoubleData
    {
        void *                m_broadphaseHandle;
        void *                m_collisionShape;
        btCollisionShapeData *m_rootCollisionShape;
        char *                m_name;
        btTransformDoubleData m_worldTransform;
        btTransformDoubleData m_interpolationWorldTransform;
        btVector3DoubleData   m_interpolationLinearVelocity;
        btVector3DoubleData   m_interpolationAngularVelocity;
        btVector3DoubleData   m_anisotropicFriction;
        double                m_contactProcessingThreshold;
        double                m_deactivationTime;
        double                m_friction;
        double                m_rollingFriction;
        double                m_contactDamping;
        double                m_contactStiffness;
        double                m_restitution;
        double                m_hitFraction;
        double                m_ccdSweptSphereRadius;
        double                m_ccdMotionThreshold;
        int                   m_hasAnisotropicFriction;
        int                   m_collisionFlags;
        int                   m_islandTag1;
        int                   m_companionId;
        int                   m_activationState1;
        int                   m_internalType;
        int                   m_checkCollideWith;
        int                   m_collisionFilterGroup;
        int                   m_collisionFilterMask;
        int                   m_uniqueId;
    };

    struct btRigidBodyDoubleData
    {
        btCollisionObjectDoubleData m_collisionObjectData;
        btMatrix3x3DoubleData       m_invInertiaTensorWorld;
        btVector3DoubleData         m_linearVelocity;
        btVector3DoubleData         m_angularVelocity;
        btVector3DoubleData         m_angularFactor;
        btVector3DoubleData         m_linearFactor;
        btVector3DoubleData         m_gravity;
        btVector3DoubleData         m_gravity_acceleration;
        btVector3DoubleData         m_invInertiaLocal;
        btVector3DoubleData         m_totalForce;
        btVector3DoubleData         m_totalTorque;
        double                      m_inverseMass;
        double                      m_linearDamping;
        double                      m_angularDamping;
        double                      m_additionalDampingFactor;
        double                      m_additionalLinearDampingThresholdSqr;
        double                      m_additionalAngularDampingThresholdSqr;
        double                      m_additionalAngularDampingFactor;
        double                      m_linearSleepingThreshold;
        double                      m_angularSleepingThreshold;
        int                         m_additionalDamping;
        char                        m_padding[4];
    };

    struct btHingeConstraintDoubleData
    {
        btTypedConstraintData m_typeConstraintData;
        btTransformDoubleData m_rbAFrame;
        btTransformDoubleData m_rbBFrame;
        int                   m_useReferenceFrameA;
        int                   m_angularOnly;
        int                   m_enableAngularMotor;
        float                 m_motorTargetVelocity;
        float                 m_maxMotorImpulse;
        float                 m_lowerLimit;
        float                 m_upperLimit;
        float                 m_limitSoftness;
        float                 m_biasFactor;
        float                 m_relaxationFactor;
    };

    struct btHingeConstraintFloatData
    {
        btTypedConstraintData m_typeConstraintData;
        btTransformFloatData  m_rbAFrame;
        btTransformFloatData  m_rbBFrame;
        int                   m_useReferenceFrameA;
        int                   m_angularOnly;
        int                   m_enableAngularMotor;
        float                 m_motorTargetVelocity;
        float                 m_maxMotorImpulse;
        float                 m_lowerLimit;
        float                 m_upperLimit;
        float                 m_limitSoftness;
        float                 m_biasFactor;
        float                 m_relaxationFactor;
    };

    struct btHingeConstraintDoubleData2
    {
        btTypedConstraintDoubleData m_typeConstraintData;
        btTransformDoubleData       m_rbAFrame;
        btTransformDoubleData       m_rbBFrame;
        int                         m_useReferenceFrameA;
        int                         m_angularOnly;
        int                         m_enableAngularMotor;
        double                      m_motorTargetVelocity;
        double                      m_maxMotorImpulse;
        double                      m_lowerLimit;
        double                      m_upperLimit;
        double                      m_limitSoftness;
        double                      m_biasFactor;
        double                      m_relaxationFactor;
        char                        m_padding1[4];
    };

    struct btConeTwistConstraintDoubleData
    {
        btTypedConstraintDoubleData m_typeConstraintData;
        btTransformDoubleData       m_rbAFrame;
        btTransformDoubleData       m_rbBFrame;
        double                      m_swingSpan1;
        double                      m_swingSpan2;
        double                      m_twistSpan;
        double                      m_limitSoftness;
        double                      m_biasFactor;
        double                      m_relaxationFactor;
        double                      m_damping;
    };

    struct btConeTwistConstraintData
    {
        btTypedConstraintData m_typeConstraintData;
        btTransformFloatData  m_rbAFrame;
        btTransformFloatData  m_rbBFrame;
        float                 m_swingSpan1;
        float                 m_swingSpan2;
        float                 m_twistSpan;
        float                 m_limitSoftness;
        float                 m_biasFactor;
        float                 m_relaxationFactor;
        float                 m_damping;
        char                  m_pad[4];
    };

    struct btGeneric6DofConstraintData
    {
        btTypedConstraintData m_typeConstraintData;
        btTransformFloatData  m_rbAFrame;
        btTransformFloatData  m_rbBFrame;
        btVector3FloatData    m_linearUpperLimit;
        btVector3FloatData    m_linearLowerLimit;
        btVector3FloatData    m_angularUpperLimit;
        btVector3FloatData    m_angularLowerLimit;
        int                   m_useLinearReferenceFrameA;
        int                   m_useOffsetForConstraintFrame;
    };

    struct btGeneric6DofSpringConstraintData
    {
        btGeneric6DofConstraintData m_6dofData;
        int                         m_springEnabled[6];
        float                       m_equilibriumPoint[6];
        float                       m_springStiffness[6];
        float                       m_springDamping[6];
    };

    struct btGeneric6DofConstraintDoubleData2
    {
        btTypedConstraintDoubleData m_typeConstraintData;
        btTransformDoubleData       m_rbAFrame;
        btTransformDoubleData       m_rbBFrame;
        btVector3DoubleData         m_linearUpperLimit;
        btVector3DoubleData         m_linearLowerLimit;
        btVector3DoubleData         m_angularUpperLimit;
        btVector3DoubleData         m_angularLowerLimit;
        int                         m_useLinearReferenceFrameA;
        int                         m_useOffsetForConstraintFrame;
    };

    struct btGeneric6DofSpringConstraintDoubleData2
    {
        btGeneric6DofConstraintDoubleData2 m_6dofData;
        int                                m_springEnabled[6];
        double                             m_equilibriumPoint[6];
        double                             m_springStiffness[6];
        double                             m_springDamping[6];
    };

    struct btGeneric6DofSpring2ConstraintData
    {
        btTypedConstraintData m_typeConstraintData;
        btTransformFloatData  m_rbAFrame;
        btTransformFloatData  m_rbBFrame;
        btVector3FloatData    m_linearUpperLimit;
        btVector3FloatData    m_linearLowerLimit;
        btVector3FloatData    m_linearBounce;
        btVector3FloatData    m_linearStopERP;
        btVector3FloatData    m_linearStopCFM;
        btVector3FloatData    m_linearMotorERP;
        btVector3FloatData    m_linearMotorCFM;
        btVector3FloatData    m_linearTargetVelocity;
        btVector3FloatData    m_linearMaxMotorForce;
        btVector3FloatData    m_linearServoTarget;
        btVector3FloatData    m_linearSpringStiffness;
        btVector3FloatData    m_linearSpringDamping;
        btVector3FloatData    m_linearEquilibriumPoint;
        char                  m_linearEnableMotor[4];
        char                  m_linearServoMotor[4];
        char                  m_linearEnableSpring[4];
        char                  m_linearSpringStiffnessLimited[4];
        char                  m_linearSpringDampingLimited[4];
        char                  m_padding1[4];
        btVector3FloatData    m_angularUpperLimit;
        btVector3FloatData    m_angularLowerLimit;
        btVector3FloatData    m_angularBounce;
        btVector3FloatData    m_angularStopERP;
        btVector3FloatData    m_angularStopCFM;
        btVector3FloatData    m_angularMotorERP;
        btVector3FloatData    m_angularMotorCFM;
        btVector3FloatData    m_angularTargetVelocity;
        btVector3FloatData    m_angularMaxMotorForce;
        btVector3FloatData    m_angularServoTarget;
        btVector3FloatData    m_angularSpringStiffness;
        btVector3FloatData    m_angularSpringDamping;
        btVector3FloatData    m_angularEquilibriumPoint;
        char                  m_angularEnableMotor[4];
        char                  m_angularServoMotor[4];
        char                  m_angularEnableSpring[4];
        char                  m_angularSpringStiffnessLimited[4];
        char                  m_angularSpringDampingLimited[4];
        int                   m_rotateOrder;
    };

    struct btGeneric6DofSpring2ConstraintDoubleData2
    {
        btTypedConstraintDoubleData m_typeConstraintData;
        btTransformDoubleData       m_rbAFrame;
        btTransformDoubleData       m_rbBFrame;
        btVector3DoubleData         m_linearUpperLimit;
        btVector3DoubleData         m_linearLowerLimit;
        btVector3DoubleData         m_linearBounce;
        btVector3DoubleData         m_linearStopERP;
        btVector3DoubleData         m_linearStopCFM;
        btVector3DoubleData         m_linearMotorERP;
        btVector3DoubleData         m_linearMotorCFM;
        btVector3DoubleData         m_linearTargetVelocity;
        btVector3DoubleData         m_linearMaxMotorForce;
        btVector3DoubleData         m_linearServoTarget;
        btVector3DoubleData         m_linearSpringStiffness;
        btVector3DoubleData         m_linearSpringDamping;
        btVector3DoubleData         m_linearEquilibriumPoint;
        char                        m_linearEnableMotor[4];
        char                        m_linearServoMotor[4];
        char                        m_linearEnableSpring[4];
        char                        m_linearSpringStiffnessLimited[4];
        char                        m_linearSpringDampingLimited[4];
        char                        m_padding1[4];
        btVector3DoubleData         m_angularUpperLimit;
        btVector3DoubleData         m_angularLowerLimit;
        btVector3DoubleData         m_angularBounce;
        btVector3DoubleData         m_angularStopERP;
        btVector3DoubleData         m_angularStopCFM;
        btVector3DoubleData         m_angularMotorERP;
        btVector3DoubleData         m_angularMotorCFM;
        btVector3DoubleData         m_angularTargetVelocity;
        btVector3DoubleData         m_angularMaxMotorForce;
        btVector3DoubleData         m_angularServoTarget;
        btVector3DoubleData         m_angularSpringStiffness;
        btVector3DoubleData         m_angularSpringDamping;
        btVector3DoubleData         m_angularEquilibriumPoint;
        char                        m_angularEnableMotor[4];
        char                        m_angularServoMotor[4];
        char                        m_angularEnableSpring[4];
        char                        m_angularSpringStiffnessLimited[4];
        char                        m_angularSpringDampingLimited[4];
        int                         m_rotateOrder;
    };

    struct btSliderConstraintData
    {
        btTypedConstraintData m_typeConstraintData;
        btTransformFloatData  m_rbAFrame;
        btTransformFloatData  m_rbBFrame;
        float                 m_linearUpperLimit;
        float                 m_linearLowerLimit;
        float                 m_angularUpperLimit;
        float                 m_angularLowerLimit;
        int                   m_useLinearReferenceFrameA;
        int                   m_useOffsetForConstraintFrame;
    };

    struct btSliderConstraintDoubleData
    {
        btTypedConstraintDoubleData m_typeConstraintData;
        btTransformDoubleData       m_rbAFrame;
        btTransformDoubleData       m_rbBFrame;
        double                      m_linearUpperLimit;
        double                      m_linearLowerLimit;
        double                      m_angularUpperLimit;
        double                      m_angularLowerLimit;
        int                         m_useLinearReferenceFrameA;
        int                         m_useOffsetForConstraintFrame;
    };

    struct SoftRigidAnchorData
    {
        btMatrix3x3FloatData m_c0;
        btVector3FloatData   m_c1;
        btVector3FloatData   m_localFrame;
        btRigidBodyData *    m_rigidBody;
        int                  m_nodeIndex;
        float                m_c2;
    };

    struct SoftBodyPoseData
    {
        btMatrix3x3FloatData m_rot;
        btMatrix3x3FloatData m_scale;
        btMatrix3x3FloatData m_aqq;
        btVector3FloatData   m_com;
        btVector3FloatData * m_positions;
        float *              m_weights;
        int                  m_numPositions;
        int                  m_numWeigts;
        int                  m_bvolume;
        int                  m_bframe;
        float                m_restVolume;
        int                  m_pad;
    };

    struct SoftBodyClusterData
    {
        btTransformFloatData m_framexform;
        btMatrix3x3FloatData m_locii;
        btMatrix3x3FloatData m_invwi;
        btVector3FloatData   m_com;
        btVector3FloatData   m_vimpulses[2];
        btVector3FloatData   m_dimpulses[2];
        btVector3FloatData   m_lv;
        btVector3FloatData   m_av;
        btVector3FloatData * m_framerefs;
        int *                m_nodeIndices;
        float *              m_masses;
        int                  m_numFrameRefs;
        int                  m_numNodes;
        int                  m_numMasses;
        float                m_idmass;
        float                m_imass;
        int                  m_nvimpulses;
        int                  m_ndimpulses;
        float                m_ndamping;
        float                m_ldamping;
        float                m_adamping;
        float                m_matching;
        float                m_maxSelfCollisionImpulse;
        float                m_selfCollisionImpulseFactor;
        int                  m_containsAnchor;
        int                  m_collide;
        int                  m_clusterIndex;
    };

    struct btSoftBodyFloatData
    {
        btCollisionObjectFloatData m_collisionObjectData;
        SoftBodyPoseData *         m_pose;
        SoftBodyMaterialData **    m_materials;
        SoftBodyNodeData *         m_nodes;
        SoftBodyLinkData *         m_links;
        SoftBodyFaceData *         m_faces;
        SoftBodyTetraData *        m_tetrahedra;
        SoftRigidAnchorData *      m_anchors;
        SoftBodyClusterData *      m_clusters;
        btSoftBodyJointData *      m_joints;
        int                        m_numMaterials;
        int                        m_numNodes;
        int                        m_numLinks;
        int                        m_numFaces;
        int                        m_numTetrahedra;
        int                        m_numAnchors;
        int                        m_numClusters;
        int                        m_numJoints;
        SoftBodyConfigData         m_config;
    };

    struct btMultiBodyLinkColliderFloatData
    {
        btCollisionObjectFloatData m_colObjData;
        btMultiBodyFloatData *     m_multiBody;
        int                        m_link;
        char                       m_padding[4];
    };

    struct btMultiBodyLinkColliderDoubleData
    {
        btCollisionObjectDoubleData m_colObjData;
        btMultiBodyDoubleData *     m_multiBody;
        int                         m_link;
        char                        m_padding[4];
    };

#pragma endregion

}  // namespace BulletFile
#endif  //_BulletFile_h_
