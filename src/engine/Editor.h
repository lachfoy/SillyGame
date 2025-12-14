#pragma once

#include <SDL3/SDL.h>
#include <memory>
#include <string>
#include <vector>

struct EditorTool
{
	explicit EditorTool(const std::string &_name) : name(_name) {}
	virtual ~EditorTool() = default;

	virtual void draw() = 0;

	bool open = false;
	std::string name;
};

class Editor
{
  public:
	void init(SDL_Window *window, SDL_GLContext glContext);
	void shutdown();

	template <typename T, typename... Args> void registerTool(Args &&...args)
	{
		static_assert(std::is_base_of_v<EditorTool, T>);
		mTools.push_back(std::make_unique<T>(std::forward<Args>(args)...));
	}

	void processEvent(const SDL_Event &e);

	void update(float dt);

	void beginFrame();
	void draw();
	void endFrame();

  private:
	std::vector<std::unique_ptr<EditorTool>> mTools;
};
