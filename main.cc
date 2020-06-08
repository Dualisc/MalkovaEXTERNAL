#include "commons.h"

#include "driver_ctrl.h"
#include "mmap.h"
#include "security.h"

#include "renderer/menu/menu.h"

#include "sdk/classes.h"
#include "settings.h"
#include "utils/xor.h"

#pragma comment(lib, "urlmon.lib")
#pragma comment(lib,"wininet.lib")

std::string folderpath;
std::string dllpath;
std::string dllpath2;
std::string driverpath;

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK MyWindowProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
void WINAPI UpdateSurface(HWND hWnd);
HWND WINAPI InitializeWin(HINSTANCE hInst);
void UpdateWinPosition();

typedef struct _MyUncStr
{
	char stub[0x10];
	int len;
	wchar_t str[1];
} *pMyUncStr;

std::vector<std::uint32_t> GetProcessIds(const std::wstring& processName);
JBMenu menu;
HWND GameWindow = NULL;
HWND ThisWindow = NULL;
char WINNAME[19] = " ";
HINSTANCE asdd;
DWORD GetThreadId(DWORD dwProcessId);
uintptr_t GABaseAddress;
uintptr_t UnityBaseAddress;
uintptr_t GOM;
uintptr_t BaseNetworkable;
uintptr_t TOD_Sky;
BasePlayer LocalPlayer;
int ScreenWidth;
int ScreenHeight;
int Width2 = GetSystemMetrics(SM_CXSCREEN);
int Height2 = GetSystemMetrics(SM_CYSCREEN);
int midX = Width2 / 2;
int midY = Height2 / 2;
int ticks = 0;
int beforeclock = 0;
int FPS = 0;
bool overlaycreated = false;
bool InjectTheDll = true;
bool MenuOpen = true;
bool PrintedAddresses = false;
LPDIRECT3DDEVICE9 pDevice;

wchar_t LicenseHash;

float FOV = 300, curFOV;

BasePlayer closestPlayer;

Vector2 MenuPos = { 200.f, 200.f };

JBMenu::JBMenu(void)
{
	this->Visible = true;
}


void JBMenu::Init_Menu(LPCWSTR Title, int x, int y)
{
	this->Is_Ready = true;
	this->sMenu.Title = Title;
	this->sMenu.x = x;
	this->sMenu.y = y;
}

void JBMenu::AddFolder(LPCWSTR Name, int* Pointer, int limit)
{
	sOptions[this->Items].Name = (LPCWSTR)Name;
	sOptions[this->Items].Function = Pointer;
	sOptions[this->Items].Type = T_FOLDER;
	this->Items++;
}

void JBMenu::AddOption(LPCWSTR Name, int* Pointer, int* Folder, int Limit, int type)
{
	if (*Folder == 0)
		return;
	sOptions[this->Items].Name = Name;
	sOptions[this->Items].Function = Pointer;
	sOptions[this->Items].Type = type;
	sOptions[this->Items].limit = Limit;
	this->Items++;
}

void JBMenu::Navigation()
{
	if (GetAsyncKeyState(VK_INSERT) & 1)
		this->Visible = !this->Visible;

	if (!this->Visible)
		return;

	int value = 0;

	if (GetAsyncKeyState(VK_DOWN))
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		this->Cur_Pos++;
		if (sOptions[this->Cur_Pos].Name == 0)
			this->Cur_Pos--;
	}

	if (GetAsyncKeyState(VK_UP))
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		this->Cur_Pos--;
		if (this->Cur_Pos == -1)
			this->Cur_Pos++;
	}

	else if (GetAsyncKeyState(VK_RIGHT))
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		if (*sOptions[this->Cur_Pos].Function <= 0)
			value++;
	}

	else if ((GetAsyncKeyState(VK_LEFT)) && *sOptions[this->Cur_Pos].Function <= 1)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		value--;
	}


	if (value)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		*sOptions[this->Cur_Pos].Function += value;
		if (sOptions[this->Cur_Pos].Type == T_FOLDER)
		{
			memset(&sOptions, 0, sizeof(sOptions));
			this->Items = 0;
		}
	}


}

void JBMenu::Draw_Menu()
{
	if (!this->Visible)
		return;

	//this->DrawText(this->sMenu.Title, 14, sMenu.x + 10, sMenu.y, this->Color_Font);
	for (int i = 0; i < this->Items; i++)
	{
		if (this->sOptions[i].Type == T_OPTION)
		{
			if (*this->sOptions[i].Function)
			{
				DrawWString(L"On", 15, (float)(sMenu.x + 200), (float)(sMenu.y + LineH * (i + 2)), settings::menucolors::on::r, settings::menucolors::on::g, settings::menucolors::on::b);
				//DrawWString(Vector2{ (float)(sMenu.x + 150), (float)(sMenu.y + LineH * (i + 2)) }, );
				//Render::String(vec2_t{ (float)(sMenu.x + 150), (float)(sMenu.y + LineH * (i + 2)) }, L"On");
				//DrawWString(L"On", 12, sMenu.x + 150, sMenu.y + LineH * (i + 2), this->Color_On);
			}
			else
			{
				DrawWString(L"Off", 15, (float)(sMenu.x + 200), (float)(sMenu.y + LineH * (i + 2)), settings::menucolors::off::r, settings::menucolors::off::g, settings::menucolors::off::b);
				//i_renderer.helper->string(Vector2{ (float)(sMenu.x + 150), (float)(sMenu.y + LineH * (i + 2)) }, L"Off");
				//this->DrawText(L"Off", 12, sMenu.x + 150, sMenu.y + LineH * (i + 2), this->Color_Off);
			}
		}
		if (this->sOptions[i].Type == T_FOLDER)
		{
			if (*this->sOptions[i].Function)
			{
				DrawWString(L"Opened", 15, (float)(sMenu.x + 200), (float)(sMenu.y + LineH * (i + 2)), settings::menucolors::folderopened::r, settings::menucolors::folderopened::g, settings::menucolors::folderopened::b);
				//i_renderer.helper->string(Vector2{ (float)(sMenu.x + 150), (float)(sMenu.y + LineH * (i + 2)) }, L"Opened");
				//this->DrawText(L"Opend", 12, sMenu.x + 150, sMenu.y + LineH * (i + 2), this->Color_Folder);
			}
			else
			{
				DrawWString(L"Closed", 15, (float)(sMenu.x + 200), (float)(sMenu.y + LineH * (i + 2)), settings::menucolors::folderclosed::r, settings::menucolors::folderclosed::g, settings::menucolors::folderclosed::b);
				//i_renderer.helper->string(Vector2{ (float)(sMenu.x + 150), (float)(sMenu.y + LineH * (i + 2)) }, L"Closed");
				//this->DrawText(L"Closed", 12, sMenu.x + 150, sMenu.y + LineH * (i + 2), this->Color_Folder);
			}
		}
		if (this->sOptions[i].Type == T_BOOL)
		{
			if (*this->sOptions[i].Function)
			{
				DrawWString(L"True", 15, (float)(sMenu.x + 200), (float)(sMenu.y + LineH * (i + 2)), settings::menucolors::booltrue::r, settings::menucolors::booltrue::g, settings::menucolors::booltrue::b);
				//i_renderer.helper->string(Vector2{ (float)(sMenu.x + 150), (float)(sMenu.y + LineH * (i + 2)) }, L"True", D2D1::ColorF::Green);
				//this->DrawText(L"Opend", 12, sMenu.x + 150, sMenu.y + LineH * (i + 2), this->Color_Folder);
			}
			else
			{
				DrawWString(L"False", 15, (float)(sMenu.x + 200), (float)(sMenu.y + LineH * (i + 2)), settings::menucolors::boolfalse::r, settings::menucolors::boolfalse::g, settings::menucolors::boolfalse::b);
				//i_renderer.helper->string(Vector2{ (float)(sMenu.x + 150), (float)(sMenu.y + LineH * (i + 2)) }, L"False", D2D1::ColorF::Red);
				//this->DrawText(L"Closed", 12, sMenu.x + 150, sMenu.y + LineH * (i + 2), this->Color_Folder);
			}
		}
		if (this->sOptions[i].Type == T_INT)
		{
			auto sval = std::to_string((*this->sOptions[i].Function));
			std::wstring wide_string = std::wstring(sval.begin(), sval.end());
			const wchar_t* result = wide_string.c_str();
			DrawWString(result, 15, (float)(sMenu.x + 200), (float)(sMenu.y + LineH * (i + 2)), settings::menucolors::menuint::r, settings::menucolors::menuint::g, settings::menucolors::menuint::b);
			//i_renderer.helper->string(Vector2{ (float)(sMenu.x + 150), (float)(sMenu.y + LineH * (i + 2)) }, result);
		}
		//D2D1::ColorF Color = 250, 250, 250);
		Vector3 Color = { (float)settings::menucolors::menutext::r, (float)settings::menucolors::menutext::g, (float)settings::menucolors::menutext::b };
		if (this->Cur_Pos == i)
			Color = { (float)settings::menucolors::selected::r, (float)settings::menucolors::selected::g, (float)settings::menucolors::selected::b }; //D2D1::ColorF(250, 0, 200);
		else if (this->sOptions[i].Type == T_FOLDER)
			Color = { (float)settings::menucolors::folder::r, (float)settings::menucolors::folder::g, (float)settings::menucolors::folder::b };//D2D1::ColorF(0, 250, 250);

		//this->DrawText(this->sOptions[i].Name, 12, sMenu.x + 5, sMenu.y + LineH * (i + 2), Color);
		if (this->sOptions[i].Type == T_FOLDER)
			DrawWString(this->sOptions[i].Name, 15, (float)(sMenu.x + 15), (float)(sMenu.y + LineH * (i + 2)), Color.x, Color.y, Color.z);
			//i_renderer.helper->string(Vector2{ (float)(sMenu.x + 15), (float)(sMenu.y + LineH * (i + 2)) }, this->sOptions[i].Name, Color);
		else
			DrawWString(this->sOptions[i].Name, 15, (float)(sMenu.x + 30), (float)(sMenu.y + LineH * (i + 2)), Color.x, Color.y, Color.z);
			//i_renderer.helper->string(Vector2{ (float)(sMenu.x + 30), (float)(sMenu.y + LineH * (i + 2)) }, this->sOptions[i].Name, Color);

	}
}

