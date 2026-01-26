#version 330 core

// Позиция вершины
layout(location = 0) in vec2 aPos;

// Матрицы трансформации
uniform mat4 uMVP;      // Общая матрица

// Цвет вершины
uniform vec4 uColor;

// Передача цвета в фрагментный шейдер
out vec4 vColor;

void main()
{
        gl_Position = uMVP * vec4(aPos, 0.0, 1.0);
        vColor = uColor;
}