#include "UILayoutTest.h"

#include "../Editor.h"
#include "../Engine.h"
#include "../IconsMaterialSymbols.h"

#if WITH_EDITOR
#include <imgui.h>
class UIInspector : public EditorTool
{
  public:
	UIInspector(Panel *rootPanel)
		: EditorTool("UIInspector"), mRootPanel(rootPanel)
	{
	}

	void draw() override
	{
		ImGui::BeginChild("Hierarchy", ImVec2(0, 0), 0);
		drawTree2(mRootPanel);
		ImGui::EndChild();

		ImGui::Begin("Properties");
		drawPropertiesPanel();
		ImGui::End();

		if (gSelectedElement)
		{
			ImDrawList *dl = ImGui::GetForegroundDrawList();

			auto rect = gSelectedElement->layoutRect;

			dl->AddRect(ImVec2(rect.x, rect.y),
						ImVec2(rect.x + rect.width, rect.y + rect.height),
						IM_COL32(255, 0, 255, 255), 0.0f, 0, 2.0f);
		}
	}

  private:
	FrameworkElement *gSelectedElement = nullptr;
	void drawTree2(FrameworkElement *frameworkElement)
	{
		char buf[32];
		std::snprintf(buf, sizeof(buf), "%s_%p",
					  frameworkElement->getClassName(), frameworkElement);

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
								   ImGuiTreeNodeFlags_OpenOnDoubleClick |
								   ImGuiTreeNodeFlags_DefaultOpen;

		if (gSelectedElement == frameworkElement)
			flags |= ImGuiTreeNodeFlags_Selected;

		auto *panel = dynamic_cast<Panel *>(frameworkElement);

		if (!panel || panel->mChildren.empty())
			flags |= ImGuiTreeNodeFlags_Leaf;

		bool opened = ImGui::TreeNodeEx(buf, flags);

		// Handle selection
		if (ImGui::IsItemClicked())
		{
			gSelectedElement = frameworkElement;
		}

		if (opened)
		{
			if (panel)
			{
				for (const auto &child : panel->mChildren)
				{
					if (auto *childFE =
							dynamic_cast<FrameworkElement *>(child.get()))
					{
						drawTree2(childFE);
					}
				}
			}

			ImGui::TreePop();
		}
	}

	void drawPropertiesPanel()
	{
		if (!gSelectedElement)
		{
			ImGui::Text("Nothing selected");
			return;
		}

		drawInfo(gSelectedElement);
	}

	void drawInfo(FrameworkElement *element)
	{
		ImGui::DragFloat("Width", &element->width);
		ImGui::DragFloat("Height", &element->height);

		ImGui::Text("Actual Width %.3f", element->layoutRect.width);
		ImGui::Text("Actual Height %.3f", element->layoutRect.height);

		ImGui::DragFloat4("Margin", &element->margin.bottom);

		// Horizontal alignment
		ImGui::Text("Horizontal Alignment");

		HorizontalAlignment &hAlign = element->horizontalAlignment;

		ImGui::PushStyleVar(
			ImGuiStyleVar_SelectableTextAlign,
			ImVec2(0.5f, 0.5f)); // I dont even think this did anything

		ImGui::SameLine();
		if (ImGui::Selectable(ICON_MS_ALIGN_HORIZONTAL_LEFT,
							  hAlign == HorizontalAlignment::Left, 0,
							  ImVec2(16, 16)))
		{
			hAlign = HorizontalAlignment::Left;
		}

		ImGui::SameLine();
		if (ImGui::Selectable(ICON_MS_ALIGN_HORIZONTAL_CENTER,
							  hAlign == HorizontalAlignment::Center, 0,
							  ImVec2(16, 16)))
		{
			hAlign = HorizontalAlignment::Center;
		}

		ImGui::SameLine();
		if (ImGui::Selectable(ICON_MS_ALIGN_HORIZONTAL_RIGHT,
							  hAlign == HorizontalAlignment::Right, 0,
							  ImVec2(16, 16)))
		{
			hAlign = HorizontalAlignment::Right;
		}

		ImGui::SameLine();
		if (ImGui::Selectable(ICON_MS_HORIZONTAL_DISTRIBUTE,
							  hAlign == HorizontalAlignment::Stretch, 0,
							  ImVec2(16, 16)))
		{
			hAlign = HorizontalAlignment::Stretch;
		}

		ImGui::PopStyleVar(); // ImGuiStyleVar_SelectableTextAlign

		// Horizontal alignment
		ImGui::Text("Vertical Alignment");

		VerticalAlignment &vAlign = element->verticalAlignment;

		ImGui::PushStyleVar(
			ImGuiStyleVar_SelectableTextAlign,
			ImVec2(0.5f, 0.5f)); // I dont even think this did anything

		ImGui::SameLine();
		if (ImGui::Selectable(ICON_MS_VERTICAL_ALIGN_TOP,
							  vAlign == VerticalAlignment::Top, 0,
							  ImVec2(16, 16)))
		{
			vAlign = VerticalAlignment::Top;
		}

		ImGui::SameLine();
		if (ImGui::Selectable(ICON_MS_VERTICAL_ALIGN_CENTER,
							  vAlign == VerticalAlignment::Center, 0,
							  ImVec2(16, 16)))
		{

			vAlign = VerticalAlignment::Center;
		}

		ImGui::SameLine();
		if (ImGui::Selectable(ICON_MS_VERTICAL_ALIGN_BOTTOM,
							  vAlign == VerticalAlignment::Bottom, 0,
							  ImVec2(16, 16)))
		{
			vAlign = VerticalAlignment::Bottom;
		}

		ImGui::SameLine();
		if (ImGui::Selectable(ICON_MS_VERTICAL_DISTRIBUTE,
							  vAlign == VerticalAlignment::Stretch, 0,
							  ImVec2(16, 16)))
		{
			vAlign = VerticalAlignment::Stretch;
		}

		ImGui::PopStyleVar(); // ImGuiStyleVar_SelectableTextAlign

		// Canvas attached properties
		if (auto *canvas = dynamic_cast<Canvas *>(element->owningPanel))
		{
			float tmpCanvasLeft = canvas->mSlots[element].left;
			if (ImGui::DragFloat("Canvas Left", &tmpCanvasLeft))
			{
				canvas->SetLeft(element, tmpCanvasLeft);
			}

			float tmpCanvasTop = canvas->mSlots[element].top;
			if (ImGui::DragFloat("Canvas Top", &tmpCanvasTop))
			{
				canvas->SetTop(element, tmpCanvasTop);
			}

			float tmpCanvasRight = canvas->mSlots[element].right;
			if (ImGui::DragFloat("Canvas Right", &tmpCanvasRight))
			{
				canvas->SetRight(element, tmpCanvasRight);
			}

			float tmpCanvasBottom = canvas->mSlots[element].bottom;
			if (ImGui::DragFloat("Canvas Bottom", &tmpCanvasBottom))
			{
				canvas->SetBottom(element, tmpCanvasBottom);
			}
		}

		if (auto *stackPanel = dynamic_cast<StackPanel *>(element))
		{
			ImGui::DragFloat("Spacing", &stackPanel->spacing);
		}
	}

