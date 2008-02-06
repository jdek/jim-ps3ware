struct BinaryParticle
{
    uint16 startFrame;
    uint16 endFrame;
    uint16 qwordOffset;
    uint8  qwordSize;
    uint8  flags;
};

struct ParticleHeader
{
    uint16 particles;
    uint16 loopFrame;
    uint32 trackSize;
    uint32 particleOffset;
    uint32 trackOffset;
};
