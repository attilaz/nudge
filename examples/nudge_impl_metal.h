// nudge: Renderer for Metal

@class MTLRenderPassDescriptor;
@protocol MTLDevice, MTLCommandBuffer, MTLRenderCommandEncoder;

bool Nudge_ImplMetal_Init(id<MTLDevice> device);
void Nudge_ImplMetal_Shutdown();
void Nudge_ImplMetal_NewFrame(MTLRenderPassDescriptor* renderPassDescriptor);
void Nudge_ImplMetal_RenderDraw(id<MTLCommandBuffer> commandBuffer,
                                    id<MTLRenderCommandEncoder> commandEncoder,
                                    float* spheres,
                                    float* boxes);