bool JBMenu::IsReady()
{
	if (this->Items)
		return true;
	return false;
}


class PlayerClass
{
public:
	uintptr_t Player;
	uintptr_t ObjectClass;
	std::string ClassName;
	std::string Name;
	std::wstring WName;
	Vector3 Position;
	bool IsLocalPlayer;
	int Health;
	int MaxHealth;

public:
	bool operator==(PlayerClass ent)
	{
		if (ent.Player == this->Player)
			return true;
		else
			return false;
	}
};

class OreClass
{
public:
	uintptr_t Ore;
};

class CollectableClass
{
public:
	uintptr_t Object;
};

class VehicleClass
{
public:
	uintptr_t Object;
	std::string Name;
};

class CrateClass
{
public:
	uintptr_t Object;
};

class StashClass
{
public:
	uintptr_t Object;
};

class ItemClass
{
public:
	uintptr_t Object;
	uintptr_t ObjectClass;
	std::wstring Name;
};

Vector3 localPos;

std::vector<BasePlayer> PlayerList;
std::vector<OreClass> OreList;
std::vector<CollectableClass> CollectibleList;
std::vector<VehicleClass> VehicleList;
std::vector<CrateClass> CrateList;
std::vector<StashClass> StashList;
std::vector<Item> DroppedItemList;

HANDLE hProcess = INVALID_HANDLE_VALUE;

Matrix4x4 pViewMatrix;

uint64_t scan_for_klass(const char* name)
{
	auto base = mex.GetModuleBase("GameAssembly.dll"); //mem::get_module_base(L"GameAssembly.dll");
	auto dos_header = mex.Read<IMAGE_DOS_HEADER>(base);
	auto data_header = mex.Read<IMAGE_SECTION_HEADER>(base + dos_header.e_lfanew + sizeof(IMAGE_NT_HEADERS64) + (3 * 40));
	auto next_section = mex.Read<IMAGE_SECTION_HEADER>(base + dos_header.e_lfanew + sizeof(IMAGE_NT_HEADERS64) + (4 * 40));
	auto data_size = next_section.VirtualAddress - data_header.VirtualAddress;

	if (strcmp((char*)data_header.Name, ".data")) {
		printf("[!] Section order changed\n");
	}

	for (uint64_t offset = data_size; offset > 0; offset -= 8) {
		char klass_name[0x100] = { 0 };
		auto klass = mex.Read<uint64_t>(base + data_header.VirtualAddress + offset);
		if (klass == 0) { continue; }
		auto name_pointer = mex.Read<uint64_t>(klass + 0x10);
		if (name_pointer == 0) { continue; }
		mex.Read(name_pointer, klass_name, sizeof(klass_name));
		if (!strcmp(klass_name, name)) {
			//printf("[*] 0x%x -> %s\n", data_header.VirtualAddress + offset, name);
			return klass;
		}
	}

	printf("[!] Unable to find %s in scan\n", name);
	//exit(0);
}

std::uintptr_t get_base_player(std::uintptr_t entity)
{
	const auto unk1 = mex.Read<uintptr_t>(entity + 0x18);

	if (!unk1)
		return 0;
	return mex.Read<uintptr_t>(unk1 + 0x28);
}

std::uint32_t get_player_health(std::uint64_t entity)
{
	const auto base_player = get_base_player(entity);

	if (!base_player)
		return 0;

	const auto player_health = mex.Read<float>(base_player + 0x1F4);

	if (player_health <= 0.8f)
		return 0;

	return std::lround(player_health);
}

Vector3 GetVelocity(uintptr_t Entity)
{
	uintptr_t player_model = mex.Read<uintptr_t>(Entity + 0x118);
	return mex.Read<Vector3>(player_model + 0x1D4);
}

enum BoneList : int
{
	l_hip = 1,
	l_knee,
	l_foot,
	l_toe,
	l_ankle_scale,
	pelvis,
	penis,
	GenitalCensor,
	GenitalCensor_LOD0,
	Inner_LOD0,
	GenitalCensor_LOD1,
	GenitalCensor_LOD2,
	r_hip,
	r_knee,
	r_foot,
	r_toe,
	r_ankle_scale,
	spine1,
	spine1_scale,
	spine2,
	spine3,
	spine4,
	l_clavicle,
	l_upperarm,
	l_forearm,
	l_hand,
	l_index1,
	l_index2,
	l_index3,
	l_little1,
	l_little2,
	l_little3,
	l_middle1,
	l_middle2,
	l_middle3,
	l_prop,
	l_ring1,
	l_ring2,
	l_ring3,
	l_thumb1,
	l_thumb2,
	l_thumb3,
	IKtarget_righthand_min,
	IKtarget_righthand_max,
	l_ulna,
	neck,
	head,
	jaw,
	eyeTranform,
	l_eye,
	l_Eyelid,
	r_eye,
	r_Eyelid,
	r_clavicle,
	r_upperarm,
	r_forearm,
	r_hand,
	r_index1,
	r_index2,
	r_index3,
	r_little1,
	r_little2,
	r_little3,
	r_middle1,
	r_middle2,
	r_middle3,
	r_prop,
	r_ring1,
	r_ring2,
	r_ring3,
	r_thumb1,
	r_thumb2,
	r_thumb3,
	IKtarget_lefthand_min,
	IKtarget_lefthand_max,
	r_ulna,
	l_breast,
	r_breast,
	BoobCensor,
	BreastCensor_LOD0,
	BreastCensor_LOD1,
	BreastCensor_LOD2,
	collision,
	displacement
};

Vector3 GetPosition(uintptr_t transform)
{
	if (!transform) return Vector3{ 0.f, 0.f, 0.f };

	struct Matrix34 { BYTE vec0[16]; BYTE vec1[16]; BYTE vec2[16]; };
	const __m128 mulVec0 = { -2.000, 2.000, -2.000, 0.000 };
	const __m128 mulVec1 = { 2.000, -2.000, -2.000, 0.000 };
	const __m128 mulVec2 = { -2.000, -2.000, 2.000, 0.000 };

	int Index = mex.Read<int>(transform + 0x40);// *(PINT)(transform + 0x40);
	uintptr_t pTransformData = mex.Read<uintptr_t>(transform + 0x38);
	uintptr_t transformData[2];
	mex.Read((pTransformData + 0x18), &transformData, 16);
	//mex.Read(&transformData, (PVOID)(pTransformData + 0x18), 16);
	//safe_memcpy(&transformData, (PVOID)(pTransformData + 0x18), 16);

	size_t sizeMatriciesBuf = 48 * Index + 48;
	size_t sizeIndicesBuf = 4 * Index + 4;

	PVOID pMatriciesBuf = malloc(sizeMatriciesBuf);
	PVOID pIndicesBuf = malloc(sizeIndicesBuf);

	if (pMatriciesBuf && pIndicesBuf)
	{
		// Read Matricies array into the buffer
		mex.Read(transformData[0], pMatriciesBuf, sizeMatriciesBuf);
		//impl::memory->read(transformData[0], pMatriciesBuf, sizeMatriciesBuf);
		// Read Indices array into the buffer
		mex.Read(transformData[1], pIndicesBuf, sizeIndicesBuf);

		__m128 result = *(__m128*)((ULONGLONG)pMatriciesBuf + 0x30 * Index);
		int transformIndex = *(int*)((ULONGLONG)pIndicesBuf + 0x4 * Index);

		while (transformIndex >= 0)
		{
			Matrix34 matrix34 = *(Matrix34*)((ULONGLONG)pMatriciesBuf + 0x30 * transformIndex);
			__m128 xxxx = _mm_castsi128_ps(_mm_shuffle_epi32(*(__m128i*)(&matrix34.vec1), 0x00));
			__m128 yyyy = _mm_castsi128_ps(_mm_shuffle_epi32(*(__m128i*)(&matrix34.vec1), 0x55));
			__m128 zwxy = _mm_castsi128_ps(_mm_shuffle_epi32(*(__m128i*)(&matrix34.vec1), 0x8E));
			__m128 wzyw = _mm_castsi128_ps(_mm_shuffle_epi32(*(__m128i*)(&matrix34.vec1), 0xDB));
			__m128 zzzz = _mm_castsi128_ps(_mm_shuffle_epi32(*(__m128i*)(&matrix34.vec1), 0xAA));
			__m128 yxwy = _mm_castsi128_ps(_mm_shuffle_epi32(*(__m128i*)(&matrix34.vec1), 0x71));
			__m128 tmp7 = _mm_mul_ps(*(__m128*)(&matrix34.vec2), result);

			result = _mm_add_ps(
				_mm_add_ps(
					_mm_add_ps(
						_mm_mul_ps(
							_mm_sub_ps(
								_mm_mul_ps(_mm_mul_ps(xxxx, mulVec1), zwxy),
								_mm_mul_ps(_mm_mul_ps(yyyy, mulVec2), wzyw)),
							_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(tmp7), 0xAA))),
						_mm_mul_ps(
							_mm_sub_ps(
								_mm_mul_ps(_mm_mul_ps(zzzz, mulVec2), wzyw),
								_mm_mul_ps(_mm_mul_ps(xxxx, mulVec0), yxwy)),
							_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(tmp7), 0x55)))),
					_mm_add_ps(
						_mm_mul_ps(
							_mm_sub_ps(
								_mm_mul_ps(_mm_mul_ps(yyyy, mulVec0), yxwy),
								_mm_mul_ps(_mm_mul_ps(zzzz, mulVec1), zwxy)),
							_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(tmp7), 0x00))),
						tmp7)), *(__m128*)(&matrix34.vec0));
			try {
				transformIndex = *(int*)((ULONGLONG)pIndicesBuf + 0x4 * transformIndex);
			}
			catch (...)
			{
				// Do nothing
			}
		}

		return Vector3(result.m128_f32[0], result.m128_f32[1], result.m128_f32[2]);
	}
}

