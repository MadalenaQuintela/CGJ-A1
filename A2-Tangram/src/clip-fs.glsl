#version 460 core

// in vec4 exColor;
uniform vec4 PieceColor;
out vec4 outColor;

void main(void) {
    // outColor = exColor;
    outColor = PieceColor;
}
