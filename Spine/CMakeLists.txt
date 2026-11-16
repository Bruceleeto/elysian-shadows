# Spine runtime (skeletal animation), rendered through libGyro.

add_library(Spine STATIC
    src/spine/Animation.c
    src/spine/AnimationState.c
    src/spine/AnimationStateData.c
    src/spine/Atlas.c
    src/spine/AtlasAttachmentLoader.c
    src/spine/Attachment.c
    src/spine/AttachmentLoader.c
    src/spine/Bone.c
    src/spine/BoneData.c
    src/spine/BoundingBoxAttachment.c
    src/spine/Event.c
    src/spine/EventData.c
    src/spine/extension.c
    src/spine/IkConstraint.c
    src/spine/IkConstraintData.c
    src/spine/Json.c
    src/spine/MeshAttachment.c
    src/spine/RegionAttachment.c
    src/spine/Skeleton.c
    src/spine/SkeletonBounds.c
    src/spine/SkeletonData.c
    src/spine/SkeletonJson.c
    src/spine/Skin.c
    src/spine/SkinnedMeshAttachment.c
    src/spine/Slot.c
    src/spine/SlotData.c
    src/spine/spine-libgyro.c)

set_target_properties(Spine PROPERTIES C_STANDARD 99 C_EXTENSIONS OFF)
target_compile_definitions(Spine PRIVATE ELYSIAN_DEREESTING ELYSIAN_RUNTIME_CHECKS)
target_include_directories(Spine
    PUBLIC include
    PRIVATE ${PROJECT_SOURCE_DIR}/libGyro/include ${PROJECT_SOURCE_DIR}/libGyro/api)
