#include <iostream>
#include <YDXRender/DXWindow.h>
#include <imgui/imgui.h>

using namespace std;

class Test :public YXX::DXFramework
{
public:

	void Init() override
	{
	}


	void Update(float dt) override
	{
		ImGui::Begin("Tets");
		ImGui::Text("WT");
		ImGui::End();
	}


	void Quit() override
	{
	}

};
int main()
{
	cout << "Begin Test";
	Test t;
	t.Run();
	cout << "end";
}