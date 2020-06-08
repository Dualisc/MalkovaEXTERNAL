#pragma once
#include "../commons.h"

#include "utils/memory/mex.h"
#include "utils/math.h"

MemEx mex;

std::string ws2s(const std::wstring& wstr)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(wstr);
}

std::wstring s2ws(const std::string& str)
{
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}

std::wstring read_unicode(const std::uintptr_t address, std::size_t size)
{
	const auto buffer = std::make_unique<wchar_t[]>(size);
	mex.Read(address, buffer.get(), size * 2);
	return std::wstring(buffer.get());
}

std::string read_ascii(const std::uintptr_t address, std::size_t size)
{
	std::unique_ptr<char[]> buffer(new char[size]);
	mex.Read(address, buffer.get(), size);
	return std::string(buffer.get());
}

class WeaponInfo
{
public:
	uintptr_t Weapon;
	char buffer[0x1000];

public:
	int GetUID()
	{
		return *(int*)(buffer + 0x28);
	}
};

const unsigned short Crc16Table[256] = {
0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};


class Item
{
public:
	uintptr_t Item;
	uintptr_t ObjectClass;
	char buffer[0x1000];

private:
	unsigned short CRC(unsigned char* pcBlock, unsigned short len)
	{
		unsigned short crc = 0xFFFF;
		while (len--)
			crc = (crc << 8) ^ Crc16Table[(crc >> 8) ^ *pcBlock++];
		return crc;
	}

public:
	int GetAmount()
	{
		return mex.Read<int>(Item + 0x30);
	}

	int GetUID()
	{
		return mex.Read<int>(Item + 0x28);
	}

	std::wstring GetItemName()
	{
		auto Info = mex.Read<uintptr_t>((uintptr_t)Item + 0x20);
		auto DisplayName = mex.Read<uintptr_t>(Info + 0x20);
		//auto name = mex.Read<pMyUncStr>(DisplayName + 0x18);
		auto wname = read_unicode(DisplayName + 0x14, 0x64);
		if (!wname.empty())
		{
			return wname;
		}
		return L"No Item!";
	}

	bool IsItemGun()
	{
		auto ItemName = GetItemName();
		if (ItemName.find(L"rifle") != std::string::npos)
		{
			return true;
		}
		if (ItemName.find(L"pistol") != std::string::npos)
		{
			return true;
		}
		if (ItemName.find(L"bow") != std::string::npos)
		{
			return true;
		}
		if (ItemName.find(L"lmg") != std::string::npos)
		{
			return true;
		}
		if (ItemName.find(L"shotgun") != std::string::npos)
		{
			return true;
		}
		if (ItemName.find(L"smg") != std::string::npos)
		{
			return true;
		}
		else
			return false;
	}

	std::string GetItemClassName()
	{
		auto Held = mex.Read<uintptr_t>(Item + 0x98);
		const auto baseObject = mex.Read<uintptr_t>(Held + 0x10);

		const auto object = mex.Read<uintptr_t>(baseObject + 0x30);
		char className[64];
		auto name_pointer = mex.Read<uint64_t>(object + 0x60);
		mex.Read(name_pointer, &className, sizeof(className));
		return className;
	}

	float GetBulletSpeed()
	{
		auto ItemName = GetItemName();
		if (ItemName.find(L"rifle") != std::string::npos)
		{
			return 475.f;
		}
		if (ItemName.find(L"pistol") != std::string::npos)
		{
			return 300.f;
		}
		if (ItemName.find(L"bow") != std::string::npos)
		{
			return 50.f;
		}
		if (ItemName.find(L"lmg") != std::string::npos)
		{
			return 375.f;
		}
		if (ItemName.find(L"smg") != std::string::npos)
		{
			return 300.f;
		}
		else
			return 0;
	}
	

	void SetAutomatic()
	{
		auto Held = mex.Read<uintptr_t>(Item + 0x98); //BaseProjectile
		if (!Held)
			return;
		if (!IsItemGun())
			return;
		mex.Write<bool>(Held + 0x270, true); //automatic
	}

	void SetNoSpread()
	{
		if (!Item)
			return;

		auto Held = mex.Read<uintptr_t>(Item + 0x98); //BaseProjectile
		if (!Held)
			return;

		mex.Write<float>(Held + 0x304, -3.f); //stancePenalty
		mex.Write<float>(Held + 0x308, -3.f); //aimconePenalty
		mex.Write<float>(Held + 0x2D0, -3.f); //aimCone
		mex.Write<float>(Held + 0x2D4, -3.f); //hipAimCone
		mex.Write<float>(Held + 0x2D8, -3.f); //aimconePenaltyPerShot
		mex.Write<float>(Held + 0x2DC, -3.f); //aimConePenaltyMax
	}

