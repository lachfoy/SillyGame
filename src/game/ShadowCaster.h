#pragma once

#include <glm/glm.hpp>

#include "../engine/Entity.h"
#include "../engine/Renderer.h"

class ShadowCaster : public Entity
{
  public:
	ShadowCaster();
	~ShadowCaster();

  protected:
	void drawShadow();

  private:
	Texture shadowTexture;
};
