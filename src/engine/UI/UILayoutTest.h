#pragma once

#include <cmath>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../EngineDefs.h"
#include "../Renderer.h"

struct Size
{
	float width;
	float height;
};

struct Rect
{
	float x;
	float y;
	float width;
	float height;
};

struct Thickness
{
	float bottom = 0.f;
	float left = 0.f;
	float right = 0.f;
	float top = 0.f;
};

inline Size Deflate(Size size, Thickness thick)
{
	return {std::max(0.f, size.width - thick.left - thick.right),
			std::max(0.f, size.height - thick.top - thick.bottom)};
}

inline Size Inflate(Size size, Thickness thick)
{
	return {std::max(0.f, size.width + thick.left + thick.right),
			std::max(0.f, size.height + thick.top + thick.bottom)};
}

inline Rect Deflate(Rect rect, Thickness thick)
{
	return {rect.x + thick.left, rect.y + thick.top,
			std::max(0.f, rect.width - thick.left - thick.right),
			std::max(0.f, rect.height - thick.top - thick.bottom)};
}

inline Rect Inflate(Rect rect, Thickness thick)
{
	return {rect.x - thick.left, rect.y - thick.top,
			std::max(0.f, rect.width + thick.left + thick.right),
			std::max(0.f, rect.height + thick.top + thick.bottom)};
}

class UIElement
{
  public:
	Size desiredSize; // result of Measure. Content size + margin
	Rect layoutRect;  // result of Arrange

	virtual ~UIElement() {}

	virtual void Measure(Size availableSize) = 0;
	virtual void Arrange(Rect finalRect) { layoutRect = finalRect; }

	virtual void Render(Renderer &renderer) = 0;

#if WITH_EDITOR
	class Panel *owningPanel = nullptr;
#endif
};

enum class HorizontalAlignment
{
	Left,
	Center,
	Right,
	Stretch
};

enum class VerticalAlignment
{
	Top,
	Center,
	Bottom,
	Stretch
};

class FrameworkElement : public UIElement
{
  public:
	float width = std::numeric_limits<float>::quiet_NaN();	// NaN = Auto
	float height = std::numeric_limits<float>::quiet_NaN(); // NaN = Auto

	Thickness margin;

	HorizontalAlignment horizontalAlignment = HorizontalAlignment::Stretch;
	VerticalAlignment verticalAlignment = VerticalAlignment::Stretch;

	virtual ~FrameworkElement() {}

#if WITH_EDITOR
	virtual const char *getClassName() =0;
#endif

	virtual void Measure(Size availableSize) override;
	virtual void Arrange(Rect finalRect) override;

	virtual void Render(Renderer &renderer) override
	{
		renderer.drawUIQuad(glm::vec2(layoutRect.x, layoutRect.y),
							glm::vec2(layoutRect.width, layoutRect.height),
							glm::vec4(1.0f, 0.0f, 0.0f, 0.5f));

		// Framework elements render themselves
	}

  protected:
	virtual Size MeasureOverride(Size availableSize) = 0;
	virtual void ArrangeOverride(Rect finalRect)
	{
		UIElement::Arrange(finalRect);
	}
};

// Panels arrange children
class Panel : public FrameworkElement
{
  public:
#if WITH_EDITOR
	virtual const char *getClassName() override { return "Panel"; };
#endif

	template <typename T, typename... Args> T *AddChild(Args &&...args)
	{
		static_assert(std::is_base_of_v<UIElement, T>);
		auto child = std::make_unique<T>(std::forward<Args>(args)...);
#if WITH_EDITOR
		child->owningPanel = this;
#endif
		T *ptr = child.get();
		mChildren.push_back(std::move(child));
		return ptr;
	}

	void Render(Renderer &renderer) override
	{
		renderer.drawUIQuad(glm::vec2(layoutRect.x, layoutRect.y),
							glm::vec2(layoutRect.width, layoutRect.height),
							glm::vec4(1.0f, 0.0f, 0.0f, 0.5f));

		// Panels render children
		for (const auto &child : mChildren)
		{
			child->Render(renderer);
		}
	}

  protected:
	Size MeasureOverride(Size available) override
	{
		Size maxSize{0, 0};
		for (const auto &child : mChildren)
		{
			child->Measure(available);
			maxSize.width = std::max(maxSize.width, child->desiredSize.width);
			maxSize.height =
				std::max(maxSize.height, child->desiredSize.height);
		}
		return maxSize;
	}