	Panel *mRootPanel;
};
#endif

void FrameworkElement::Measure(Size availableSize)
{
	// Deflate available space by margin
	Size innerAvailable = Deflate(availableSize, margin);

	// Ask derived class how big it wants to be
	Size desired = MeasureOverride(innerAvailable);

	// Apply Width / Height overrides
	if (!std::isnan(width))
	{
		desired.width = width;
	}
	if (!std::isnan(height))
	{
		desired.height = height;
	}

	// Desired size with margins
	desiredSize = Inflate(desired, margin);
}

void FrameworkElement::Arrange(Rect finalRect)
{
	Rect inner = Deflate(finalRect, margin);

	float arrangedWidth = inner.width;
	float arrangedHeight = inner.height;

	// Alignment handling
	if (!std::isnan(width))
	{
		arrangedWidth = width;
	}
	else if (horizontalAlignment != HorizontalAlignment::Stretch)
	{
		arrangedWidth = desiredSize.width - margin.left - margin.right;
	}

	if (!std::isnan(height))
	{
		arrangedHeight = height;
	}
	else if (verticalAlignment != VerticalAlignment::Stretch)
	{
		arrangedHeight = desiredSize.height - margin.top - margin.bottom;
	}

	arrangedWidth = std::min(arrangedWidth, inner.width);
	arrangedHeight = std::min(arrangedHeight, inner.height);

	float x = inner.x;
	float y = inner.y;

	float freeX = inner.width - arrangedWidth;
	float freeY = inner.height - arrangedHeight;

	if (horizontalAlignment == HorizontalAlignment::Center)
	{
		x += freeX * 0.5f;
	}
	else if (horizontalAlignment == HorizontalAlignment::Right)
	{
		x += freeX;
	}

	if (verticalAlignment == VerticalAlignment::Center)
	{
		y += freeY * 0.5f;
	}
	else if (verticalAlignment == VerticalAlignment::Bottom)
	{
		y += freeY;
	}

	Rect arranged = {x, y, arrangedWidth, arrangedHeight};

	layoutRect = arranged;

	ArrangeOverride(arranged);
}

void TestUI::Init()
{
	mRoot = std::make_unique<Panel>();

#if WITH_EDITOR
	Engine::instance->editor->registerTool<UIInspector>(mRoot.get());
#endif

	auto *stackPanel = mRoot->AddChild<StackPanel>();

	for (int i = 0; i < 4; ++i)
	{
		auto *img = stackPanel->AddChild<Image>();
		img->sourceSize = {100, 100};
		img->horizontalAlignment = HorizontalAlignment::Center;
		img->verticalAlignment = VerticalAlignment::Center;
	}

	/*auto *canvas = mRoot->AddChild<Canvas>();
	canvas->width = 300;
	canvas->height = 300;

	auto *thingInCanvas = canvas->AddChild<Image>();
	thingInCanvas->sourceSize = {50, 50};*/
}

void TestUI::Render()
{
	mRoot->Measure({800, 600});
	mRoot->Arrange({50, 50, 800, 600});

	mRoot->Render(*Engine::instance->renderer.get());
}
