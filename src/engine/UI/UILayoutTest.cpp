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
		ImGui::BeginChild("Layout Tree", ImVec2(0, 0));
		drawTree2(mRootPanel);
		ImGui::EndChild();

		ImGui::Begin("Properties");
		drawPropertiesPanel();
		ImGui::End();

		if (gSelectedElement)
		{
			ImDrawList *dl = ImGui::GetBackgroundDrawList();

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

		drawInfo2(gSelectedElement);
	}

	void drawInfo2(FrameworkElement *element)
	{
		if (!element)
		{
			return;
		}

		constexpr float columnWidth = 160.f;

		if (ImGui::TreeNodeEx("Layout", ImGuiTreeNodeFlags_DefaultOpen))
		{
			// Width
			ImGui::BeginGroup();
			{
				char widthLabelBuf[32];
				std::snprintf(widthLabelBuf, sizeof(widthLabelBuf),
							  "Width (%.2f)", element->layoutRect.width);

				ImGui::Text(widthLabelBuf);

				ImGui::SameLine(columnWidth);

				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::DragFloat("##Width", &element->width);

				ImGui::EndGroup();
			}

			// Height
			ImGui::BeginGroup();
			{
				char heightLabelBuf[32];
				std::snprintf(heightLabelBuf, sizeof(heightLabelBuf),
							  "Height (%.2f)", element->layoutRect.height);

				ImGui::Text(heightLabelBuf);

				ImGui::SameLine(columnWidth);

				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::DragFloat("##Height", &element->height);

				ImGui::EndGroup();
			}

			// Margin
			ImGui::BeginGroup();
			{
				ImGui::Text("Margin");

				float startX = ImGui::GetCursorPosX();
				ImGui::SameLine(columnWidth);

				ImGui::SetNextItemWidth(40.0f);
				ImGui::DragFloat(ICON_MS_BORDER_LEFT, &element->margin.left,
								 1.f, 0.f, 0.f, "%.0f");
				ImGui::SameLine();
				ImGui::SetNextItemWidth(40.0f);
				ImGui::DragFloat(ICON_MS_BORDER_RIGHT, &element->margin.right,
								 1.f, 0.f, 0.f, "%.0f");

				ImGui::SetCursorPosX(startX + columnWidth);

				ImGui::SetNextItemWidth(40.0f);
				ImGui::DragFloat(ICON_MS_BORDER_TOP, &element->margin.top, 1.f,
								 0.f, 0.f, "%.0f");
				ImGui::SameLine();
				ImGui::SetNextItemWidth(40.0f);
				ImGui::DragFloat(ICON_MS_BORDER_BOTTOM, &element->margin.bottom,
								 1.f, 0.f, 0.f, "%.0f");

				ImGui::EndGroup();
			}

			// Horizontal alignment
			auto hAlignButton =
				[element](const char *label, HorizontalAlignment hAlign)
			{
				ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign,
									ImVec2(0.5f, 0.5f));
				if (ImGui::Selectable(
						label, element->horizontalAlignment == hAlign,
						ImGuiSelectableFlags_None, ImVec2(15, 15)))
				{
					element->horizontalAlignment = hAlign;
				}
				ImGui::PopStyleVar();
			};

			ImGui::BeginGroup();
			{
				ImGui::Text("Horizontal Alignment");

				ImGui::SameLine(columnWidth);

				hAlignButton(ICON_MS_ALIGN_HORIZONTAL_LEFT,
							 HorizontalAlignment::Left);
				ImGui::SameLine();
				hAlignButton(ICON_MS_ALIGN_HORIZONTAL_CENTER,
							 HorizontalAlignment::Center);
				ImGui::SameLine();
				hAlignButton(ICON_MS_ALIGN_HORIZONTAL_RIGHT,
							 HorizontalAlignment::Right);
				ImGui::SameLine();
				hAlignButton(ICON_MS_HORIZONTAL_DISTRIBUTE,
							 HorizontalAlignment::Stretch);

				ImGui::EndGroup();
			}

			// Vertical alignment
			auto vAlignButton =
				[element](const char *label, VerticalAlignment vAlign)
			{
				ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign,
									ImVec2(0.5f, 0.5f));
				if (ImGui::Selectable(
						label, element->verticalAlignment == vAlign,
						ImGuiSelectableFlags_None, ImVec2(15, 15)))
				{
					element->verticalAlignment = vAlign;
				}
				ImGui::PopStyleVar();
			};

			ImGui::BeginGroup();
			{
				ImGui::Text("Vertical Alignment");

				ImGui::SameLine(columnWidth);

				vAlignButton(ICON_MS_VERTICAL_ALIGN_TOP,
							 VerticalAlignment::Top);
				ImGui::SameLine();
				vAlignButton(ICON_MS_VERTICAL_ALIGN_CENTER,
							 VerticalAlignment::Center);
				ImGui::SameLine();
				vAlignButton(ICON_MS_VERTICAL_ALIGN_BOTTOM,
							 VerticalAlignment::Bottom);
				ImGui::SameLine();
				vAlignButton(ICON_MS_VERTICAL_DISTRIBUTE,
							 VerticalAlignment::Stretch);

				ImGui::EndGroup();
			}

			// Canvas attached properties
			if (auto *owningCanvas =
					dynamic_cast<Canvas *>(element->owningPanel))
			{
				ImGui::BeginGroup();
				{
					ImGui::Text("Canvas Left");
					ImGui::SameLine(columnWidth);

					float tmpCanvasLeft = owningCanvas->mSlots[element].left;
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
					if (ImGui::DragFloat("##Canvas Left", &tmpCanvasLeft))
					{
						owningCanvas->SetLeft(element, tmpCanvasLeft);
					}

					ImGui::Text("Canvas Top");
					ImGui::SameLine(columnWidth);

					float tmpCanvasTop = owningCanvas->mSlots[element].top;
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
					if (ImGui::DragFloat("##Canvas Top", &tmpCanvasTop))
					{
						owningCanvas->SetTop(element, tmpCanvasTop);
					}

					ImGui::Text("Canvas Right");
					ImGui::SameLine(columnWidth);

					float tmpCanvasRight = owningCanvas->mSlots[element].right;
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
					if (ImGui::DragFloat("##Canvas Right", &tmpCanvasRight))
					{
						owningCanvas->SetRight(element, tmpCanvasRight);
					}

					ImGui::Text("Canvas Bottom");
					ImGui::SameLine(columnWidth);

					float tmpCanvasBottom =
						owningCanvas->mSlots[element].bottom;
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
					if (ImGui::DragFloat("##Canvas Bottom", &tmpCanvasBottom))
					{
						owningCanvas->SetBottom(element, tmpCanvasBottom);
					}

					ImGui::EndGroup();
				}
			}

			// Stack panel
			if (auto *stackPanel = dynamic_cast<StackPanel *>(element))
			{
				ImGui::BeginGroup();
				{
					ImGui::Text("Orientation");
					ImGui::SameLine(columnWidth);

					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

					const char *items[] = {"Horizontal", "Vertical"};
					int orientationTmp = static_cast<int>(stackPanel->orientation);
					if (ImGui::Combo("##Orientation", &orientationTmp, items,
									 IM_ARRAYSIZE(items)))
					{
						stackPanel->orientation =
							static_cast<Orientation>(orientationTmp);
					}

					ImGui::EndGroup();
				}

				ImGui::BeginGroup();
				{
					ImGui::Text("Spacing");
					ImGui::SameLine(columnWidth);

					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
					ImGui::DragFloat("##Spacing", &stackPanel->spacing);

					ImGui::EndGroup();
				}
			}

			ImGui::TreePop();
		}

		// Panel
		if (auto *panel = dynamic_cast<Panel *>(element))
		{
			ImGui::BeginGroup();
			{
				ImGui::Text("Background");
				ImGui::SameLine(columnWidth);

				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::ColorEdit4("##Background", (float *)&panel->background,
								  ImGuiColorEditFlags_NoInputs |
									  ImGuiColorEditFlags_NoLabel);

				ImGui::EndGroup();
			}
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
	auto *renderer = Engine::instance->renderer.get();

	auto loadTex = [this, renderer](const char *path)
	{
		Texture tex = renderer->loadTexture(path);
		mUITextures.push_back(tex);
		return tex;
	};

	mRoot = std::make_unique<Panel>();

#if WITH_EDITOR
	Engine::instance->editor->registerTool<UIInspector>(mRoot.get());
#endif

	auto *canvas = mRoot->AddChild<Canvas>();
	canvas->width = 400.f;
	canvas->height = 400.f;

	auto *img = canvas->AddChild<Image>();
	img->sourceSize = {100, 100};
	canvas->SetLeft(img, 10.f);
	canvas->SetTop(img, 10.f);

	auto *stackPanel = mRoot->AddChild<StackPanel>();
	stackPanel->horizontalAlignment = HorizontalAlignment::Center;
	stackPanel->verticalAlignment = VerticalAlignment::Center;

	for (int i = 0; i < 4; ++i)
	{
		auto *img = stackPanel->AddChild<Image>();
		img->sourceSize = {100, 100};
		img->margin = {10.f, 10.f, 10.f, 10.f};
	}
}

void TestUI::Shutdown()
{
	auto *renderer = Engine::instance->renderer.get();
	for (auto tex : mUITextures)
	{
		renderer->deleteTexture(tex);
	}
}

void TestUI::Render()
{
	// Perform layout
	mRoot->Measure({800, 600});
	mRoot->Arrange({50, 50, 800, 600});

	// Draw a viewport background
	auto *renderer = Engine::instance->renderer.get();

	renderer->drawUIQuad(
		glm::vec2(mRoot->layoutRect.x, mRoot->layoutRect.y),
		glm::vec2(mRoot->layoutRect.width, mRoot->layoutRect.height),
		glm::vec4(0.5f, 0.5f, 0.5f, 1.f));

	// Render the tree
	mRoot->Render(*renderer);
}