	void ArrangeOverride(Rect rect) override
	{
		for (const auto &child : mChildren)
		{
			child->Arrange(rect);
		}
	}

	std::vector<std::unique_ptr<UIElement>> mChildren;

  private:
#if WITH_EDITOR
	friend class UIInspector;
#endif
};

class Canvas : public Panel
{
  public:
#if WITH_EDITOR
	virtual const char *getClassName() override { return "Canvas"; };
#endif

	void SetLeft(UIElement *element, float left)
	{
		mSlots[element].left = left;
	}

	void SetTop(UIElement *element, float top) { mSlots[element].top = top; }

	void SetRight(UIElement *element, float right)
	{
		mSlots[element].right = right;
	}

	void SetBottom(UIElement *element, float bottom)
	{
		mSlots[element].bottom = bottom;
	}

  protected:
	Size MeasureOverride(Size available) override
	{
		Size maxSize{0, 0};
		for (auto &child : mChildren)
		{
			child->Measure(available);
			auto &slot = mSlots[child.get()];

			float x =
				!std::isnan(slot.left) ? slot.left
				: !std::isnan(slot.right)
					? (available.width - slot.right - child->desiredSize.width)
					: 0;

			float y = !std::isnan(slot.top) ? slot.top
					  : !std::isnan(slot.bottom)
						  ? (available.height - slot.bottom -
							 child->desiredSize.height)
						  : 0;

			maxSize.width =
				std::max(maxSize.width, x + child->desiredSize.width);
			maxSize.height =
				std::max(maxSize.height, y + child->desiredSize.height);
		}
		return maxSize;
	}

	void ArrangeOverride(Rect rect) override
	{
		for (auto &child : mChildren)
		{
			auto &slot = mSlots[child.get()];

			float x = rect.x;
			float y = rect.y;

			if (!std::isnan(slot.left))
			{
				x += slot.left;
			}
			else if (!std::isnan(slot.right))
			{
				x += rect.width - slot.right - child->desiredSize.width;
			}

			if (!std::isnan(slot.top))
			{
				y += slot.top;
			}
			else if (!std::isnan(slot.bottom))
			{
				y += rect.height - slot.bottom - child->desiredSize.height;
			}

			Rect r{x, y, child->desiredSize.width, child->desiredSize.height};

			child->Arrange(r);
		}
	}

  private:
	struct CanvasSlot
	{
		float left = std::numeric_limits<float>::quiet_NaN();
		float top = std::numeric_limits<float>::quiet_NaN();
		float right = std::numeric_limits<float>::quiet_NaN();
		float bottom = std::numeric_limits<float>::quiet_NaN();
	};

	std::unordered_map<UIElement *, CanvasSlot> mSlots;

#if WITH_EDITOR
	friend class UIInspector;
#endif
};

class StackPanel : public Panel
{
  public:
#if WITH_EDITOR
	virtual const char *getClassName() override { return "StackPanel"; };
#endif

	float spacing = 0;

  protected:
	Size MeasureOverride(Size available) override
	{
		Size total{0, 0};

		for (const auto &child : mChildren)
		{
			child->Measure(available);

			total.height += child->desiredSize.height + spacing;
			total.width = std::max(total.width, child->desiredSize.width);
		}

		return total;
	}

	void ArrangeOverride(Rect rect) override
	{
		float offset = 0;

		for (const auto &child : mChildren)
		{
			Rect r{rect.x, rect.y + offset, rect.width,
				   child->desiredSize.height};
			child->Arrange(r);
			offset += r.height + spacing;
		}
	}
};

class Image : public FrameworkElement
{
  public:
#if WITH_EDITOR
	virtual const char *getClassName() override { return "Image"; };
#endif

	Size sourceSize;

	void Render(Renderer &renderer) override
	{
		FrameworkElement::Render(renderer);

		renderer.drawUIQuad(glm::vec2(layoutRect.x, layoutRect.y),
							glm::vec2(layoutRect.width, layoutRect.height),
							glm::vec4(0.0f, 1.0f, 1.0f, 1.0f));
	}

  protected:
	Size MeasureOverride(Size available) override
	{
		return {std::min(sourceSize.width, available.width),
				std::min(sourceSize.height, available.height)};
	}

	void ArrangeOverride(Rect rect) override { layoutRect = rect; }
};

class TestUI
{
  public:
	void Init();
	void Render();

  private:
	std::unique_ptr<Panel> mRoot;
};