bool WorldToScreen(const Vector3& EntityPos, Vector2& ScreenPos)
{
	Vector3 TransVec = Vector3(pViewMatrix._14, pViewMatrix._24, pViewMatrix._34);
	Vector3 RightVec = Vector3(pViewMatrix._11, pViewMatrix._21, pViewMatrix._31);
	Vector3 UpVec = Vector3(pViewMatrix._12, pViewMatrix._22, pViewMatrix._32);
	float w = Math::Dot(TransVec, EntityPos) + pViewMatrix._44;
	if (w < 0.098f) return false;
	float y = Math::Dot(UpVec, EntityPos) + pViewMatrix._42;
	float x = Math::Dot(RightVec, EntityPos) + pViewMatrix._41;
	ScreenPos = Vector2((ScreenWidth / 2) * (1.f + x / w), (ScreenHeight / 2) * (1.f - y / w));
	return true;
}

Vector3 GetBonePosition(uintptr_t Entity, int bone)
{
	uintptr_t player_model = mex.Read<uintptr_t>(Entity + 0x118);
	uintptr_t BoneTransforms = mex.Read<uintptr_t>(player_model + 0x48);
	uintptr_t entity_bone = mex.Read<uintptr_t>(BoneTransforms + (0x20 + (bone * 0x8)));
	return GetPosition(mex.Read<uintptr_t>(entity_bone + 0x10));
}

float GetFov(uintptr_t Entity, int Bone) {
	Vector2 ScreenPos;
	if (!WorldToScreen(GetBonePosition(Entity, Bone), ScreenPos))
		return 1000.f;
	return Math::Calc2D_Dist(Vector2(ScreenWidth / 2, ScreenHeight / 2), ScreenPos);
}

std::string get_class_name(std::uint64_t class_object)
{
	const auto object_unk = mex.Read<uintptr_t>(class_object);

	if (!object_unk)
		return {};

	return read_ascii(mex.Read<uintptr_t>(object_unk + 0x10), 64);
}

Vector3 get_obj_pos(std::uint64_t entity)
{
	const auto player_visual = mex.Read<uintptr_t>(entity + 0x8);

	if (!player_visual)
		return {};

	const auto visual_state = mex.Read<uintptr_t>(player_visual + 0x38);

	if (!visual_state)
		return {};

	return mex.Read<Vector3>(visual_state + 0x90);
}

Vector3 GetCurrentObjectPosition(std::uintptr_t entity)
{
	const auto unk1 = mex.Read<uintptr_t>(entity + 0x10);

	if (!unk1)
		return Vector3{ NULL, NULL, NULL };

	const auto unk2 = mex.Read<uintptr_t>(unk1 + 0x30);

	if (!unk2)
		return Vector3{ NULL, NULL, NULL };



	const auto unk3 = mex.Read<uintptr_t>(unk2 + 0x30);

	if (!unk3)
		return Vector3{ NULL, NULL, NULL };



	/* shouldn't be needed, but in case */
	if (!entity)
		return Vector3{ NULL, NULL, NULL };

	Vector2 ScreenPos;
	return get_obj_pos(unk3);
}


void SetAdminFlag(uintptr_t LocalPlayer)
{
	int flags = mex.Read<int>(LocalPlayer + 0x5B8);

	flags |= 4;

	mex.Write<uintptr_t>(LocalPlayer + 0x5B8, flags);
}

void SetAimingFlag(uintptr_t LocalPlayer)
{
	int flags = mex.Read<int>(LocalPlayer + 0x5B0);

	flags |= 16384;

	mex.Write<uintptr_t>(LocalPlayer + 0x5B0, flags);
}

void SetRunningFlag(uintptr_t LocalPlayer)
{
	int flags = mex.Read<int>(LocalPlayer + 0x5B0);

	flags |~ 8192;

	mex.Write<uintptr_t>(LocalPlayer + 0x5B0, flags);
}

void SetGroundAngles(std::uintptr_t LocalPlayer)
{
	auto BaseMovement = mex.Read<uintptr_t>(LocalPlayer + 0x5E8);
	if (!BaseMovement)
		return;

	mex.Write<float>(BaseMovement + 0xAC, 0.f); // private float groundAngle; // 0xAC
	mex.Write<float>(BaseMovement + 0xB0, 0.f); // private float groundAngleNew; // 0xB0
}

inline float distance_cursor(Vector2 vec)
{
	POINT p;
	if (GetCursorPos(&p))
	{
		float ydist = (vec.y - p.y);
		float xdist = (vec.x - p.x);
		float ret = sqrt(pow(ydist, 2) + pow(xdist, 2));
		return ret;
	}
}

Vector2 smooth(Vector2 pos)
{
	Vector2 center{ (float)(ScreenWidth / 2), (float)(ScreenHeight / 2) };
	Vector2 target{ 0, 0 };
	if (pos.x != 0) {
		if (pos.x > center.x) {
			target.x = -(center.x - pos.x);
			target.x /= 1;
			if (target.x + center.x > center.x * 2)
				target.x = 0;
		}

		if (pos.x < center.x) {
			target.x = pos.x - center.x;
			target.x /= 1;
			if (target.x + center.x < 0)
				target.x = 0;
		}
	}

	if (pos.y != 0) {
		if (pos.y > center.y) {
			target.y = -(center.y - pos.y);
			target.y /= 1;
			if (target.y + center.y > center.y * 2)
				target.y = 0;
		}

		if (pos.y < center.y) {
			target.y = pos.y - center.y;
			target.y /= 1;
			if (target.y + center.y < 0)
				target.y = 0;
		}
	}

	target.x /= settings::aimbot::smooth;
	target.y /= settings::aimbot::smooth;

	if (abs(target.x) < 1) {
		if (target.x > 0) {
			target.x = 1;
		}
		if (target.x < 0) {
			target.x = -1;
		}
	}
	if (abs(target.y) < 1) {
		if (target.y > 0) {
			target.y = 1;
		}
		if (target.y < 0) {
			target.y = -1;
		}
	}

	return target;
}

void SetThickBullet(uintptr_t Weapon)
{
	if (!Weapon)
		return;

	auto Held = mex.Read<uintptr_t>(Weapon + 0x98);

	if (!Held)
	{
		std::cout << "!Held" << std::endl;
		return;
	}
	auto projectile_list = mex.Read<uintptr_t>(Held + 0x338);

	if (!projectile_list)
	{
		std::cout << "!projectile_list" << std::endl;
		return;
	}

	auto projectile_array = mex.Read<uintptr_t>(projectile_list + 0x18);

	if (!projectile_array)
	{
		std::cout << "!projectile_array" << std::endl;
		return;
	}

	auto projectile_list_size = mex.Read<int>(projectile_list + 0x10);

	try
	{
		for (auto i = 0; i < projectile_list_size; i++)
		{

			auto current_projectile = mex.Read<uintptr_t>(projectile_array + (0x20 + (i * 8)));

			if (!current_projectile)
			{
				std::cout << "!current_projectile" << std::endl;
				continue;
			}

			if (true) {
				auto old_thickness = mex.Read<float>(current_projectile + 0x2C);


				if (old_thickness <= 0.f || old_thickness >= 1.f)
				{
					std::cout << "old_thickness" << std::endl;
					continue;
				}

				mex.Write<float>(current_projectile + 0x2C, 1.f);
				auto new_thickness = mex.Read<float>(current_projectile + 0x2C);
				printf("[Bullet Thickness] Changed %f.f to %f.f \n", old_thickness, new_thickness);

			}

			std::this_thread::sleep_for(std::chrono::microseconds(3));
		}
	}
	catch (...) {}
}

void Normalize(float& Yaw, float& Pitch) {
	if (Pitch < -89) Pitch = -89;
	else if (Pitch > 89) Pitch = 89;
	if (Yaw < -360) Yaw += 360;
	else if (Yaw > 360) Yaw -= 360;
}

void SetAlwaysDay()
{
	//auto TOD_Sky = mex.Read<uintptr_t>();
	if (TOD_Sky)
	{
		//std::cout << "TOD_SKY: " << std::hex << std::uppercase << TOD_Sky << std::endl;
		auto unk1 = mex.Read<uintptr_t>(TOD_Sky + 0xB8);
		auto instance_list = mex.Read<uintptr_t>(unk1 + 0x0);

		auto instances = mex.Read<uintptr_t>(instance_list + 0x10);
		//std::cout << "instances: " << std::hex << std::uppercase << instances << std::endl;
		auto tSky = mex.Read<uintptr_t>(instances + 0x20);
		//std::cout << "tSky: " << std::hex << std::uppercase << tSky << std::endl;
		auto TOD_CycleParameters = mex.Read<uintptr_t>(tSky + 0x38);
		//std::cout << "TOD_CycleParameters: " << std::hex << std::uppercase << TOD_CycleParameters << std::endl;
		mex.Write<float>(TOD_CycleParameters + 0x10, 12.f);
		//std::cout << "TOD_SKY Written!" << std::endl;
	}
}

