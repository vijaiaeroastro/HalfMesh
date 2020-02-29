# HalfMesh
A Simple and flexible half edge data structure

This is a very simple Half Edge data structure implementation. It is currently only tailored for
triangular meshes. However, its quite straightforward to modify them for general polygonal surfaces.
Its relatively easy to implement most of the surface algorithms such as
* Mesh subdivision
* Smoothing
* Hole detection
* Discrete curvature computation
* Feature edge detection

and so on..

It has a simple property store based on JSON. It allows one to assign property to
vertex / edge / face in a mesh. There is a custom mesh format which uses bson and it has an
extension **.bm**. This format allows one to complete serialize the entire mesh along with
all property traits stored in a mesh.

# Input / Output Capabilities
* OBJ (Triangles only) - Read & Write
* GMSH (Version 2, Triangles only) - Read & Write
* VTK (Triangles only) - Write
* BM (Binary Mesh format) - Custom format that is used to read and write properties along with mesh

# Dependencies
C++11 

# License
MIT

# Notes
I will try to keep it free from clutter as much as possible and only depend on STL. 
But this may change in future. If you would like to see any specific geometry manipulation or
repair algorithm implemented, i would be more than happy to add it here in future. Just contact me.
