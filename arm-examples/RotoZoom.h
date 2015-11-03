/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2012 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef ROTOZOOM_H
#define ROTOZOOM_H

#define GLES_VERSION 2 

#include <GLES2/gl2.h>

/* These indices describe the quad triangle strip. */
static const GLubyte quadIndices[] =
{
    0, 1, 2, 3,
};

/* Tri strips, so quad is in this order:
 *
 * 2 ----- 3
 * | \     |
 * |   \   |
 * |     \ |
 * 0 ----- 1
 */
static const float quadVertices[] =
{
    /* Front. */
    -1.0f, -1.0f,  0.0f, /* 0 */
     1.0f, -1.0f,  0.0f, /* 1 */
    -1.0f,  1.0f,  0.0f, /* 2 */
     1.0f,  1.0f,  0.0f, /* 3 */
};

static const float quadTextureCoordinates[] =
{
    /* Front. */
    0.0f, 1.0f, /* 0 */
    1.0f, 1.0f, /* 1 */
    0.0f, 0.0f, /* 2 */
    1.0f, 0.0f, /* 3 */ 
    /* Flipped Y coords. */
};

#endif /* ROTOZOOM_H */