void SetFatBullet(Item item, BasePlayer enemy)
{
	if (!item.IsItemGun())
		return;
	auto Held = mex.Read<uintptr_t>(item.Item + 0x98); //BaseProjectile

	auto CreatedProjectiles = mex.Read<uintptr_t>(Held + 0x338);

	//std::cout << "CreatedProjectiles: " << std::hex << std::uppercase << CreatedProjectiles << std::endl;

	auto CreatedProjectilesArray = mex.Read<uintptr_t>(CreatedProjectiles + 0x10);

	const auto size = mex.Read<int>(CreatedProjectiles + 0x18);

	for (int i = 0u; i < size; i++)
	{
		uintptr_t Projectile = mex.Read<uintptr_t>(CreatedProjectilesArray + (0x20 + (i * 0x8)));

		std::cout << "Thicc: " << mex.Read<float>(Projectile + 0x2C) << std::endl;
		mex.Write<float>(Projectile + 0x2C, 1.5f);
		//std::cout << "Projectile: " << std::hex << std::uppercase << Projectile << std::endl;
	}
}

void SetSilentAim(Item item, BasePlayer enemy)
{
	auto Held = mex.Read<uintptr_t>(item.Item + 0x98); //BaseProjectile

	auto CreatedProjectiles = mex.Read<uintptr_t>(Held + 0x338);

	//std::cout << "CreatedProjectiles: " << std::hex << std::uppercase << CreatedProjectiles << std::endl;

	auto CreatedProjectilesArray = mex.Read<uintptr_t>(CreatedProjectiles + 0x10);

	const auto size = mex.Read<int>(CreatedProjectiles + 0x18);

	for (int i = 0u; i < size; i++)
	{
		uintptr_t Projectile = mex.Read<uintptr_t>(CreatedProjectilesArray + (0x20 + (i * 0x8)));

		auto currentpos = mex.Read<Vector3>(Projectile + 0x124);
		std::cout << "Projectile Pos: " << std::hex << std::uppercase << currentpos.x << " | " << currentpos.y << " | " << currentpos.z << std::endl;
		auto enemypos = enemy.GetBonePosition(head);
		//mex.Write<float>(Projectile + 0x124, enemypos);
		//mex.Write<Vector3>(Projectile + 0x138, enemypos);
		//mex.Write<Vector3>(Projectile + 0x144, enemypos);
		auto newpos = mex.Read<Vector3>(Projectile + 0x124);
		std::cout << "New Pos: " << std::hex << std::uppercase << newpos.x << " | " << newpos.y << " | " << newpos.z << std::endl;
		//mex.Write<float>(Projectile + 0x2C, 5.f);
		//std::cout << "Projectile: " << std::hex << std::uppercase << Projectile << std::endl;
	}
}

void PrintProjectileSpeed(Item item, BasePlayer enemy)
{
	auto Held = mex.Read<uintptr_t>(item.Item + 0x98); //BaseProjectile

	auto CreatedProjectiles = mex.Read<uintptr_t>(Held + 0x338);

	//std::cout << "CreatedProjectiles: " << std::hex << std::uppercase << CreatedProjectiles << std::endl;

	auto CreatedProjectilesArray = mex.Read<uintptr_t>(CreatedProjectiles + 0x10);

	const auto size = mex.Read<int>(CreatedProjectiles + 0x18);

	for (int i = 0u; i < size; i++)
	{
		uintptr_t Projectile = mex.Read<uintptr_t>(CreatedProjectilesArray + (0x20 + (i * 0x8)));
		auto ItemModProjectile = mex.Read<uintptr_t>(Projectile + 0xE8);
		auto ProjectileSpeed = mex.Read<float>(ItemModProjectile + 0x34);
		std::cout << "Projectile Speed: " << ProjectileSpeed << std::endl;
	}
}

Vector3 Prediction(const Vector3& LP_Pos, BasePlayer Player, BoneList Bone)
{
	Vector3 BonePos = Player.GetBonePosition(Bone);
	float Dist = Math::Calc3D_Dist(LP_Pos, BonePos);

	if (Dist > 0.001f) {
		float BulletTime = Dist / LocalPlayer.GetHeldItem().GetBulletSpeed();
		Vector3 vel = Player.GetVelocity();
		Vector3 PredictVel = vel * BulletTime * 0.75f;
		BonePos += PredictVel;
		BonePos.y += (4.905f * BulletTime * BulletTime);
	} return BonePos;
}

void FatBulletThread()
{
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		try
		{
			if (settings::misc::fat_bullet)
			{
				SetFatBullet(LocalPlayer.GetHeldItem(), closestPlayer);
			}
		}
		catch (...)
		{
			std::cout << "FatBulletThread Error!" << std::endl;
		}
	}
}

void DoGameHax()
{
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		try
		{
			if (!LocalPlayer.IsSleeping())
			{
				if (settings::misc::debug_cam)
					LocalPlayer.SetAdminFlag();
				if (settings::misc::spider)
					LocalPlayer.DoSpider();
				if (settings::misc::automatic)
					LocalPlayer.GetHeldItem().SetAutomatic();
				if (settings::misc::no_recoil)
					LocalPlayer.GetHeldItem().SetNoRecoil();
				if (settings::misc::always_day)
					SetAlwaysDay();

				//std::cout << LocalPlayer.GetHeldItem().GetItemClassName().c_str() << std::endl;
			}
		}
		catch (...)
		{
			std::cout << "DGH Error." << std::endl;
		}
	}
}

