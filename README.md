# Realtime Rendering Assessment 1 - Practical Artefact
heyyo!

## Usage/Controls
If you run the application as is, you should end up in a demo scene. On your right is a mini window showing some scene statistics. Once you focus the window, your mouse will be locked to the center.

**Use the mouse to look around, and WASD to walk.**

You can use the number keys (not the numpad) 1-6 to view different render passes:
1. ASCII shader full post processing
2. Untouched colour pass
3. Normal pass
4. Depth pass
5. Sharpened post processing
6. Ray-traced ambient occlusion pass

When you've had enough of looking around the scene on foot, you can **press TAB to enter debug/fly mode.**

When in debug/fly mode, you can move up and down (relative to the camera) using Q/E. **Hold shift to move faster.** You can use TAB to go back to walk mode (walk mode camera position is preserved).

While in debug/fly mode, you gain a debug overlay which is drawn on top of the post processing, showing the transforms of each object (emptys, meshes, cameras, lights).

## Features

### Lighting
- point, spot, and directional lights
- up to 8 lights in total
- shadow maps (functional for spot and directional lights)
- lights as scene objects (can be transformed, parented, etc)

### Shading
- Phong shading using albedo texture for surface colour
- use of both solid and wireframe rasterisers
- use of both triangle (for normal meshes) and line (for debug gizmos) assembler modes
- normal mapping using normal texture
- (INCOMPLETE) PBR uber-shader

### Scene Graph
- transform/object heirarchy
- transform class capable of translation, rotation, and scale, both in local and world space
- camera, light, and mesh classes

### Resource Manager
- custom OBJ file loader, capable of computing tangents for use with normal mapping
- material configuration system using JSON and DirectX11 shader reflection system
- scene loader from JSON file (includes scene graph, automatic loading of dependencies like textures, meshes, materials, shaders)
- custom JSON parser, used by above
- global resource manager class for managing loading/unloading GPU resources (textures, meshes, shaders) and materials, preventing duplication

### Miscellaneous Features
- full-screen-quad-based post-processing (including a sophisticated ASCII dithering shader)
- fog (implemented in post-processing)
- sharpen filter (implemented in post-processing)
- skybox (implemented in post-processing)
- support for multiple render passes sent to the post processing shader (colour, normal, depth, AO)
- object shader and material sorting to minimise context switches required
- (INCOMPLETE) frustrum culling based on object bounding boxes
- (INCOMPLETE) ambient occlusion using compute shaders
