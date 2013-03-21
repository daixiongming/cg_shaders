
#include <GL/glew.h>
#include <GL/glut.h>

#include "scene.h"
#include "playercycler.h"
#include "linalg.h"
#include "model.h"
#include "GLSLProgram.h"

PlayerCycler::PlayerCycler(Scene *scene, Eigen::Vector4f pos, Eigen::Vector4f rot)
  : SceneNode(scene, OBJECT, pos, rot)
{
  // use rotation to calculate initial forward facing direction on the xz plane.
  this->forward_dir = Eigen::Vector2f(0.0, 1.0);

  this->model = load_model("test_objs/Monkey.obj", "NA");
  this->model.vbo_ids = new GLuint[3];

  prep_buffers(this->model);

  this->main_shader = new GLSLProgram("shaders/toon.vert", "shaders/toon.frag");
}

void PlayerCycler::step() {
  //TODO: look up real bike dimensions and speeds in meters, m/s.
  // TODO: put these into cycler class as private attributes.
  static float speed = 0.1f;
  const float max_speed = 2.0f;
  float roll_turn_factor = 2.0f;
  float roll_angle_delta = 5.0f;

  this->pos[0] += speed * this->forward_dir[0];
  this->pos[2] += speed * this->forward_dir[1];
  
  if (!(this->scene->key_states['w'] || this->scene->key_states['s'])) {
    // bleed off speed.
    speed *= 0.99f;
  }

  if (!(this->scene->key_states['a'] || this->scene->key_states['d'])) {
    //bleed off roll.
    this->rot[2] *= 0.95L;
  }

  // check for and handle key presses
  if (this->scene->key_states['w']) {
    if (speed < max_speed) {
      speed += 0.02f;
    } else {
      speed += 0.002f;
    }
  }

  if (this->scene->key_states['s']) {
    if (speed > -max_speed) {
      speed -= 0.01f;
    } else {
      speed -= 0.001f;
    }
  }

  float rot_y = roll_turn_factor * this->rot[2] / 30.0L;
  this->rot[1] -= rot_y;
  float tadr = rot_y * PI/180L;

  this->forward_dir[0] = this->forward_dir[0] * cos(tadr)
    - this->forward_dir[1] * sin(tadr);
  this->forward_dir[1] = this->forward_dir[0] * sin(tadr)
    + this->forward_dir[1] * cos(tadr);

  // normalize forward_dir floating point roundoff causes drift towards zero vector.
  //TODO: REVAMP ENGINE TO USE GLM VECs or at least GLfloats instead of float/float.
  normalize2L(forward_dir);

  if (this->scene->key_states['a']) {
    if (this->rot[2] > -30.0L) {
      this->rot[2] -= roll_angle_delta;
    } else if (this->rot[2] < -30.0L && this->rot[2] > -45.0L) {
      this->rot[2] -= roll_angle_delta / (45.0L - this->rot[2]);
    }
  }

  if (this->scene->key_states['d']) {
    if (this->rot[2] < 30.0L) {
      this->rot[2] += roll_angle_delta;
    } else if (this->rot[2] > 30.0L && this->rot[2] < 45.0L) {
      this->rot[2] += roll_angle_delta / (45.0L + this->rot[2]);
    }
  }
}

void PlayerCycler::render() {
  glTranslatef(this->pos[0], this->pos[1], this->pos[2]);

  // need bike to turn along plane first
  glRotatef(this->rot[1], 0.0, 1.0, 0.0);

  //glRotatef(this->rot[0], 1.0, 0.0, 0.0);
  glRotatef(this->rot[2], 0.0, 0.0, 1.0);
  
  // use program.
  this->main_shader->enable();
  // TODO: implement some checks on the light sources to make sure they are 
  // there and logic to handle the case when they're not.
  SceneNode *light = this->scene->lights[0];

  // TODO: make these take in Eigen's vectors, use overloaded function defs.
  this->main_shader->set_uniform_3f("lightDir", light->pos[0], light->pos[1], light->pos[2]);
  
  glPushMatrix();
  draw_model(this->model);
  glPopMatrix();
  
  this->main_shader->disable();
}