void PlayerThreadFunc()
{
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		const auto game_window = FindWindowW(L"UnityWndClass", nullptr);
		if (GetForegroundWindow() != game_window)
		{
			//std::cout << "Player Loop: GetForegroundWindow() != game_window" << std::endl;
			continue;
		}
		UnityBaseAddress = mex.GetModuleBase("UnityPlayer.dll");
		//std::cout << "Unity Assembly: " << std::hex << std::uppercase << UnityBaseAddress << std::endl;
		if (!UnityBaseAddress)
		{
			//std::cout << "!UnityBaseAddress" << std::endl;
			continue;
		}

		GOM = mex.Read<uintptr_t>(UnityBaseAddress + 0x17A6AD8);
		if (!GOM)
		{
			//std::cout << "!GABaseAddress" << std::endl;
			continue;
		}

		GABaseAddress = mex.GetModuleBase("GameAssembly.dll");
		if (!GABaseAddress)
		{
			//std::cout << "!GABaseAddress" << std::endl;
			continue;
		}

		//std::cout << "Game Assembly: " << std::hex << std::uppercase << GABaseAddress << std::endl;

		BaseNetworkable = scan_for_klass("BaseNetworkable");
		if (!BaseNetworkable)
		{
			//std::cout << "!BaseNetworkable" << std::endl;
			continue;
		}

		TOD_Sky = scan_for_klass("TOD_Sky");
		if (!TOD_Sky)
		{
			//std::cout << "!TOD_Sky" << std::endl;
			continue;
		}

		const auto unk1 = mex.Read<uintptr_t>(BaseNetworkable + 0xB8);
		if (!unk1)
		{
			std::cout << "!unk1" << std::endl;
			return;
		}

		const auto client_entities = mex.Read<uintptr_t>(unk1);
		if (!client_entities)
		{
			std::cout << "!client_entities" << std::endl;
			return;
		}

		const auto entity_realm = mex.Read<uintptr_t>(client_entities + 0x10);
		if (!entity_realm)
		{
			std::cout << "!entity_realm" << std::endl;
			return;
		}

		const auto buffer_list = mex.Read<uintptr_t>(entity_realm + 0x28);
		if (!buffer_list)
		{
			std::cout << "!buffer_list" << std::endl;
			return;
		}

		const auto object_list = mex.Read<uintptr_t>(buffer_list + 0x18);
		if (!object_list)
		{
			std::cout << "!object_list" << std::endl;
			return;
		}

		const auto object_list_size = mex.Read<std::uint32_t>(buffer_list + 0x10);

		try
		{
			//std::ofstream classfile;
			//classfile.open("C:\\Aspect Rust\\classes.txt", std::ios_base::app);
			for (auto i = 0; i < object_list_size; i++)
			{
				const auto current_object = mex.Read<uintptr_t>(object_list + (0x20 + (i * 8)));

				if (!current_object)
				{
					//std::cout << "!current_object" << std::endl;
					continue;
				}
				//std::cout << "current_object: " << std::hex << std::uppercase << current_object << std::endl;
				const auto baseObject = mex.Read<uintptr_t>(current_object + 0x10);

				if (!baseObject)
					continue;

				const auto object = mex.Read<uintptr_t>(baseObject + 0x30);

				if (!object)
					continue;

				WORD tag = mex.Read<WORD>(object + 0x54);

				DWORD64 localElement = mex.Read<DWORD64>(object_list + 0x20);
				DWORD64 localBO = mex.Read<DWORD64>(localElement + 0x10);
				DWORD64 localPlayer = mex.Read<DWORD64>(localBO + 0x30);
				DWORD64 localOC = mex.Read<DWORD64>(localPlayer + 0x30);
				DWORD64 localT = mex.Read<DWORD64>(localOC + 0x8);
				DWORD64 localVS = mex.Read<DWORD64>(localT + 0x38);
				localPos = mex.Read<Vector3>(localVS + 0x90);

				std::string class_name = get_class_name(current_object);

				if (tag == 6)
				{
					char className[64];
					auto name_pointer = mex.Read<uint64_t>(object + 0x60);
					mex.Read(name_pointer, &className, sizeof(className));
					DWORD64 objectClass = mex.Read<DWORD64>(object + 0x30);
					DWORD64 entity = mex.Read<DWORD64>(objectClass + 0x18);
					uintptr_t player = mex.Read<uintptr_t>(entity + 0x28);

					//Get Player Position
					DWORD64 transform = mex.Read<DWORD64>(objectClass + 0x8);
					DWORD64 visualState = mex.Read<DWORD64>(transform + 0x38);
					Vector2 ScreenPos;
					Vector3 Pos = mex.Read<Vector3>(visualState + 0x90);
					//Get Player Name.
					auto Distance = Math::Calc3D_Dist(localPos, Pos);

					BasePlayer bp;
					bp.Player = player;
					bp.ObjectClass = objectClass;
					mex.Read(player, &bp.buffer, sizeof(bp.buffer));

					//Get Player Health
					auto player_health = mex.Read<float>(player + 0x1F4);
					float healthf = nearbyint(player_health);
					int health = (int)(healthf);
					//ent.TeamId = teamid;
					if (strcmp(className, "LocalPlayer") != 0)
					{
						bp.IsLocalPlayer = false;
						//if (teamid == LocalTeamID)
					}
					else
					{
						bp.IsLocalPlayer = true;
						LocalPlayer = bp;
						//LocalTeamID = teamid
						//printf("Object: %X\n", object);
						//std::cout << "object: " << std::hex << std::uppercase << object << std::endl;

					}

					const auto player_list_iter = std::find(PlayerList.begin(), PlayerList.end(), bp);

					if (player_list_iter != PlayerList.end() && health == 0)
					{
						PlayerList.erase(player_list_iter);
						continue;
					}
					else if (player_list_iter != PlayerList.end() && health > 0)
					{
						continue;
					}

					if (PlayerList.size() > 500)
						PlayerList.erase(PlayerList.begin());
					PlayerList.push_back(bp);
				}

				else if (class_name.find("OreRe") != std::string::npos)
				{
					OreClass ore;
					ore.Ore = current_object;
					const auto unk1 = mex.Read<uintptr_t>(ore.Ore + 0x10);

					if (!unk1)
						continue;

					const auto unk2 = mex.Read<uintptr_t>(unk1 + 0x30);

					if (!unk2)
						continue;



					const auto unk3 = mex.Read<uintptr_t>(unk2 + 0x30);

					if (!unk3)
						continue;



					/* shouldn't be needed, but in case */
					if (!ore.Ore)
						continue;

					Vector2 ScreenPos;
					Vector3 OrePos = get_obj_pos(unk3);

					auto distance = Math::Calc3D_Dist(localPos, OrePos);
					if (distance < settings::ore::max_distance)
					{
						if (OreList.size() > 100)
							OreList.erase(OreList.begin());
						OreList.push_back(ore);
					}

				}

				else if (class_name.find("Collectible") != std::string::npos)
				{
					CollectableClass object;
					object.Object = current_object;
					const auto unk1 = mex.Read<uintptr_t>(object.Object + 0x10);

					if (!unk1)
						continue;

					const auto unk2 = mex.Read<uintptr_t>(unk1 + 0x30);

					if (!unk2)
						continue;



					const auto unk3 = mex.Read<uintptr_t>(unk2 + 0x30);

					if (!unk3)
						continue;



					/* shouldn't be needed, but in case */
					if (!object.Object)
						continue;

					Vector2 ScreenPos;
					Vector3 OrePos = get_obj_pos(unk3);

					auto distance = Math::Calc3D_Dist(localPos, OrePos);
					if (distance < settings::collectable::max_distance)
					{
						if (CollectibleList.size() > 200)
							CollectibleList.erase(CollectibleList.begin());;
						CollectibleList.push_back(object);
					}
				}

				else if (class_name.find("DroppedItem") != std::string::npos)
				{
					auto CurrentItem = mex.Read<uintptr_t>(current_object + 0x150);
					DWORD64 objectClass = mex.Read<DWORD64>(object + 0x30);
					DWORD64 transform = mex.Read<DWORD64>(objectClass + 0x8);
					DWORD64 visualState = mex.Read<DWORD64>(transform + 0x38);
					Vector2 ScreenPos;
					Vector3 Pos = mex.Read<Vector3>(visualState + 0x90);
					auto distance = Math::Calc3D_Dist(localPos, Pos);
					if (distance < settings::dropped::max_distance)
					{
						if (DroppedItemList.size() > 100)
							DroppedItemList.erase(DroppedItemList.begin());
						Item object;
						object.Item = CurrentItem;
						object.ObjectClass = objectClass;
						DroppedItemList.push_back(object);
					}
				}
			}
			//classfile.close();
		}

		catch (...)
		{

		}
	}
}

void AimbotThread()
{
	
}