	void SetFastReload()
	{
		if (!Item)
			return;
		if (!IsItemGun())
			return;
		auto Held = mex.Read<uintptr_t>(Item + 0x98); //BaseProjectile
		if (!Held)
			return;

		mex.Write<bool>(Held + 0x2A8, 1.f);
	}

	void SetSuperBow()
	{
		if (!Item)
			return;
		if (!IsItemGun())
			return;
		auto Held = mex.Read<uintptr_t>(Item + 0x98); //BaseProjectile
		if (!Held)
			return;

		mex.Write<bool>(Held + 0x330, true);
		mex.Write<float>(Held + 0x334, 1.f);
	}

	void SetNoSway()
	{
		if (!Item)
			return;
		if (!IsItemGun())
			return;
		auto Held = mex.Read<uintptr_t>(Item + 0x98); //BaseProjectile
		if (!Held)
			return;

		mex.Write<float>(Held + 0x2B8, 0.f);
	}

	void SetNoRecoil()
	{
		if (!Item)
			return;
		if (!IsItemGun())
			return;
		auto Held = mex.Read<uintptr_t>(Item + 0x98); //BaseProjectile
		if (!Held)
			return;
		mex.Write<float>(Held + 0x2D0, -1.f); //Aim Cone
		mex.Write<float>(Held + 0x2D4, -1.f); //Hip Aim Cone.

		auto RecoilProperties = mex.Read<uintptr_t>(Held + 0x2C0); //recoil
		if (!RecoilProperties)
			return;
		mex.Write<float>(RecoilProperties + 0x18, 0.f); //recoilYawMin
		mex.Write<float>(RecoilProperties + 0x1C, 0.f); //recoilYawMax 
		mex.Write<float>(RecoilProperties + 0x20, 0.f); //recoilPitchMin
		mex.Write<float>(RecoilProperties + 0x24, 0.f); //recoilPitchMax
	}

	Vector3 GetVisualPosition()
	{
		auto Transform = mex.Read<uintptr_t>(ObjectClass + 0x8);
		auto VisualState = mex.Read<uintptr_t>(Transform + 0x38);
		return mex.Read<Vector3>(VisualState + 0x90);
		//return *(Vector3*)(VisualState + 0x90);
	}
};

class List
{
public:
	uintptr_t List;
	char buffer[0x1000];

public:
	uintptr_t getValues()
	{
		return *(uintptr_t*)((uintptr_t)buffer + 0x10);
	}

	uintptr_t getValueByIndex(int idx)
	{
		auto vals = getValues();
		return *(uintptr_t*)(vals + 0x20 + (idx * 8));
	}
};

class ItemContainer
{
public:
	uintptr_t ItemContainer;
	char buffer[0x1000];

public:
	Item GetItem(int id)
	{
		Item item;
		auto item_list = mex.Read<uintptr_t>((uintptr_t)ItemContainer + 0x38);

		if (!item_list)
		{
			//std::cout << "!belt_list" << std::endl;
			return Item();
		}

		auto Items = mex.Read<uintptr_t>((uintptr_t)item_list + 0x10);

		if (!Items)
		{
			//std::cout << "!Items" << std::endl;
			return item;
		}

		auto TheItem = mex.Read<uintptr_t>(Items + 0x20 + (id * 0x8));
		if (!TheItem)
		{
			//std::cout << "!Item" << std::endl;
			return item;
		}

		item.Item = TheItem;
		mex.Read(item.Item, &item.buffer, sizeof(item.buffer));
		return item;
	}
};

class PlayerInventory
{
public:
	uintptr_t Inventory;
	char buffer[0x1000];
public:
	ItemContainer GetBelt()
	{
		ItemContainer ic;
		ic.ItemContainer = mex.Read<uintptr_t>((uintptr_t)Inventory + 0x28);
		mex.Read(ic.ItemContainer, &ic.buffer, sizeof(ic.buffer));
		return ic;
	}
};

class PlayerModel
{
public:
	uintptr_t Model;
	char buffer[0x1000];

public:
	bool IsVisible()
	{
		return mex.Read<bool>(Model + 0x238);
	}
};

class PlayerInput
{
public:
	uintptr_t Input;

public:
	Vector2 GetViewAngles()
	{
		return mex.Read<Vector2>(Input + 0x3C); //bodyAngles
	}

