#include <iostream>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>

#include <ppgso/ppgso.h>

#include <shaders/texture_vert_glsl.h>
#include <shaders/texture_frag_glsl.h>

const unsigned int SIZE = 512;

// Object to represent Bezier patch
class BezierPatch {
private:
    std::vector<glm::vec3> vertices;       // Points/vertices of the shape
    std::vector<glm::vec3> originalVertices;
    std::vector<glm::vec2> texCoords;      // Texture coordinates
    std::vector<GLuint> indices;           // Indices for triangles

    GLuint vao, vbo, tbo, ibo;
    glm::mat4 modelMatrix{1.0f};

    glm::vec3 bezierPoint(const glm::vec3 controlPoints[4], float t) {
        glm::vec3 a = lerp(controlPoints[0], controlPoints[1], t);
        glm::vec3 b = lerp(controlPoints[1], controlPoints[2], t);
        glm::vec3 c = lerp(controlPoints[2], controlPoints[3], t);
        glm::vec3 d = lerp(a, b, t);
        glm::vec3 e = lerp(b, c, t);
        return lerp(d, e, t);
    }

  ppgso::Shader program{texture_vert_glsl, texture_frag_glsl};
  ppgso::Texture texture{ppgso::image::loadBMP("lena.bmp")};
public:
  // Public attributes that define position, color ..
  glm::vec3 position{0,0,0};
  glm::vec3 rotation{0,0,0};
  glm::vec3 scale{1,1,1};

  BezierPatch(const glm::vec3 controlPoints[4][4]) {
      unsigned int PATCH_SIZE = 10;
      for(unsigned int i = 0; i < PATCH_SIZE ; i++) {
          for (unsigned int j = 0; j < PATCH_SIZE; j++) {
              float u = (float)i / (PATCH_SIZE - 1);
              float v = (float)j / (PATCH_SIZE - 1);

              glm::vec3 rowPoints[4];
              for (int k = 0; k < 4; k++)
              {
                  rowPoints[k] = bezierPoint(controlPoints[k], u);
              }
              vertices.push_back(bezierPoint(rowPoints, v));
              texCoords.push_back({u, v});
          }
      }

      originalVertices = vertices;

      for(unsigned int i = 0; i < PATCH_SIZE - 1; i++) {
          for (unsigned int j = 0; j < PATCH_SIZE - 1; j++) {
              GLuint v0 = i * PATCH_SIZE + j;
              GLuint v1 = i * PATCH_SIZE + (j + 1);
              GLuint v2 = (i + 1) * PATCH_SIZE + (j + 1);
              GLuint v3 = (i + 1) * PATCH_SIZE + j;

              // First triangle
              indices.push_back(v0);
              indices.push_back(v1);
              indices.push_back(v2);

              // Second triangle
              indices.push_back(v0);
              indices.push_back(v2);
              indices.push_back(v3);
          }
      }

    // Copy data to OpenGL
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Copy positions to gpu
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

    // Set vertex program inputs
    auto position_attrib = program.getAttribLocation("Position");
    glEnableVertexAttribArray(position_attrib);
    glVertexAttribPointer(position_attrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // Copy texture positions to gpu
    glGenBuffers(1, &tbo);
    glBindBuffer(GL_ARRAY_BUFFER, tbo);
    glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(glm::vec2), texCoords.data(), GL_STATIC_DRAW);

    // Set vertex program inputs
    auto texCoord_attrib = program.getAttribLocation("TexCoord");
    glEnableVertexAttribArray(texCoord_attrib);
    glVertexAttribPointer(texCoord_attrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
    }

    ~BezierPatch() {
        glDeleteBuffers(1, &ibo);
        glDeleteBuffers(1, &tbo);
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
    }

    void update(float time) {
        for (size_t i = 0; i < vertices.size(); i++)
        {
            glm::vec3 pos = originalVertices[i];

            // Wave params
            float waveHeight = 0.1f;      // Height
            float waveFrequency = 2.0f;   // Frequency

            pos.y += waveHeight * glm::sin(waveFrequency * pos.x + time);

            vertices[i] = pos;
        }

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(glm::vec3), vertices.data());

        modelMatrix = glm::translate(glm::mat4{1.0f}, position) *
                      glm::eulerAngleYXZ(rotation.y, rotation.x, rotation.z) *
                      glm::scale(glm::mat4{1.0f}, scale);
    }


    void render(){
      program.use();

    // Initialize projection
    // Create projection matrix (field of view (radians), aspect ratio, near plane distance, far plane distance)
    // You can think of this as the camera objective settings
    auto projection = glm::perspective( (ppgso::PI/180.f) * 60.0f, 1.0f, 0.1f, 10.0f);
    program.setUniform("ProjectionMatrix", projection);

    // Create view matrix (translate camera a bit backwards, so we can see the geometry)
    // This can be seen as the camera position/rotation in space
    auto view = glm::translate(glm::mat4{}, {0.0f, 0.0f, -3.0f});
    program.setUniform("ViewMatrix", view);

    // Set model position
    program.setUniform("ModelMatrix", modelMatrix);

    // Bind texture
    program.setUniform("Texture", texture);

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
  };
};

class BezierSurfaceWindow : public ppgso::Window {
private:
  // Define 16 control points
  glm::vec3 controlPoints[4][4]{
      { {-1,1,0}, {-0.5,1,0}, {.5,1,0}, {1,1,3}, },
      { {-1,.5,0}, {-0.5,.5,0}, {.5,.5,0}, {1,.5,0}, },
      { {-1,-.5,0}, {-0.5,-.5,0}, {.5,-.5,0}, {1,-.5,-1}, },
      { {-1,-1,3}, {-0.5,-1,0}, {.5,-1,0}, {1,-1,0}, },
  };

  BezierPatch bezier{controlPoints};
public:
  BezierSurfaceWindow() : Window{"task6_bezier_surface", SIZE, SIZE} {
    // Initialize OpenGL state
    // Enable Z-buffer
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
  }

  void onIdle() final {
    // Set gray background
    glClearColor(.1f,.1f,.1f,1.0f);

    // Clear depth and color buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto time = static_cast<float>(glfwGetTime());
    bezier.rotation = { time * 0.1f, time * 0.1f, time * 0.1f };

    bezier.update(time);
    bezier.render();
  }
};

int main() {
  // Create new window
  auto window = BezierSurfaceWindow{};

  // Main execution loop
  while (window.pollEvents()) {}

  return EXIT_SUCCESS;
}