void drawLoop(int Width, int Height)
{
	const auto game_window = FindWindowW(L"UnityWndClass", nullptr);
	if (GetForegroundWindow() != game_window)
	{
		//std::cout << "Draw Loop: GetForegroundWindow() != game_window" << std::endl;
		return;
	}
	ScreenWidth = Width;
	ScreenHeight = Height;
	//std::cout << "Setting Up Clocks!" << std::endl;
	ticks += 1;
	if (beforeclock == 0) {
		beforeclock = clock();
	}

	DrawWString(L"Malkova", 15, 0, 0, 250, 0, 0);
	DrawCircle(ScreenWidth / 2, ScreenHeight / 2, settings::aimbot::fov, 0.5, 1, 1, 1, 1, false);
	if (settings::misc::crosshair)
	{
		//void DrawCircle(float x, float y, float radius, float thickness, float r, float g, float b, float a, bool filled);
		DrawCircle(midX, midY, 4, 0, 0, 255, 255, 1, true);
	}

	if (GetAsyncKeyState(VK_INSERT) & 1)
	{
		MenuOpen = !MenuOpen;
	}

	if (menu.IsReady() == false)
	{
		//Player ESP
		menu.AddFolder(L"Player ESP", &settings::player::folder, 1);
		menu.AddOption(L"Enabled", &settings::player::enabled, &settings::player::folder, 1, T_BOOL);
		menu.AddOption(L"Show Sleepers", &settings::player::showsleepers, &settings::player::folder, 1, T_BOOL);
		menu.AddOption(L"Show Names", &settings::player::name, &settings::player::folder, 1, T_BOOL);
		menu.AddOption(L"Show Distance", &settings::player::distance, &settings::player::folder, 1, T_BOOL);
		menu.AddOption(L"Show Snaplines", &settings::player::snaplines, &settings::player::folder, 1, T_BOOL);
		menu.AddOption(L"Show Health", &settings::player::health, &settings::player::folder, 1, T_BOOL);
		menu.AddOption(L"Show Box", &settings::player::box, &settings::player::folder, 1, T_BOOL);
		menu.AddOption(L"Show Held", &settings::player::held_item, &settings::player::folder, 1, T_BOOL);
		menu.AddOption(L"Show Hot Bar", &settings::player::hotbar, &settings::player::folder, 1, T_BOOL);

		//Ore ESP
		menu.AddFolder(L"Ore ESP", &settings::ore::folder, 1);
		menu.AddOption(L"Enabled", &settings::ore::enabled, &settings::ore::folder, 1, T_BOOL);
		menu.AddOption(L"Show Stone Ore", &settings::ore::stone, &settings::ore::folder, 1, T_BOOL);
		menu.AddOption(L"Show Metal Ore", &settings::ore::metal, &settings::ore::folder, 1, T_BOOL);
		menu.AddOption(L"Show Sulfur Ore", &settings::ore::sulfur, &settings::ore::folder, 1, T_BOOL);

		menu.AddFolder(L"Collectible ESP", &settings::collectable::folder, 1);
		menu.AddOption(L"Enabled", &settings::collectable::enabled, &settings::collectable::folder, 1, T_BOOL);

		menu.AddFolder(L"Aimbot", &settings::aimbot::folder, 1);
		menu.AddOption(L"Enabled", &settings::aimbot::enabled, &settings::aimbot::folder, 1, T_BOOL);
		menu.AddOption(L"Disable Recoil", &settings::aimbot::disable_recoil, &settings::aimbot::folder, 1, T_BOOL);
		//menu.AddOption(L"Restore Snap", &settings::aimbot::re, &settings::aimbot::folder, 1, T_BOOL);

		menu.AddFolder(L"Misc", &settings::misc::folder, 1);
		menu.AddOption(L"Spider", &settings::misc::spider, &settings::misc::folder, 1, T_BOOL);
		menu.AddOption(L"Crosshair", &settings::misc::crosshair, &settings::misc::folder, 1, T_BOOL);
		menu.AddOption(L"Always Day", &settings::misc::always_day, &settings::misc::folder, 1, T_BOOL);
		menu.AddOption(L"Fat Bullet", &settings::misc::fat_bullet, &settings::misc::folder, 1, T_BOOL);
		menu.AddOption(L"Admin Flags", &settings::misc::debug_cam, &settings::misc::folder, 1, T_BOOL);
		menu.AddOption(L"Always Auto", &settings::misc::automatic, &settings::misc::folder, 1, T_BOOL);
		menu.AddOption(L"[Rage] No Recoil", &settings::misc::no_recoil, &settings::misc::folder, 1, T_BOOL);
	}

	if (menu.Visible)
	{
		menu.Draw_Menu();
	}

	menu.Navigation();

	//std::cout << "Check if menu open." << std::endl;

	//std::cout << "GOM: " << std::hex << std::uppercase << GOM << std::endl;
	if (!GOM)
	{
		std::cout << "!GOM" << std::endl;
		return;
	}

	DWORD64 taggedObjects = mex.Read<DWORD64>(GOM + 0x8);
	if (!taggedObjects)
	{
		std::cout << "!taggedObjects" << std::endl;
		return;
	}

	DWORD64 gameObject = mex.Read<DWORD64>(taggedObjects + 0x10);
	if (!gameObject)
	{
		std::cout << "!gameObject" << std::endl;
		return;
	}

	DWORD64 objClass = mex.Read<DWORD64>(gameObject + 0x30);
	if (!objClass)
	{
		std::cout << "!objClass" << std::endl;
		return;
	}

	DWORD64 ent = mex.Read<DWORD64>(objClass + 0x18);
	if (!ent)
	{
		std::cout << "!ent" << std::endl;
		return;
	}

	pViewMatrix = mex.Read<Matrix4x4>(ent + 0xDC);

	if (true)
	{
		FOV = settings::aimbot::fov;
		if (settings::player::enabled)
		{
			for (BasePlayer player : PlayerList)
			{
				auto pos = player.GetBonePosition(neck);
				if (player.IsLocalPlayer)
				{
					localPos = pos;
					LocalPlayer = player;
					continue;
				}
				auto pos2 = player.GetBonePosition(jaw);
				auto pos3 = player.GetBonePosition(r_toe);
				auto player_health = player.GetHealth();
				float healthf = nearbyint(player_health);
				int health = (int)(healthf);

				auto distance = Math::Calc3D_Dist(localPos, pos);
				if (distance < 300 && player_health > 0)
				{
					Vector2 ScreenPos;
					Vector2 ScreenPos2;
					Vector2 ScreenPos3;
					if (WorldToScreen(pos, ScreenPos) && WorldToScreen(pos2, ScreenPos2) && WorldToScreen(pos3, ScreenPos3))
					{
						auto HeldGun = player.GetHeldItem().GetItemName();
						char healthbuffer[0x20]{};
						char distancebuffer[0x20]{};
						char healthbuffer2[0x20]{};
						char distancebuffer2[0x20]{};
						sprintf(healthbuffer, "[%d HP]", health);
						sprintf(distancebuffer, "[%dm]", (int)distance);
						sprintf(healthbuffer2, "HP: %d", health);
						sprintf(distancebuffer2, "Distance: %dm", (int)distance);
						BoneList Bones[4] = {
							/*UP*/l_upperarm, r_upperarm,
							/*DOWN*/r_foot, l_foot
						};
						Vector2 BonesPos[4];

						//get bones screen pos
						for (int j = 0; j < 4; j++) {
							if (!WorldToScreen(player.GetBonePosition(Bones[j]), BonesPos[j]))
								return;
						}

						if (player.IsSleeping() && settings::player::showsleepers)
						{
							int yoffset = 0;
							if (settings::player::name)
							{
								DrawWString(player.GetName(), 15, ScreenPos.x, ScreenPos.y + yoffset, 27, 198, 250);
								yoffset += 15;
							}
							if (settings::player::health)
							{
								DrawString(healthbuffer, 15, ScreenPos.x, ScreenPos.y + yoffset, 27, 198, 250);
									
								yoffset += 15;
							}
							if (settings::player::distance)
							{
								DrawString(distancebuffer, 15, ScreenPos.x, ScreenPos.y + yoffset, 27, 198, 250);
								yoffset += 15;
							}
							if (settings::player::held_item)
							{
								DrawWString(HeldGun, 15, ScreenPos.x, ScreenPos.y + yoffset, 27, 198, 250);
								yoffset += 15;
							}
							if (settings::player::hotbar)
							{
								if (curFOV <= 30)
								{
									DrawBox(0, ScreenHeight / 4, 50, 100, 1, 0, 0, 0, 0, 1);
									int boxoffset = 15;
									for (int i = 0; i < 6; i++)
									{
										auto ItemName = player.GetPlayerInventory().GetBelt().GetItem(i).GetItemName();
										DrawWString(ItemName, 15, 0, (ScreenHeight / 4) + yoffset, 27, 198, 250);
										yoffset += 15;
									}
								}
							}

							continue;
						}
						else if (player.IsSleeping() && !settings::player::showsleepers)
						{
							continue;
						}

						curFOV = distance_cursor(ScreenPos);
						if (FOV > curFOV && !player.IsLocalPlayer && player.Player != LocalPlayer.Player)
						{
							FOV = curFOV;
							closestPlayer = player;
						}


						if (player.GetPlayerModel().IsVisible())
						{
							int yoffset = 0;
							if (settings::player::name)
							{
								DrawWString(player.GetName(), 15, ScreenPos.x, ScreenPos.y + yoffset, 199, 88, 14);
								yoffset += 15;
							}

							if (settings::player::health)
							{
								DrawString(healthbuffer, 15, ScreenPos.x, ScreenPos.y + yoffset, 199, 88, 14);
								yoffset += 15;
							}

							if (settings::player::distance)
							{
								DrawString(distancebuffer, 15, ScreenPos.x, ScreenPos.y + yoffset, 199, 88, 14);
								yoffset += 15;
							}
							if (settings::player::snaplines)
							{
								DrawLine(midX, midY, ScreenPos.x, ScreenPos.y + 45, 1, 255, 0, 0, 1);
								yoffset += 15;
							}

							if (settings::player::held_item)
							{
								DrawWString(HeldGun, 15, ScreenPos.x, ScreenPos.y + yoffset, 199, 88, 14);
								yoffset += 15;
							}

							if (settings::player::hotbar)
							{
								if (curFOV <= 30 && player == closestPlayer)
								{
									int boxoffset = ScreenHeight / 4;
									DrawBox(0, boxoffset - 2, 204, 174, 1, 1, 0, 0, 0.5, 1);
									DrawBox(2, boxoffset, 200, 170, 1, 0, 0, 0, 0.5, 1);
									boxoffset += 2;
									DrawWString(player.GetName(), 15, 2, boxoffset, 27, 198, 250);
									boxoffset += 15;
									DrawString(healthbuffer2, 15, 2, boxoffset, 	27, 198, 250);
									boxoffset += 15;
									DrawString(distancebuffer2, 15, 2, boxoffset, 27, 198, 250);
									boxoffset += 30;
									for (int i = 0; i < 6; i++)
									{
										auto ItemName = player.GetPlayerInventory().GetBelt().GetItem(i).GetItemName();
										DrawWString(ItemName, 15, 2, boxoffset, 27, 198, 250);
										boxoffset += 15;
									}
								}
							}
							Vector2 tempFeetR;
							Vector2 tempFeetL;
							WorldToScreen(player.GetBonePosition(r_foot), tempFeetR);
							WorldToScreen(player.GetBonePosition(l_foot), tempFeetL);
							Vector2 tempHead;
							WorldToScreen(player.GetBonePosition(jaw) + Vector3(0.f, 0.16f, 0.f), tempHead);
							Vector2 tempFeet = (tempFeetR + tempFeetL) / 2.f;
							float Entity_h = tempHead.y - tempFeet.y;
							float w = Entity_h / 4;
							float Entity_x = tempFeet.x - w;
							float Entity_y = tempFeet.y;
							float Entity_w = Entity_h / 2;
							if (settings::player::box)
							{
							/*	DrawLine(BonesPos[0].x, BonesPos[0].y, BonesPos[1].x, BonesPos[1].y, 1, 1, 0, 0, 1);
								DrawLine(BonesPos[0].x, BonesPos[0].y, BonesPos[1].x, BonesPos[1].y, 1, 1, 0, 0, 1);
								DrawLine(BonesPos[0].x, BonesPos[0].y, BonesPos[3].x, BonesPos[3].y, 1, 1, 0, 0, 1);
								DrawLine(BonesPos[0].x, BonesPos[0].y, BonesPos[3].x, BonesPos[3].y, 1, 1, 0, 0, 1);
								DrawLine(BonesPos[2].x, BonesPos[2].y, BonesPos[1].x, BonesPos[1].y, 1, 1, 0, 0, 1);
								DrawLine(BonesPos[2].x, BonesPos[2].y, BonesPos[1].x, BonesPos[1].y, 1, 1, 0, 0, 1);
								DrawLine(BonesPos[2].x, BonesPos[2].y, BonesPos[3].x, BonesPos[3].y, 1, 1, 0, 0, 1);
								DrawLine(BonesPos[2].x, BonesPos[2].y, BonesPos[3].x, BonesPos[3].y, 1, 1, 0, 0, 1);*/
								DrawBox(Entity_x, Entity_y, Entity_w, Entity_h, 1.f, 0, 242, 255, 1, false);
								DrawBox(Entity_x, Entity_y, Entity_w - 1, Entity_h - 1, 1.f, 196, 203, 204, 0.4, false);
							}
						}
						else
						{
							int yoffset = 0;
							if (settings::player::name)
							{
								DrawWString(player.GetName(), 15, ScreenPos.x, ScreenPos.y + yoffset, 242, 10, 10);
								yoffset += 15;
							}

							if (settings::player::health)
							{
								DrawString(healthbuffer, 15, ScreenPos.x, ScreenPos.y + yoffset, 242, 10, 10);
								yoffset += 15;
							}

							if (settings::player::distance)
							{
								DrawString(distancebuffer, 15, ScreenPos.x, ScreenPos.y + yoffset, 242, 10, 10);
								yoffset += 15;
							}

							if (settings::player::snaplines)
							{
								DrawLine(midX, midY, ScreenPos.x, ScreenPos.y + 45, 1, 255, 0, 0, 1);
								yoffset += 15;
							}

							if (settings::player::held_item)
							{
								DrawWString(HeldGun, 15, ScreenPos.x, ScreenPos.y + yoffset, 242, 10, 10);
								yoffset += 15;
							}

							if (settings::player::hotbar)
							{
								if (curFOV <= 30 && player == closestPlayer)
								{
									int boxoffset = ScreenHeight / 4;
									DrawBox(0, boxoffset - 2, 204, 174, 1, 1, 0, 0, 0.5, 1);
									DrawBox(2, boxoffset, 200, 170, 1, 0, 0, 0, 0.5, 1);
									boxoffset += 2;
									DrawWString(player.GetName(), 15, 2, boxoffset, 27, 198, 250);
									boxoffset += 15;
									DrawString(healthbuffer2, 15, 2, boxoffset, 27, 198, 250);
									boxoffset += 15;
									DrawString(distancebuffer2, 15, 2, boxoffset, 27, 198, 250);
									boxoffset += 30;
									for (int i = 0; i < 6; i++)
									{
										auto ItemName = player.GetPlayerInventory().GetBelt().GetItem(i).GetItemName();
										DrawWString(ItemName, 15, 2, boxoffset, 27, 198, 250);
										boxoffset += 15;
									}
								}
							}
														Vector2 tempFeetR;
							Vector2 tempFeetL;
							WorldToScreen(player.GetBonePosition(r_foot), tempFeetR);
							WorldToScreen(player.GetBonePosition(l_foot), tempFeetL);
							Vector2 tempHead;
							WorldToScreen(player.GetBonePosition(jaw) + Vector3(0.f, 0.16f, 0.f), tempHead);
							Vector2 tempFeet = (tempFeetR + tempFeetL) / 2.f;
							float Entity_h = tempHead.y - tempFeet.y;
							float w = Entity_h / 4;
							float Entity_x = tempFeet.x - w;
							float Entity_y = tempFeet.y;
							float Entity_w = Entity_h / 2;
							if (settings::player::box)
							if (settings::player::box)
							{
							/*	DrawLine(BonesPos[0].x, BonesPos[0].y, BonesPos[1].x, BonesPos[1].y, 1, 1, 0, 0, 1);
								DrawLine(BonesPos[0].x, BonesPos[0].y, BonesPos[1].x, BonesPos[1].y, 1, 1, 0, 0, 1);
								DrawLine(BonesPos[0].x, BonesPos[0].y, BonesPos[3].x, BonesPos[3].y, 1, 1, 0, 0, 1);
								DrawLine(BonesPos[0].x, BonesPos[0].y, BonesPos[3].x, BonesPos[3].y, 1, 1, 0, 0, 1);
								DrawLine(BonesPos[2].x, BonesPos[2].y, BonesPos[1].x, BonesPos[1].y, 1, 1, 0, 0, 1);
								DrawLine(BonesPos[2].x, BonesPos[2].y, BonesPos[1].x, BonesPos[1].y, 1, 1, 0, 0, 1);
								DrawLine(BonesPos[2].x, BonesPos[2].y, BonesPos[3].x, BonesPos[3].y, 1, 1, 0, 0, 1);
								DrawLine(BonesPos[2].x, BonesPos[2].y, BonesPos[3].x, BonesPos[3].y, 1, 1, 0, 0, 1);*/

									DrawBox(Entity_x, Entity_y, Entity_w, Entity_h, 1.f, 255, 0, 0, 1, false);
									DrawBox(Entity_x, Entity_y, Entity_w - 1, Entity_h - 1, 1.f, 196, 203, 204, 0.4, false);
							}
						}
					}
				}
			}
		}

		if (settings::ore::enabled)
		{
			for (OreClass ore : OreList)
			{
				const auto unk1 = mex.Read<uintptr_t>(ore.Ore + 0x10);

				if (!unk1)
					continue;

				const auto unk2 = mex.Read<uintptr_t>(unk1 + 0x30);

				if (!unk2)
					continue;



				const auto unk3 = mex.Read<uintptr_t>(unk2 + 0x30);

				if (!unk3)
					continue;



				/* shouldn't be needed, but in case */
				if (!ore.Ore)
					continue;




				const auto oreName = mex.Read<uintptr_t>(unk2 + 0x60);
				std::string name = read_ascii(oreName, 64);

				if (name.find("stone-ore") != std::string::npos)
				{
					if (settings::ore::stone)
					{
						Vector2 ScreenPos;
						Vector3 OrePos = get_obj_pos(unk3);

						if (WorldToScreen(OrePos, ScreenPos))
						{
							auto distance = Math::Calc3D_Dist(localPos, OrePos);
							if (distance < settings::ore::max_distance)
							{
								int yoffset = 0;
								if (settings::ore::name)
								{
									DrawString("Stone Ore", 15, ScreenPos.x, ScreenPos.y + yoffset, 95, 118, 125);
									yoffset += 15;
								}
								if (settings::ore::distance)
								{
									char buffer[0x100]{};
									sprintf(buffer, "[%dm]", (int)distance);
									DrawString(buffer, 15, ScreenPos.x, ScreenPos.y + yoffset, 95, 118, 125);
									yoffset += 15;
								}
							}
						}
					}
					continue;
				}
				else if (name.find("metal-ore") != std::string::npos)
				{
					if (settings::ore::metal)
					{
						Vector2 ScreenPos;
						Vector3 OrePos = get_obj_pos(unk3);

						if (WorldToScreen(OrePos, ScreenPos))
						{
							auto distance = Math::Calc3D_Dist(localPos, OrePos);
							if (distance < settings::ore::max_distance)
							{
								int yoffset = 0;
								if (settings::ore::name)
								{
									DrawString("Metal Ore", 15, ScreenPos.x, ScreenPos.y + yoffset, 38, 112, 1);
									//DrawWString(L"Metal Ore New", 15, ScreenPos.x, ScreenPos.y + yoffset, 0.15, 0.45, 0);
									yoffset += 15;
								}
								if (settings::ore::distance)
								{
									char buffer[0x100]{};
									sprintf(buffer, "[%dm]", (int)distance);
									DrawString(buffer, 15, ScreenPos.x, ScreenPos.y + yoffset, 38, 112, 1);
								}
							}
						}
					}
					continue;
				}
				else if (name.find("sulfur-ore") != std::string::npos)
				{
					if (settings::ore::sulfur)
					{
						Vector2 ScreenPos;
						Vector3 OrePos = get_obj_pos(unk3);

						if (WorldToScreen(OrePos, ScreenPos))
						{
							auto distance = Math::Calc3D_Dist(localPos, OrePos);
							if (distance < settings::ore::max_distance)
							{
								int yoffset = 0;
								if (settings::ore::name)
								{
									DrawString("Sulfur Ore", 15, ScreenPos.x, ScreenPos.y + yoffset, 242, 235, 31);
									yoffset += 15;
								}
								if (settings::ore::distance)
								{
									char buffer[0x100]{};
									sprintf(buffer, "[%dm]", (int)distance);
									DrawString(buffer, 15, ScreenPos.x, ScreenPos.y + yoffset, 242, 235, 31);
								}
							}
						}
					}
					continue;
				}
			}
		}

		if (settings::collectable::enabled)
		{
			for (CollectableClass obj : CollectibleList)
			{
				const auto unk1 = mex.Read<uintptr_t>(obj.Object + 0x10);
				if (!unk1)
					continue;
				const auto unk2 = mex.Read<uintptr_t>(unk1 + 0x30);
				if (!unk2)
					continue;
				const auto unk3 = mex.Read<uintptr_t>(unk2 + 0x30);
				if (!unk3)
					continue;
				if (!obj.Object)
					continue;

				const auto objName = mex.Read<uintptr_t>(unk2 + 0x60);
				std::string name = read_ascii(objName, 64);
				if (name.find("hemp") != std::string::npos)
				{
					Vector2 ScreenPos;
					Vector3 OrePos = get_obj_pos(unk3);

					if (WorldToScreen(OrePos, ScreenPos))
					{
						auto distance = Math::Calc3D_Dist(localPos, OrePos);
						if (distance < settings::collectable::max_distance)
						{
							char buffer[0x100]{};
							sprintf(buffer, "Hemp\n[%dm]", (int)distance);
							auto text = s2ws(buffer);
							DrawString(buffer, 15, ScreenPos.x, ScreenPos.y, 65, 252, 3);
						}
					}
				}
				else if (name.find("metal-collect") != std::string::npos)
				{
					Vector2 ScreenPos;
					Vector3 OrePos = get_obj_pos(unk3);

					if (WorldToScreen(OrePos, ScreenPos))
					{
						auto distance = Math::Calc3D_Dist(localPos, OrePos);
						if (distance < 300)
						{
							char buffer[0x100]{};
							sprintf(buffer, "Metal Col\n[%dm]", (int)distance);
							auto text = s2ws(buffer);
							DrawString(buffer, 15, ScreenPos.x, ScreenPos.y, 65, 252, 3);
						}
					}
				}
				else if (name.find("sulfur") != std::string::npos)
				{
					Vector2 ScreenPos;
					Vector3 OrePos = get_obj_pos(unk3);

					if (WorldToScreen(OrePos, ScreenPos))
					{
						auto distance = Math::Calc3D_Dist(localPos, OrePos);
						if (distance < 300)
						{
							char buffer[0x100]{};
							sprintf(buffer, "Sulfur Col\n[%dm]", (int)distance);
							auto text = s2ws(buffer);
							DrawString(buffer, 15, ScreenPos.x, ScreenPos.y, 65, 252, 3);
						}
					}
				}

				else if (name.find("stone") != std::string::npos)
				{
					Vector2 ScreenPos;
					Vector3 OrePos = get_obj_pos(unk3);

					if (WorldToScreen(OrePos, ScreenPos))
					{
						auto distance = Math::Calc3D_Dist(localPos, OrePos);
						if (distance < 300)
						{
							char buffer[0x100]{};
							sprintf(buffer, "Stone Col\n[%dm]", (int)distance);
							auto text = s2ws(buffer);
							DrawString(buffer, 15, ScreenPos.x, ScreenPos.y, 65, 252, 3);
						}
					}
				}
			}
		}

		if (settings::dropped::enabled)
		{
			for (Item item : DroppedItemList)
			{
				Vector3 Pos = item.GetVisualPosition();
				//std::cout << "Getting Item amount" << std::endl;
				auto amount = item.GetAmount();
				//std::cout << "Getting Item distance" << std::endl;
				auto distance = Math::Calc3D_Dist(localPos, Pos);
				//std::cout << "Distance: " << distance << std::endl;
				if (distance < settings::dropped::max_distance)
				{
					//std::cout << ws2s(item.GetItemName()) << " | " << amount << std::endl;
					Vector2 ScreenPos;
					if (WorldToScreen(Pos, ScreenPos))
					{
						char buffer[0x100]{};
						sprintf(buffer, "[%dm]\n[%d]", (int)distance, amount);
						DrawWString(item.GetItemName(), 15, ScreenPos.x, ScreenPos.y, .5, .5, .5);
						DrawString(buffer, 15, ScreenPos.x, ScreenPos.y + 15, .5, .5, .5);
					}
				}
			}
		}
	}

	try
	{
		if (settings::aimbot::enabled)
		{
			if (closestPlayer.Player != NULL && LocalPlayer.Player != NULL)
			{
				if ((GetKeyState(settings::aimbot::aim_key) & 0x8000))
				{
					Vector2 ScreenPos;
					auto Pos = closestPlayer.GetBonePosition(neck); //GetBonePosition(closestPlayer, neck);
					auto distance = Math::Calc3D_Dist(localPos, Pos);
					if (distance < 300)
					{
						if (WorldToScreen(Pos, ScreenPos))
						{
							auto fov = distance_cursor(ScreenPos);
							if (fov < settings::aimbot::fov)
							{
								if (settings::aimbot::use_mouse)
								{
									Vector2 target = smooth(ScreenPos);
									INPUT input;
									input.type = INPUT_MOUSE;
									input.mi.mouseData = 0;
									input.mi.time = 0;
									input.mi.dx = target.x;
									input.mi.dy = target.y;
									input.mi.dwFlags = MOUSEEVENTF_MOVE;
									SendInput(1, &input, sizeof(input));
								}
								else
								{
									auto OrigVA = LocalPlayer.GetPlayerInput().GetViewAngles();
									Vector3 LocalPos = LocalPlayer.GetBonePosition(neck); // GetBonePosition(LocalPlayer, neck);
									//Vector3 EnemyPos = Prediction(LocalPos, closestPlayer, neck);
									auto RecAng = LocalPlayer.GetPlayerInput().GetRecoilAngle(); //GetRA(LocalPlayer);
									Vector2 Offset = Math::CalcAngle(LocalPos, Pos) - LocalPlayer.GetPlayerInput().GetViewAngles();
									//printf("Offset VA: %f | %f\n", Offset.x, Offset.y);
									//Normalize(Offset.y, Offset.x);
									Vector2 AngleToAim = LocalPlayer.GetPlayerInput().GetViewAngles() + Offset;
									if (settings::aimbot::disable_recoil)
									{
										AngleToAim = AngleToAim - RecAng;
									}
									Normalize(AngleToAim.y, AngleToAim.x);
									LocalPlayer.GetPlayerInput().SetViewAngles(AngleToAim); // SetVA(LocalPlayer, AngleToAim);
								}
							}
						}
					}
				}
			}
		}
	}
	catch (...)
	{
		std::cout << "Aimbot Error!" << std::endl;
	}
}

