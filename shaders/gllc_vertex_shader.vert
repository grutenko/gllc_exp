#version 330 core

// Позиция вершины
layout(location = 0) in vec2 aPos;

uniform mat4 uMVP;

const uint FLAG_POINT_SCALE_INVARIANT = 1u << 0;

uniform vec4 uColor;
uniform vec2 uViewport;
uniform float uScale;
uniform uint uFlags;
uniform vec2 uCenterPoint;

out vec4 vColor;

void main()
{
        vec2 pos = aPos;

        if ((uFlags & FLAG_POINT_SCALE_INVARIANT) != 0u)
        {
                pos = uCenterPoint + ((pos - uCenterPoint) * uScale);
        }

        gl_Position = uMVP * vec4(pos, 0.0, 1.0);

        gl_PointSize = 5.0;
        vColor = uColor;
}