// Task 7 - Dynamically generate objects in a 3D scene
//        - Implement a particle system where particles have position and speed
//        - Any object can be a generator and can add objects to the scene
//        - Create dynamic effect such as fireworks, rain etc.
//        - Encapsulate camera in a class

#include <iostream>
#include <vector>
#include <map>
#include <list>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>

#include <ppgso/ppgso.h>

#include <shaders/color_vert_glsl.h>
#include <shaders/color_frag_glsl.h>
#include <random>

const unsigned int SIZE = 512;

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<> dist(-0.5f, 0.5f);
std::uniform_real_distribution<float> size_variation(0.2f, 0.5f);

glm::vec3 wind = glm::vec3(0.7f, 0.0f, 0.0f);

class Camera {
public:
  // Add parameters
  glm::mat4 viewMatrix;
  glm::mat4 projectionMatrix;
  glm::vec3 position;
  glm::vec3 target;
  glm::vec3 up;

  /// Representaiton of
  /// \param fov - Field of view (in degrees)
  /// \param ratio - Viewport ratio (width/height)
  /// \param near - Distance of the near clipping plane
  /// \param far - Distance of the far clipping plane
  Camera(float fov = 45.0f, float ratio = 1.0f, float near = 0.1f, float far = 10.0f) {
    // Initialize perspective projection (hint: glm::perspective)
    projectionMatrix = glm::perspective(glm::radians(fov), ratio, near, far);
    position = {0.0f, 0.0f, -5.0f};
    target = {0.0f, 0.0f, 0.0f};
    up = {0.0f, 1.0f, 0.0f};
    update();
  }

  /// Recalculate viewMatrix from position, rotation and scale
  void update() {
    // Update viewMatrix (hint: glm::lookAt)
      viewMatrix = glm::lookAt(position, target, up);
  }
};

/// Abstract renderable object interface
class Renderable; // Forward declaration for Scene
using Scene = std::list<std::unique_ptr<Renderable>>; // Type alias

class Renderable {
public:
  // Virtual destructor is needed for abstract interfaces
  virtual ~Renderable() = default;

  /// Render the object
  /// \param camera - Camera to use for rendering
  virtual void render(const Camera& camera) = 0;

  /// Update the object. Useful for specifing animation and behaviour.
  /// \param dTime - Time delta
  /// \param scene - Scene reference
  /// \return - Return true to keep object in scene
  virtual bool update(float dTime, Scene &scene) = 0;
};

enum class ParticleType { RED, YELLOW, SMOKE };
std::uniform_real_distribution<float> color_variation(0.0f, 0.6f);

/// Basic particle that will render a sphere
/// Implement Renderable particle
class Particle final : public Renderable {
  // Static resources shared between all particles
  static std::unique_ptr<ppgso::Mesh> mesh;
  static std::unique_ptr<ppgso::Shader> shader;

  // add more parameters as needed
  glm::vec3 position;
  glm::vec3 speed;
  glm::vec3 color;
  float lifetime;
  ParticleType type;
  float scale;
  glm::vec3 acceleration;

public:
  /// Construct a new Particle
  /// \param p - Initial position
  /// \param s - Initial speed
  /// \param c - Color of particle
  Particle(glm::vec3 p, glm::vec3 s, glm::vec3 c, float d, ParticleType particleType)
          : position(p), speed(s), lifetime(d), type(particleType) {
      if (!shader) shader = std::make_unique<ppgso::Shader>(color_vert_glsl, color_frag_glsl);
      if (!mesh) mesh = std::make_unique<ppgso::Mesh>("sphere.obj");

      switch (type) {
          case ParticleType::RED:
              color = glm::vec3(1.0f, color_variation(gen), color_variation(gen));
              break;
          case ParticleType::YELLOW:
              color = glm::vec3(1.0f, 1.0f - color_variation(gen) * 0.3f, color_variation(gen));
              break;
          case ParticleType::SMOKE:
              float gray = 0.3f + color_variation(gen) * 0.7f;
              color = glm::vec3(gray, gray, gray);
              break;
      }

      scale = size_variation(gen);
      position += glm::vec3(dist(gen), dist(gen), dist(gen));
      acceleration = glm::vec3(dist(gen) * 0.5f, 0.1f, dist(gen) * 0.5f);
  }