void startoverlay()
{
	const auto game_window = FindWindowW(L"UnityWndClass", nullptr);
	DirectOverlaySetOption(D2DOV_DRAW_FPS | D2DOV_FONT_ARIAL | D2DOV_VSYNC | D2DOV_REQUIRE_FOREGROUND);
	DirectOverlaySetup(drawLoop, game_window);
	getchar();
}

int WINAPI main(HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	/*std::string title = "Malkova Rust | Version: " + sec.version;
	SetConsoleTitle(title.c_str());
	if (!sec.check_version())
	{
		std::cout << xorstr("[Malkova] Loader Version Outdated!").crypt_get() << std::endl << xorstr("[Malkova] Please go to https ://aspectnetwork.net/aspect/rust/malkova/download to get the latest download!").crypt_get();
		Sleep(10000);
	}
	else
	{
		
		std::cout << xorstr("[Malkova] Enter Username:").crypt_get() << std::endl;
		std::string username;
		std::cin >> username;
		std::cout << xorstr("[Malkova] Enter Password:").crypt_get() << std::endl;
		std::string password;
		std::cin >> password;
		
		if (sec.authenticate(username, password))*/
		{
			auto xorfolder = xorstr("C:\\Rust\\");
			folderpath = xorfolder.crypt_get();

			auto xorfix = xorstr("C:\\Rust\\Fix.dll");
			dllpath = xorfix.crypt_get();

			auto xordriver = xorstr("C:\\Rust\\Malkova.sys");
			driverpath = xordriver.crypt_get();

			system("CLS");
			auto IsRustOpen = FindWindowW(L"UnityWndClass", nullptr);
			bool DidWeInject = false;
			if (IsRustOpen)
			{
				InjectTheDll = false;
			}
			//std::cout << "Driver Path: " << driverpath.c_str() << std::endl;
			if (Bypass::Installer::InstallService("Amdkfca", "AMD Processor Driver", xordriver.crypt_get()) == 0) //https://i.gyazo.com/fd624705e5f8759abdb202fc1cc6ca65.png
			{
				if (Bypass::Driver::OpenDriver() == true)
				{
					if (Bypass::Driver::ProtectProcess(GetCurrentProcessId()) == true)
						std::cout << "[Malkova] Waiting For Game." << std::endl;
					else
						std::cout << "[Malkova] Could not protect process." << std::endl;

					
					std::map<std::uint32_t, std::uint8_t> UsedProcessIds;
					while (true)
					{
						//Sleep(250);
						auto ProcIds = GetProcessIds(std::wstring(L"RustClient.exe"));
						for (auto Id : ProcIds)
						{
							if (DidWeInject)
							{
								if (GetAsyncKeyState(VK_INSERT) & 1)
								{
									if (!overlaycreated)
									{
										std::thread PlayerThread(PlayerThreadFunc);
										PlayerThread.detach();
										std::thread GameHax(DoGameHax);
										GameHax.detach();
										std::thread FBT(FatBulletThread);
										FBT.detach();
										std::thread FBT2(FatBulletThread);
										FBT2.detach();
										startoverlay();
										overlaycreated = true;
									}
									else
										MenuOpen = !MenuOpen;
								}
								
								continue;
							}
							std::cout << "[Malkova] Loading!" << std::endl;
							if (InjectTheDll)
							{
								/*
								while (!(GetAsyncKeyState(VK_HOME) & 1))
								{
									std::this_thread::sleep_for(std::chrono::milliseconds(100));
								}
								*/
								//std::cout << "Injecting Haxx" << std::endl;
								//InjectIntoRust(Id);
								//std::cout << "Injected Haxx" << std::endl;
							}
							mex.Open("RustClient.exe");
							std::cout << "[Malkova] Loaded!" << std::endl;
							std::cout << "[Malkova] Press Insert AFTER You Join A Server." << std::endl;
							DidWeInject = true;
							//break;
							UsedProcessIds[Id] = 1;
							//delete CheatDll;
						}
					}
				}
				else
				{
					std::cout << "[Malkova] Failed to load: error #2." << std::endl;
				}
			}
			else
			{
				std::cout << "[Malkova] Failed to load: error #1." << std::endl;
			}
			Sleep(5000);
		}
		//else
		{
			Sleep(10000);
		}
//	}
	//return 1;
}

