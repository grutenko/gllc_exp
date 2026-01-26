#version 330 core

// Входящий цвет от вершинного шейдера
in vec4 vColor;

// Цвет, который будет записан в буфер кадра
out vec4 FragColor;

void main()
{
        // Просто передаем цвет
        FragColor = vColor;
}
