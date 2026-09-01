3D Project made by 
Emma Oliv
Felix Ortner

Key Binds:
WASD - Move around
SPACE / CRTL - Up / Down
ARROWS - Look around

Toggle on/off:
P - Particles
T - Tesselation
X - Wireframe
O - Shadows
C - (Printing coordinates to the terminal)

Rendering modes:
1 - Standard
2 - Position G-buffer
3 - Normal G-buffer
4 - Diffuse G-buffer
5 - Ambient
6 - Specular

Run .bat to run the project.


Deferred Rendering
All the objects in the scene are rendered using deferred rendering, using DeferredPS.hlsl and ComputeShader.hlsl.

Shadow Mapping
In the scene, we have four lights: one directional light comming straight from above, pointing straight down, and three spotlights whose position and direction is marked with a flashlight object. The casted shadows can be seen on the ground in various darkness levels depending on light intensity/amount of lights casting shadows in the same spot. Can be turned on and off by pressing 'O'.

Tessellation
Tessellation are applied on the ico-sphears and the morel mushrooms, where the LOD can easily be seen by toggling the wireframe ('X'), tessellation ('T'), and/or moving further away/closer to the objects.

OBJ-parser
We imported a OBJ-parser we found on GitHub. However, it is a bit superfluous, reading in more data than neccessary, making the start-up of the program slower than desired.

DCEM
You can see this technique by observing the ico-sphears in the scene. 

Frustum Culling / Quadtree
The frustum culling can be seen by going forward (from the start-position) and seeing the shadows disappear. It can also be seen by uncommenting row 170 (and commenting out row 169)in renderer.cpp (
169    pCamera = camera.GetConstantBuffer();
170    //pCamera = scene.GetShadowCamera(0);
) and move around while viewing it from another angle. It can be difficult to pin point your exact location, but the culling is clearly visible.


Billboarded Particles
The particles are placed above the house, comming from the chimney. Can be toggled on/off pressing 'P'.

Normal Mapping / Parallax Occlusion Mapping
These techniques can be viewed on the cube above the fountain. By swapping rendering modes between standard ('1'), normal ('3'), and diffuse ('4'), you can clearly see the techiques working.

..\x64\Debug\ProjectRasterizer.exe