std::vector<std::uint32_t> GetProcessIds(const std::wstring& processName)
{
	std::vector<std::uint32_t> procs;
	PROCESSENTRY32W processInfo;
	processInfo.dwSize = sizeof(processInfo);

	HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (Snapshot == INVALID_HANDLE_VALUE) return procs;

	Process32FirstW(Snapshot, &processInfo);
	if (wcsstr(processName.c_str(), processInfo.szExeFile))
	{
		CloseHandle(Snapshot);
		procs.push_back(processInfo.th32ProcessID);
		return procs;
	}

	while (Process32NextW(Snapshot, &processInfo))
	{
		if (wcsstr(processName.c_str(), processInfo.szExeFile))
		{
			procs.push_back(processInfo.th32ProcessID);
		}
	}

	CloseHandle(Snapshot);
	return procs;
};

DWORD GetThreadId(DWORD dwProcessId)
{
	THREADENTRY32 threadinfo;
	threadinfo.dwSize = sizeof(threadinfo);

	HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, dwProcessId);
	if (Snapshot == INVALID_HANDLE_VALUE) return false;

	Thread32First(Snapshot, &threadinfo);

	while (Thread32Next(Snapshot, &threadinfo))
	{
		if (threadinfo.th32ThreadID && threadinfo.th32OwnerProcessID == dwProcessId)
		{
			CloseHandle(Snapshot);
			return threadinfo.th32ThreadID;
		}
	}
	CloseHandle(Snapshot);
	return 0;
};