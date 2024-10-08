// Task 4 - Render the letter q using OpenGL
//        - Implement a function to generate bezier curve points
//        - Generate multiple points on each bezier curve
//        - Draw lines connecting the generated points using OpenGL

#include <iostream>
#include <vector>

#include <ppgso/ppgso.h>

#include <shaders/color_vert_glsl.h>
#include <shaders/color_frag_glsl.h>

const unsigned int SIZE = 512;

class BezierWindow : public ppgso::Window {
private:

    // Control points for the bezier curves
    // First curve is 4 control points
    // Rest of the curves are 3 control points, each reusing the last curve end
    // Defines the greek letter "q"
    std::vector<glm::vec2> controlPoints = {
            {0,     0},
            {-.5,   0},
            {-.5, -.7},
            {0,   -.7},
            {.5,  -.7},
            {.5,    0},
            {.0,   .2},
            {-.5,  .3},
            {-.5,  .7},
            {.5,   .7},
    };

    // This will hold the bezier curve geometry once we generate it
    std::vector<glm::vec3> points;

    // GLSL Program to use for rendering
    ppgso::Shader program = {color_vert_glsl, color_frag_glsl};

    // These numbers are used to pass buffer data to OpenGL
    GLuint vao = 0, vbo = 0;

    // Compute points for Bezier curve using 4 control points
    glm::vec2 bezierPoint(const glm::vec2 &p0, const glm::vec2 &p1, const glm::vec2 &p2, const glm::vec2 &p3, const float t) {
        glm::vec2 a = lerp( p0, p1, t );
        glm::vec2 b = lerp( p1, p2, t );
        glm::vec2 c = lerp( p2, p3, t );
        glm::vec2 d = lerp( a, b, t );
        glm::vec2 e = lerp( b, c, t );
        glm::vec2 point = lerp( d, e, t );
        return point;
    }

    // Compute points for a sequence of Bezier curves defined by a vector of control points
    // Each bezier curve will reuse the end point of the previous curve
    // count - Number of points to generate on each curve
    void bezierShape(int count) {
        for(int i = 0; i < (int) controlPoints.size() - 3; i+=3) {
            for (int j = 0; j <= count; j++) {
                float t = (float)j / (float)count;
                glm::vec2 point = bezierPoint(controlPoints[i], controlPoints[i + 1], controlPoints[i + 2], controlPoints[i + 3], t);
                points.emplace_back(point, 0.0f);
            }
        }
    }

public:
    BezierWindow() : Window{"task3_bezier", SIZE, SIZE} {
        // Generate Bezier curve points
        bezierShape(15);

        // Generate a vertex array object
        // This keeps track of what attributes are associated with buffers
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        // Generate a vertex buffer object, this will feed data to the vertex shader
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        // Pass the control points to the GPU
        glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec3), points.data(), GL_STATIC_DRAW);

        // Setup vertex array lookup, this tells the shader how to pick data for the "Position" input
        auto position_attrib = program.getAttribLocation("Position");
        glVertexAttribPointer(position_attrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(position_attrib);

        // Set model matrix to identity
        program.setUniform("ModelMatrix", glm::mat4{});
        program.setUniform("ViewMatrix", glm::mat4{});
        program.setUniform("ProjectionMatrix", glm::mat4{});

        // Set the color uniform
        program.setUniform("OverallColor", glm::vec3{1.0f,1.0f,1.0f});
    }

    ~BezierWindow() final {
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
    }

    void onIdle() final {
        // Set gray background
        glClearColor(0.5f,0.5f,0.5f,0);

        // Clear depth and color buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw shape
        glBindVertexArray(vao);

        // Define the correct render mode
        glDrawArrays(GL_LINE_STRIP, 0, points.size());
    }
};

int main() {
    // Create our window
    BezierWindow window;

    // Main execution loop
    while (window.pollEvents()) {}

    return EXIT_SUCCESS;
}
