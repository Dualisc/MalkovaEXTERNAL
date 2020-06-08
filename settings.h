#pragma once


namespace settings
{

	namespace menucolors
	{
		int colorfolder;

		namespace on
		{
			int folder;
			int r = 0;
			int g = 150;
			int b = 0;
			int a = 1;
		}

		namespace off
		{
			int folder;
			int r = 250;
			int g = 0;
			int b = 50;
			int a = 1;
		}

		namespace folder
		{
			int folder;
			int r = 0;
			int g = 200;
			int b = 250;
			int a = 1;
		}

		namespace folderopened
		{
			int folder;
			int r = 250;
			int g = 250;
			int b = 250;
			int a = 1;
		}
		
		namespace folderclosed
		{
			int folder;
			int r = 250;
			int g = 250;
			int b = 250;
			int a = 1;
		}

		namespace menutext
		{
			int folder;
			int r = 250;
			int g = 250;
			int b = 250;
			int a = 1;
		}

		namespace selected
		{
			int folder;
			int r = 250;
			int g = 0;
			int b = 250;
			int a = 1;
		}

		namespace booltrue
		{
			int folder;
			int r = 0;
			int g = 150;
			int b = 0;
			int a = 1;
		}

		namespace boolfalse
		{
			int folder;
			int r = 250;
			int g = 0;
			int b = 0;
			int a = 1;
		}

		namespace menuint
		{
			int folder;
			int r = 250;
			int g = 250;
			int b = 250;
			int a = 1;
		}
	}

	namespace player
	{
		int folder;
		int colorfolder;
		int enabled = true;
		int name = true;
		int health = true;
		int distance = true;
		int snaplines = true;
		int box = true;
		int max_distance = 300;
		int held_item = true;
		int hotbar = true;
		int showsleepers = false;
		int visible_r = 240;
		int visible_g = 130;
		int visible_b = 10;
		int visible_a = 1;

		int invisible_r = 240;
		int invisible_g = 10;
		int invisible_b = 10;
		int invisible_a = 1;

		int sleeperr = 30;
		int sleeperg = 200;
		int sleeperb = 250;
		int sleepera = 1;
	}

	namespace ore
	{
		int folder;
		int colorfolder;
		int enabled = true;
		int name = true;
		int distance = true;
		int max_distance = 100;
		int stone = true;
		int metal = true;
		int sulfur = true;

		int stone_r = 120;
		int stone_g = 120;
		int stone_b = 120;

		int metal_r = 40;
		int metal_g = 115;
		int metal_b = 0;

		float sulfur_r = 0.98;
		float sulfur_g = 0.94;
		float sulfur_b = 0;
	}

	namespace collectable
	{
		int folder;
		int colorfolder;
		int enabled = true;
		int name = true;
		int distance = true;
		int max_distance = 100;
		int r = 100;
		int g = 100;
		int b = 100;
	}

	namespace dropped
	{
		int folder;
		int colorfolder;
		int enabled = true;
		int name = true;
		int distance = true;
		int max_distance = 100;
		int r = 100;
		int g = 100;
		int b = 100;
	}

	namespace misc
	{
		int folder;
		int colorfolder;
		int spider = true;
		int crosshair = true;
		int debug_cam = true;
		int automatic = true;
		int no_recoil = false;
		int fast_reload = true;
		int fat_bullet = false;
		int always_day = true;
		int r = 100;
		int g = 100;
		int b = 100;
	}

	namespace aimbot
	{
		int folder;
		int enabled = true;
		int fov = 100;
		int max_distance = 300;
		int aim_key = 2;
		int silent_key = 1;
		int silent = true;
		int use_mouse = false;
		int smooth = 10;
		int disable_recoil = true;
	}
}