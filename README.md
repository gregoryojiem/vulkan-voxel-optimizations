## Sparse Voxel Octree Renderering, and Voxel Compression Techniques

This project is mainly for me to explore computer graphics programming, and to get more experience with the Vulkan API in C/C++. 

My current goal is implementing the sparse voxel octree (SVO) system described in, "Efficient Sparse Voxel Octrees – Analysis, Extensions, and Implementation" by [NVIDIA](https://www.nvidia.com/docs/io/88972/nvr-2010-001.pdf). I'm also interested in other compression techniques that can be used on voxel data structures such as [variable and fixed length encoding](https://ieeexplore.ieee.org/abstract/document/9378675), [directed acyclic graphs](https://doi.org/10.31577/cai_2020_3_587), and lossy compression.

### Current Goals

- [x] **Vulkan pipeline for rendering Voxels**: Functional, and mostly finished. My rendering system could be improved further through utilizing more GPU queues, and multi-threading some of the frame by frame calls.

- [x] **Octree chunking system:** I've implemented a relatively memory efficient sparse octree system. The next step is a pointerless SVO implementation using bit masks, and then improving the memory efficiency through compression and acyclic directed graphs.  

- [x] **Text rendering**: Finished for now. Text rendering has been a surprisingly difficult task, but important for debugging my code. Currently, I use a multi-channel signed distance field font atlas generated through [Viktor Chlumský's](https://github.com/Chlumsky/msdfgen) library. I'd like to add true alpha signed distance for text effects, but it's low priority. 

- [x] **Vertex pooling system:** Finished. It's important for performance reasons to minimize CPU - GPU memory transfer. With my VertexPool class, I'm able to dynamically assign memory ranges to chunk vertices, only updating the required memory ranges. 

- [ ] **Greedy octree meshing**: This involves developing binary greedy meshing algorithms for SVOs, intended to be compatible with pointerless SVOs in the future. Currently a WIP. 

### Future Goals

* **Raymarching octrees:** Many voxel engines opt to use ray-marching/ray-tracing due to the possible optimizations you can make with many voxel data structures. I'd like to do tests on the efficiency of a ray-marching rendering approach with a custom rasterizer/digital differential analyzer.

* **Physical and liquid simulations:** Experimenting with large-scale liquid and physics simulation techniques is a future goal of mine. [Grant Kot](https://www.youtube.com/@GrantKot) has shared many interesting techniques for deformable and liquid voxel physics that I'd like to look into.

* **GPU-side octrees, compute shaders:** While I still believe a multi-threaded CPU approach is optimal, it could be valuable to investigate storing voxel data entirely in GPU memory and manipulating it with compute shaders.
