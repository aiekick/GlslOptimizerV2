# GlslOptimizer

GlslOptimizer is an optimizer for glsl language with support of last opengl api and Stages 
Vertex \ Tesselation Control \ Tesselation Evaluation \ Geometry \ Fragment

Based on [Mesa3D](https://www.mesa3d.org/) and [glsl-optimizer](https://github.com/aras-p/glsl-optimizer)

## Why ?

Some time ago i found the original glsl-optimizer, but this project is stopped since a while and without support of Tesselation and Geometry shader stage nor last Opengl Api.

After analysed it a bit, i discovered than most of the Optimization code is coming from Mesa3D Implementation. 
So after a bit of search, i decided to create an updated version of Gsl-Optimizer who can support last stages and last Opengl api too.

All this work is mostly based on Mesa3D and original Glsl-Optimizer, but all not work porperly for the moment.
I not guaranty than generated optimized shader can compile

I have exposed the most of params i have found in Mesa3D for better optimization possibilities, and proposed a GlslOptimizer module and an standalone App for optimize in an easy way your shaders.

I am very interested in graphic programming but as a self-learning guy, im in no way an expert.

So if you feel like it, help me improve this Software that I make available in open source for that.
