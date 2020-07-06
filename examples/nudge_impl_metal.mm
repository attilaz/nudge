#include "nudge_impl_metal.h"


bool Nudge_ImplMetal_Init(id<MTLDevice> device)
{
    return false;
}
void Nudge_ImplMetal_Shutdown()
{
}

void Nudge_ImplMetal_NewFrame(MTLRenderPassDescriptor* renderPassDescriptor)
{
}

void Nudge_ImplMetal_RenderDrawData(id<MTLCommandBuffer> commandBuffer,
                                    id<MTLRenderCommandEncoder> commandEncoder)
{
}