  bool update(float dTime, Scene &scene) override {
    // Animate position using speed and dTime.
    // - Return true to keep the object alive
    // - Returning false removes the object from the scene
    // - hint: you can add more particles to the scene here also

    speed += (acceleration + wind) * dTime;
    position += speed * dTime;
    lifetime -= dTime;

    if (lifetime <= 0.0f) {
        if (type == ParticleType::RED) {
            type = ParticleType::YELLOW;
            color = glm::vec3(1.0f, 1.0f - color_variation(gen) * 0.3f, color_variation(gen));
            speed = glm::vec3(dist(gen) * 0.2f, 0.7f, dist(gen) * 0.2f);
            lifetime = 3.0f;
        } else if (type == ParticleType::YELLOW) {
            type = ParticleType::SMOKE;
            float gray = 0.3f + color_variation(gen) * 0.7f;
            color = glm::vec3(gray, gray, gray);
            speed = glm::vec3(dist(gen) * 0.1f, 0.5f, dist(gen) * 0.1f);
            lifetime = 2.0f;
        } else {
            return false;
        }
    }

    return true;
  }

  void render(const Camera& camera) override {
    // Render the object
    // - Use the shader
    // - Setup all needed shader inputs
    // - hint: use OverallColor in the color_vert_glsl shader for color
    // - Render the mesh
    shader->use();
    shader->setUniform("ProjectionMatrix", camera.projectionMatrix);
    shader->setUniform("ViewMatrix", camera.viewMatrix);
    shader->setUniform("ModelMatrix", glm::translate(position) * glm::scale(glm::vec3(scale)));
    shader->setUniform("OverallColor", color);
    mesh->render();
  }
};
// Static resources need to be instantiated outside of the class as they are globals
std::unique_ptr<ppgso::Mesh> Particle::mesh;
std::unique_ptr<ppgso::Shader> Particle::shader;

class ParticleWindow : public ppgso::Window {
private:
  // Scene of objects
  Scene scene;

  // Create camera
  Camera camera = {120.0f, (float)width/(float)height, 1.0f, 400.0f};

  // Store keyboard state
  std::map<int, int> keys;
public:
  ParticleWindow() : Window{"task7_particles", SIZE, SIZE} {
    // Initialize OpenGL state
    // Enable Z-buffer
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
  }

  void onKey(int key, int scanCode, int action, int mods) override {
    // Collect key state in a map
    keys[key] = action;
    if (keys[GLFW_KEY_SPACE]) {
      // Add renderable object to the scene
      scene.push_back(std::make_unique<Particle>(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), 2.0f, ParticleType::RED));
    }
  }

  void onIdle() override {
    // Track time
    static auto time = (float) glfwGetTime();
    // Compute time delta
    float dTime = (float)glfwGetTime() - time;
    time = (float) glfwGetTime();

    // Set gray background
    glClearColor(.1f,.1f,.1f,1.0f);

    // Clear depth and color buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Update all objects in scene
    // Because we need to delete while iterating this is implemented using c++ iterators
    // In most languages mutating the container during iteration is undefined behaviour
    auto i = std::begin(scene);
    while (i != std::end(scene)) {
      // Update object and remove from list if needed
      auto obj = i->get();
      if (!obj->update(dTime, scene))
        i = scene.erase(i);
      else
        ++i;
    }

    // Render every object in scene
    for(auto& object : scene) {
      object->render(camera);
    }
  }
};

int main() {
  // Create new window
  auto window = ParticleWindow{};

  // Main execution loop
  while (window.pollEvents()) {}

  return EXIT_SUCCESS;
}
