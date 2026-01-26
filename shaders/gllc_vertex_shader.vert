#version 330 core

// Позиция вершины
layout(location = 0) in vec2 aPos;

// Матрицы трансформации
uniform mat4 uModel;      // Модельная матрица
uniform mat4 uView;       // Матрица вида
uniform mat4 uProjection; // Проекционная матрица

// Цвет вершины
uniform vec4 uColor;

// Передача цвета в фрагментный шейдер
out vec4 vColor;

void main()
{
        // Преобразование позиции вершины в экранные координаты
        gl_Position = uProjection * uView * uModel * vec4(aPos, 0.0, 1.0);

        // Передача цвета во фрагментный шейдер
        vColor = uColor;
}