	Vector2 GetRecoilAngle()
	{
		return mex.Read<Vector2>(Input + 0x64); //recoilAngles
	}

	void SetViewAngles(Vector2 angle)
	{
		mex.Write<Vector2>(Input + 0x3C, angle);
	}
};

class BasePlayer
{
public:
	uintptr_t Player = NULL;
	uintptr_t ObjectClass = NULL;
	bool IsLocalPlayer = false;
	char buffer[0x1000];

public:
	bool operator==(BasePlayer p)
	{
		if (p.Player == this->Player)
			return true;
		else
			return false;
	}

public:
	std::wstring GetName()
	{
		auto nameptr = mex.Read<uintptr_t>(Player + 0x620);
		return read_unicode(nameptr + 0x14, 32);
	}
	
	float GetHealth()
	{
		return mex.Read<float>(Player + 0x1F4);
		//return //*(float*)(buffer + 0x1F4);
	}

	int GetActiveWeaponID()
	{
		return *(int*)(buffer + 0x530);
	}

	bool IsSleeping()
	{
		auto Flags = mex.Read<int>(Player + 0x5B8);
		return Flags & 16;
	}

	void SetAdminFlag()
	{
		int flags = mex.Read<int>(Player + 0x5B8);

		flags |= 4;

		mex.Write<uintptr_t>(Player + 0x5B8, flags);
	}

	void SetGravity(float grav)
	{
		auto BaseMovement = mex.Read<uintptr_t>(Player + 0x5E8);
		if (!BaseMovement)
			return;

		mex.Write<float>(BaseMovement + 0x74, grav);
	}

	void DoSpider()
	{
		auto BaseMovement = mex.Read<uintptr_t>(Player + 0x5E8);
		if (!BaseMovement)
			return;

		mex.Write<float>(BaseMovement + 0xAC, 0.f); // private float groundAngle; // 0xAC
		mex.Write<float>(BaseMovement + 0xB0, 0.f); // private float groundAngleNew; // 0xB0
	}

	void DoWaterHax()
	{
		mex.Write<float>(Player + 0x644, 0.10f);
	}

	PlayerModel GetPlayerModel()
	{
		PlayerModel model;
		model.Model = mex.Read<uintptr_t>(Player + 0x490);
		mex.Read(model.Model, &model.buffer, sizeof(model.buffer));
		return model;
	}

	PlayerInput GetPlayerInput()
	{
		PlayerInput input;
		input.Input = mex.Read<uintptr_t>(Player + 0x5E0);
		return input;
	}

	PlayerInventory GetPlayerInventory()
	{
		PlayerInventory pi;
		pi.Inventory = mex.Read<uintptr_t>((uintptr_t)Player + 0x5C8);
		mex.Read(pi.Inventory, &pi.buffer, sizeof(pi.buffer));
		return pi;
	}

	Item GetHeldItem()
	{
		Item item;
		item.Item = NULL;
		if (!Player)
			return item;

		auto active_item_id = GetActiveWeaponID();
		//std::cout << "Active Item ID: " << active_item_id << std::endl;
		if (!active_item_id)
			return item;

		int ActiveWeapon;

		for (int i = 0; i < 6; i++)
		{
			auto Inventory = GetPlayerInventory();
			auto Belt = Inventory.GetBelt();
			Item weapon = Belt.GetItem(i);
			ActiveWeapon = weapon.GetUID();
			//std::cout << "ActiveWeapon: " << ActiveWeapon << std::endl;
			if (active_item_id == ActiveWeapon)
			{
				return weapon;
			}
		}
		return item;
	}

	Vector3 GetVisualPosition()
	{
		auto Transform = *(uintptr_t*)(ObjectClass + 0x8);
		auto VisualState = *(uintptr_t*)(Transform + 0x38);
		return *(Vector3*)(VisualState + 0x90);	
	}

	Vector3 GetTransformPosition(uintptr_t transform)
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

	Vector3 GetBonePosition(int Bone)
	{
		uintptr_t player_model = mex.Read<uintptr_t>(Player + 0x118);
		uintptr_t BoneTransforms = mex.Read<uintptr_t>(player_model + 0x48);
		uintptr_t entity_bone = mex.Read<uintptr_t>(BoneTransforms + (0x20 + (Bone * 0x8)));
		return GetTransformPosition(mex.Read<uintptr_t>(entity_bone + 0x10));
	}

	Vector3 GetVelocity()
	{
		uintptr_t player_model = mex.Read<uintptr_t>(Player + 0x490);
		return mex.Read<Vector3>(player_model + 0x1D4);
	}
};

