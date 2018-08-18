#include "../includes.h"
#include "../UTILS/interfaces.h"
#include "../SDK/IEngine.h"
#include "../SDK/CClientEntityList.h"
#include "../SDK/CInputSystem.h"
#include "../UTILS/render.h"
#include "../SDK/ConVar.h"
#include "Components.h"
#include "../xdxdxd.h"
#include "menu_framework.h"

int AutoCalc(int va)
{
	if (va == 1)
		return va * 35;
	else if (va == 2)
		return va * 34;
	else
		return va * 25 + 7.5;
}

struct hud_weapons_t {
	std::int32_t* get_weapon_count() {
		return reinterpret_cast<std::int32_t*>(std::uintptr_t(this) + 0x80);
	}
};
template<class T>
static T* FindHudElement(const char* name)
{
	static auto pThis = *reinterpret_cast<DWORD**>(UTILS::FindSignature("client_panorama.dll", "B9 ? ? ? ? E8 ? ? ? ? 85 C0 0F 84 ? ? ? ? 8D 58") + 1);

	static auto find_hud_element = reinterpret_cast<DWORD(__thiscall*)(void*, const char*)>(UTILS::FindSignature("client_panorama.dll", "55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39"));
	return (T*)find_hud_element(pThis, name);
}
void KnifeApplyCallbk()
{

	static auto clear_hud_weapon_icon_fn =
		reinterpret_cast<std::int32_t(__thiscall*)(void*, std::int32_t)>(
			UTILS::FindSignature("client_panorama.dll", "55 8B EC 51 53 56 8B 75 08 8B D9 57 6B FE 2C"));

	auto element = FindHudElement<std::uintptr_t*>("CCSGO_HudWeaponSelection");

	auto hud_weapons = reinterpret_cast<hud_weapons_t*>(std::uintptr_t(element) - 0x9c);
	if (hud_weapons == nullptr)
		return;

	if (!*hud_weapons->get_weapon_count())
		return;

	for (std::int32_t i = 0; i < *hud_weapons->get_weapon_count(); i++)
		i = clear_hud_weapon_icon_fn(hud_weapons, i);

	typedef void(*ForceUpdate) (void);
	ForceUpdate FullUpdate = (ForceUpdate)UTILS::FindSignaturenew("engine.dll", "FullUpdate", "A1 ? ? ? ? B9 ? ? ? ? 56 FF 50 14 8B 34 85");
	FullUpdate();
}



namespace MENU
{

	void InitColors()
	{
		using namespace PPGUI_PP_GUI;
		colors[WINDOW_BODY] = CColor(20, 20, 20, 200);
		colors[WINDOW_TITLE_BAR] = CColor(21, 21, 21, 230);
		colors[WINDOW_TEXT] = CColor(169, 45, 252);
		colors[GROUPBOX_BODY] = CColor(21, 21, 21, 200);
		colors[GROUPBOX_OUTLINE] = CColor(48, 48, 48, 200);
		colors[GROUPBOX_TEXT] = WHITE;
		colors[SCROLLBAR_BODY] = CColor(30, 30, 30, 255);
		colors[SEPARATOR_TEXT] = WHITE;
		colors[SEPARATOR_LINE] = CColor(90, 90, 90, 255);
		colors[CHECKBOX_CLICKED] = CColor(169, 45, 252);
		colors[CHECKBOX_NOT_CLICKED] = CColor(32, 32, 32, 255);
		colors[CHECKBOX_TEXT] = WHITE;
		colors[BUTTON_BODY] = CColor(23, 23, 23, 255), CColor(16, 16, 16, 255);
		colors[BUTTON_TEXT] = WHITE;
		colors[COMBOBOX_TEXT] = WHITE;
		colors[COMBOBOX_SELECTED] = CColor(23, 23, 23, 255), CColor(16, 16, 16, 255);
		colors[COMBOBOX_SELECTED_TEXT] = WHITE;
		colors[COMBOBOX_ITEM] = CColor(23, 23, 23, 255), CColor(16, 16, 16, 255);
		colors[COMBOBOX_ITEM_TEXT] = WHITE;
		colors[SLIDER_BODY] = CColor(23, 23, 23, 255);
		colors[SLIDER_VALUE] = CColor(169, 45, 252);
		colors[SLIDER_TEXT] = WHITE;
		colors[TAB_BODY] = CColor(20, 20, 20, 120);
		colors[TAB_TEXT] = LIGHTGREY;
		colors[TAB_BODY_SELECTED] = CColor(169, 45, 252);
		colors[TAB_TEXT_SELECTED] = WHITE;
		colors[VERTICAL_TAB_BODY] = CColor(255, 40, 40, 255);
		colors[VERTICAL_TAB_TEXT] = WHITE;
		colors[VERTICAL_TAB_OUTLINE] = CColor(0, 0, 0, 100);
		colors[VERTICAL_TAB_BODY_SELECTED] = CColor(255, 20, 20, 255);
		colors[COLOR_PICKER_BODY] = CColor(23, 23, 23, 255);
		colors[COLOR_PICKER_TEXT] = WHITE;
		colors[COLOR_PICKER_OUTLINE] = CColor(WHITE);
	}

	void Do()
	{
		

		if (UTILS::INPUT::input_handler.GetKeyState(VK_INSERT) & 1)
		{
			menu_open = !menu_open;
			INTERFACES::InputSystem->EnableInput(!menu_open);
		}

		if (UTILS::INPUT::input_handler.GetKeyState(UTILS::INPUT::input_handler.keyBindings(SETTINGS::settings.flip_int)) & 1)
			SETTINGS::settings.flip_bool = !SETTINGS::settings.flip_bool;

		if (UTILS::INPUT::input_handler.GetKeyState(UTILS::INPUT::input_handler.keyBindings(SETTINGS::settings.quickstopkey)) & 1)
			SETTINGS::settings.stop_flip = !SETTINGS::settings.stop_flip;

		if (UTILS::INPUT::input_handler.GetKeyState(UTILS::INPUT::input_handler.keyBindings(SETTINGS::settings.overridekey)) & 1)
			SETTINGS::settings.overridething = !SETTINGS::settings.overridething;

		InitColors();
		if (menu_open)
		{
			using namespace PPGUI_PP_GUI;
			if (!menu_open) return;

			DrawMouse();

			SetFont(FONTS::menu_window_font);
			WindowBegin("cathak ", Vector2D(270, 270), Vector2D(700, 500));

			std::vector<std::string> tabs = { "Aimbot", "Visuals", "Miscellaneous", "AntiAim", "Colours", "Configuration" };
			std::vector<std::string> aim_mode = { "rage", "legit" };
			std::vector<std::string> acc_mode = { "Head", "Body Aim", "Hitscan" };
			std::vector<std::string> chams_mode = { "None", "Visible", "Invisible" };
			std::vector<std::string> aa_pitch = { "None", "Emotion", "Fake Down", "Fake Up", "Fake Zero" };
			std::vector<std::string> aa_mode = { "None", "backwards", "Sideways", "BackJitter", "Lowerbody", "Legit Troll", "Rotational", "Freestanding" };
			std::vector<std::string> aa_fake = { "None", "BackJitter", "Random", "Local View", "Opposite", "Rotational" };
			std::vector<std::string> configs = { "Default", "Legit", "Autos", "Scouts", "Pistols", "Awps", "Nospread" };
			std::vector<std::string> box_style = { "None", "Full", "Debug" };
			std::vector<std::string> media_style = { "Perfect", "Random" };
			std::vector<std::string> delay_shot = { "Off", "lag Compensation" };
			std::vector<std::string> fakelag_mode = { "Factor", "Adaptive" };
			std::vector<std::string> key_binds = { "None", "Mouse1", "Mouse2", "Mouse3", "Mouse4", "Mouse5", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12" };
			std::vector<std::string> hitmarker = { "None", "GameSense", "Bameware", "Bubble","rifk7beep","aimware","gay mario","metalnigger","button","oofnigger","tinybutton", "Custom" };
			std::vector<std::string> antiaimmode = { "Standing", "Moving", "Jumping" };
			std::vector<std::string> glow_styles = { "Regular", "Pulsing", "Outline" };
			std::vector<std::string> local_chams = { "None","Sim Fakelag: Normal", "Non Sim Fakelag", "Sim Fakelag: Color" };
			std::vector<std::string> chams_type = { "Metallic", "Basic" };
			std::vector<std::string> team_select = { "Enemy", "Team" };
			std::vector<std::string> crosshair_select = { "None", "Static", "Recoil" };
			std::vector<std::string> autostop_method = { "Head", "Hitscan" };
			std::vector<std::string> override_method = { "Set", "Key Press" };
			std::vector<std::string> newbuybot_options = { "None", "Auto", "Scout", "Awp" };
			std::vector<std::string> weptype = { "Weapon", "Weapon + Text Ammo", "Weapon Icon" };
			std::vector<std::string> ResolverOptions = { "None", "cathak",  };


		
			std::string config;

			std::vector<std::string> KnifeModel = { "Bayonet",
				"Flip Knife",
				"Gut Knife",
				"Karambit",
				"M9 Bayonet",
				"Huntsman Knife",
				"Butterfly Knife",
				"Falchion Knife",
				"Shadow Daggers",
				"Bowie Knife",
				"Navaja Knife",
				"Stiletto Knife",
				"Ursus Knife",
				"Talon Knife" };
			std::vector<std::string> knifeskins = { "None",
				"Crimson Web",
				"Bone Mask",
				"Fade",
				"Night",
				"Blue Steel",
				"Stained",
				"Case Hardened",
				"Slaughter",
				"Safari Mesh",
				"Boreal Forest",
				"Ultraviolet",
				"Urban Masked",
				"Scorched",
				"Rust Coat",
				"Tiger Tooth",
				"Damascus Steel",
				"Damascus Steel",
				"Marble Fade",
				"Rust Coat",
				"Doppler Ruby",
				"Doppler Sapphire",
				"Doppler Blackpearl",
				"Doppler Phase 1",
				"Doppler Phase 2",
				"Doppler Phase 3",
				"Doppler Phase 4",
				"Gamma Doppler Phase 1",
				"Gamma Doppler Phase 2",
				"Gamma Doppler Phase 3",
				"Gamma Doppler Phase 4",
				"Gamma Doppler Emerald",
				"Lore",
				"Black Laminate",
				"Autotronic",
				"Freehand" };
			std::vector<std::string> ak47 = { "None",
				"Fire Serpent",
				"Fuel Injector",
				"Bloodsport",
				"Vulcan",
				"Case Hardened",
				"Hydroponic",
				"Aquamarine Revenge",
				"Frontside Misty",
				"Point Disarray",
				"Neon Revolution",
				"Red Laminate",
				"Redline",
				"Jaguar",
				"Jet Set",
				"Wasteland Rebel",
				"The Empress",
				"Elite Build",
				"Neon Rider" };
			std::vector<std::string> GloveModel = { "None",
				"Bloodhound",
				"Sport",
				"Driver",
				"Wraps",
				"Moto",
				"Specialist" };
			std::vector<std::string> xdshit = { "kit1",
				"kit2",
				"kit3",
				"kit4" };

			
			switch (Tab("main", tabs, OBJECT_FLAGS::FLAG_NONE))
			{
				case 0: {
				Checkbox("Turn Aimbot On", SETTINGS::settings.aim_bool);
				if (SETTINGS::settings.aim_bool)
				{
					GroupboxBegin("Main", AutoCalc(1));
					Combobox("Aimbot Mode", acc_mode, SETTINGS::settings.acc_type);
					GroupboxEnd();

					if (SETTINGS::settings.aim_type == 0)
					{
						GroupboxBegin("Damage Control", AutoCalc(5));
						Slider("Minimum Hit Chance", 0, 100, SETTINGS::settings.chance_val);
						Slider("Minimum Damage", 1, 100, SETTINGS::settings.damage_val);
						Slider("BAIM after x shots", 0, 20, SETTINGS::settings.baimaftershot);
						Slider("BAIM under HP", 0, 100, SETTINGS::settings.baimafterhp);
						Checkbox("BAIM when in air", SETTINGS::settings.baiminair);
						GroupboxEnd();

						GroupboxBegin("Multipoint", AutoCalc(3));
						Checkbox("Turn Multipoint On", SETTINGS::settings.multi_bool);
						Slider("Head Scale", 0, 1, SETTINGS::settings.point_val);
						Slider("Body Scale", 0, 1, SETTINGS::settings.body_val);
						GroupboxEnd();

						GroupboxBegin("Addons", AutoCalc(7));
						Combobox("Resolver", ResolverOptions, SETTINGS::settings.ResolverEnable);
						Checkbox("quick stop", SETTINGS::settings.stop_bool);
						Checkbox("NoSpread", SETTINGS::settings.nospread);
						Checkbox("Zeus Bot", SETTINGS::settings.autozeus);
						Checkbox("Auto revolver", SETTINGS::settings.auto_revolver);
						Checkbox("Enable override", SETTINGS::settings.overrideenable);
						Combobox("Override Key", key_binds, SETTINGS::settings.overridekey);
						Combobox("Override Method", override_method, SETTINGS::settings.overridemethod);
						Combobox("Fake Lag Fix", delay_shot, SETTINGS::settings.delay_shot);
						GroupboxEnd();
					}
					else
					{
						GroupboxBegin("In Progress", AutoCalc(3));

						GroupboxEnd();
					}
				}
			} break;
			    case 1: {
				Checkbox("Enable Visuals", SETTINGS::settings.esp_bool);
				if (SETTINGS::settings.esp_bool)
				{
					GroupboxBegin("Players", AutoCalc(10));
					Combobox("Select Team", team_select, SETTINGS::settings.espteamselection);
					if (SETTINGS::settings.espteamselection == 0)
					{
						Checkbox("Draw Enemy Box", SETTINGS::settings.box_bool);
						Checkbox("Draw Enemy Name", SETTINGS::settings.name_bool);
						Checkbox("Draw Enemy Weapon Icons", SETTINGS::settings.WeaponIconsOn);
						Checkbox("Draw Enemy Weapon", SETTINGS::settings.weap_bool);
						if (SETTINGS::settings.weap_bool)
						{
							Combobox("Type", weptype, SETTINGS::settings.draw_wep);
						}
						Checkbox("Draw Enemy Flags", SETTINGS::settings.info_bool);
						Checkbox("Draw Enemy Health", SETTINGS::settings.health_bool);
						Checkbox("Draw Enemy Money", SETTINGS::settings.money_bool);
						Checkbox("Draw Ammo Bar - Crashing Sometimes", SETTINGS::settings.ammo_bool);
						Checkbox("Draw Enemy Fov Arrows", SETTINGS::settings.fov_bool);
						Checkbox("Gravitational Ragdoll", SETTINGS::settings.ragdoll_bool);
					}
					else if (SETTINGS::settings.espteamselection == 1)
					{
						Checkbox("Draw Team Box", SETTINGS::settings.boxteam);
						Checkbox("Draw Team Name", SETTINGS::settings.nameteam);
						Checkbox("Draw Team Weapon", SETTINGS::settings.weaponteam);
						Checkbox("Draw Team Flags", SETTINGS::settings.flagsteam);
						Checkbox("Draw Team Health", SETTINGS::settings.healthteam);
						Checkbox("Draw Team Money", SETTINGS::settings.moneyteam);
						Checkbox("Draw Ammo Bar - crashing sometimes", SETTINGS::settings.ammoteam);
						Checkbox("Draw Team Fov Arrows", SETTINGS::settings.arrowteam);
					}
					GroupboxEnd();

					GroupboxBegin("Chams", AutoCalc(5));
					Checkbox("Fake Chams", SETTINGS::settings.draw_fake);
					if (SETTINGS::settings.draw_fake)
						ColorPicker("Fake", SETTINGS::settings.fake_darw_col);
					Combobox("Model Team Selection", team_select, SETTINGS::settings.chamsteamselection);
					if (SETTINGS::settings.chamsteamselection == 0)
						Combobox("Enemy Coloured Models", chams_mode, SETTINGS::settings.chams_type);
					else if (SETTINGS::settings.chamsteamselection == 1)
						Combobox("Team Coloured Models", chams_mode, SETTINGS::settings.chamsteam);
					Combobox("Model Type", chams_type, SETTINGS::settings.chamstype);
					GroupboxEnd();

					GroupboxBegin("Glow", AutoCalc(5));
					Combobox("Glow Team Selection", team_select, SETTINGS::settings.glowteamselection);
					if (SETTINGS::settings.glowteamselection == 0)
						Checkbox("Enemy Glow Enable", SETTINGS::settings.glowenable);
					else if (SETTINGS::settings.glowteamselection == 1)
						Checkbox("Team Glow Enable", SETTINGS::settings.glowteam);
					Combobox("Glow Style", glow_styles, SETTINGS::settings.glowstyle);
					Checkbox("Local Glow", SETTINGS::settings.glowlocal);
					Combobox("Local Glow Style", glow_styles, SETTINGS::settings.glowstylelocal);

					GroupboxEnd();

					GroupboxBegin("World", AutoCalc(16));
					//Slider("Night Value", 0, 100, SETTINGS::settings.daytimevalue);
					/*Checkbox("Sky Color Changer", SETTINGS::settings.sky_bool);
					if (SETTINGS::settings.sky_bool)
					{
						ColorPicker("Sky Color", SETTINGS::settings.sky_color);
					}*/
					Checkbox("Night mode", SETTINGS::settings.night_mode);
					Checkbox("Sky color changer", SETTINGS::settings.sky_enabled); //fixed by sleevy
					ColorPicker("Sky color", SETTINGS::settings.skycolor);
					Checkbox("Asus Props", SETTINGS::settings.asus_bool);
					Checkbox("Bullet Tracers", SETTINGS::settings.beam_bool);
					Checkbox("Bullet Impacts", SETTINGS::settings.impacts);
					Checkbox("Force crosshair", SETTINGS::settings.forcehair);
					Checkbox("Anti Flash", SETTINGS::settings.no_flash_enabled);
					//Checkbox("Radar", SETTINGS::settings.radar_enabled);
					Checkbox("Thirdperson", SETTINGS::settings.tp_bool);
					Combobox("Thirdperson Key", key_binds, SETTINGS::settings.thirdperson_int);
					Combobox("Crosshair", crosshair_select, SETTINGS::settings.xhair_type);
					Checkbox("Damage Indicator", SETTINGS::settings.dmg_bool);
					if (SETTINGS::settings.dmg_bool)
					{
						ColorPicker("Indicator Color", SETTINGS::settings.dmg_color);
					}
					Checkbox("Draw Hitboxes", SETTINGS::settings.lagcomhit);
					if (SETTINGS::settings.lagcomhit)
					{
						Slider("Duration", 0, 10, SETTINGS::settings.lagcomptime);
					}
					Checkbox("No Smoke", SETTINGS::settings.smoke_bool);
					Checkbox("Wireframe Hands", SETTINGS::settings.wirehand_bool);
					Checkbox("Remove Visual Recoil", SETTINGS::settings.novisualrecoil_bool);
					Checkbox("No Scope", SETTINGS::settings.scope_bool);
					Checkbox("Remove Scope", SETTINGS::settings.removescoping);
					Checkbox("Fix Zoom Sensitivity", SETTINGS::settings.fixscopesens);
					if (!SETTINGS::settings.matpostprocessenable)
						Checkbox("Enable Postprocessing", SETTINGS::settings.matpostprocessenable);
					else
						Checkbox("Disable Postprocessing", SETTINGS::settings.matpostprocessenable);
					GroupboxEnd();

					GroupboxBegin("Local Player", AutoCalc(6));
					Combobox("Local Chams", local_chams, SETTINGS::settings.localchams);
					Slider("Render Fov", 90, 179, SETTINGS::settings.fov_val, 0);
					Slider("Viewmodel Fov", 90, 179, SETTINGS::settings.viewfov_val, 0);
					Combobox("Hitmarker Sound", hitmarker, SETTINGS::settings.hitmarker_val);
					Checkbox("Aimware Hitmarkers", SETTINGS::settings.hitmarker);
					ColorPicker("AW Hitmarkers color", SETTINGS::settings.awcolor);
					Checkbox("LBY Indicator", SETTINGS::settings.lbyenable);
					GroupboxEnd();
				}
			} break;
			    case 2: {
				Checkbox("Enable Misc", SETTINGS::settings.misc_bool);
				if (SETTINGS::settings.misc_bool)
				{

					Checkbox("skinchanger", SETTINGS::settings.skinenabled);
					Checkbox("glovechanger", SETTINGS::settings.glovesenabled);

				
					Combobox(("Knife Model"), KnifeModel , SETTINGS::settings.Knife);
	
					Combobox(("Knife Skin"), knifeskins , SETTINGS::settings.KnifeSkin);
					Combobox(("glove model"), GloveModel, SETTINGS::settings.gloves);
					Combobox(("glove skin"), xdshit , SETTINGS::settings.skingloves);
					Combobox(("AK-47"), ak47 , SETTINGS::settings.AK47Skin);

					short cw = UTILS::SafeWeaponID();
					if (cw == 59 || cw == 500 || cw == 42 || cw == 507 || cw == 506 || cw == 508 || cw == 509 || cw == 515 || cw == 516 || cw == 505 || cw == 512 || cw == 523 || cw == 519 || cw == 521 || cw == 520)
					{

					}
					else
					{
						
						Combobox(("Stikers1"), ak47, SETTINGS::settings.weapons[cw].Stikers1);
						Combobox(("Stikers2"), ak47, SETTINGS::settings.weapons[cw].Stikers2);
						Combobox(("Stikers3"), ak47, SETTINGS::settings.weapons[cw].Stikers3);
						Combobox(("Stikers4"), ak47, SETTINGS::settings.weapons[cw].Stikers4);
						
					}

					if (Button(("Force update")))
						KnifeApplyCallbk();

					GroupboxBegin("Movement", AutoCalc(6));
					Checkbox("Auto Bunnyhop", SETTINGS::settings.bhop_bool);
					Checkbox("Auto Strafer", SETTINGS::settings.strafe_bool);
					Checkbox("Auto Duck In Air", SETTINGS::settings.duck_bool);
					Combobox("Circle Strafe", key_binds, SETTINGS::settings.circlestrafekey);
					Checkbox("FakeWalk", SETTINGS::settings.fakewalk);
					Checkbox("Meme WaLk", SETTINGS::settings.astro);
					GroupboxEnd();

					GroupboxBegin("Fakelag", AutoCalc(5));
					Checkbox("Enable", SETTINGS::settings.lag_bool);
					Combobox("Fakelag Type", fakelag_mode, SETTINGS::settings.lag_type);
					Slider("Standing Lag", 1, 14, SETTINGS::settings.stand_lag);
					Slider("Moving Lag", 1, 14, SETTINGS::settings.move_lag);
					Slider("Jumping Lag", 1, 14, SETTINGS::settings.jump_lag);
					GroupboxEnd();

					GroupboxBegin("Clantag Changer", AutoCalc(1));
					//Checkbox("StackGlock Clantag", SETTINGS::settings.misc_clantag);
					Checkbox("Gamesense Clantag", SETTINGS::settings.GameSenseClan);
					GroupboxEnd();

					GroupboxBegin("Buy Bot", AutoCalc(7));
					Checkbox("Enable", SETTINGS::settings.autobuy_bool);
					Checkbox("Auto", SETTINGS::settings.auto_bool);
					if (SETTINGS::settings.auto_bool)
						SETTINGS::settings.scout_bool = false;
					Checkbox("Scout", SETTINGS::settings.scout_bool);
					if (SETTINGS::settings.scout_bool)
						SETTINGS::settings.auto_bool = false;
					Checkbox("Revolver / Deagle", SETTINGS::settings.revolver_bool);
					if (SETTINGS::settings.revolver_bool)
						SETTINGS::settings.elite_bool = false;
					Checkbox("Elites", SETTINGS::settings.elite_bool);
					if (SETTINGS::settings.elite_bool)
						SETTINGS::settings.revolver_bool = false;
					Checkbox("Zeus", SETTINGS::settings.zeus_bool);
					Checkbox("Armour + Defuse", SETTINGS::settings.armour_bool);
					GroupboxEnd();



					
					

				}
			} break;
			    case 3: {
				Checkbox("Enable AntiAim", SETTINGS::settings.aa_bool);
				if (SETTINGS::settings.aa_bool)
				{
					if (SETTINGS::settings.aa_bool)
					{
						GroupboxBegin("AntiAim", AutoCalc(4));

						Combobox("AntiAim Mode", antiaimmode, SETTINGS::settings.aa_mode);
						switch (SETTINGS::settings.aa_mode)
						{
						case 0:
							Combobox("AntiAim Pitch - Standing", aa_pitch, SETTINGS::settings.aa_pitch_type);
							Combobox("AntiAim Real - Standing", aa_mode, SETTINGS::settings.aa_real_type);
							Combobox("AntiAim Fake - Standing", aa_fake, SETTINGS::settings.aa_fake_type);
							break;
						case 1:
							Combobox("AntiAim Pitch - Moving", aa_pitch, SETTINGS::settings.aa_pitch1_type);
							Combobox("AntiAim Real - Moving", aa_mode, SETTINGS::settings.aa_real1_type);
							Combobox("AntiAim Fake - Moving", aa_fake, SETTINGS::settings.aa_fake1_type);
							break;
						case 2:
							Combobox("AntiAim Pitch - Jumping", aa_pitch, SETTINGS::settings.aa_pitch2_type);
							Combobox("AntiAim Real - Jumping", aa_mode, SETTINGS::settings.aa_real2_type);
							Combobox("AntiAim Fake - Jumping", aa_fake, SETTINGS::settings.aa_fake2_type);
							break;
						}

						GroupboxEnd();

						GroupboxBegin("Additional Options", AutoCalc(7));

						Checkbox("Beta AA", SETTINGS::settings.Beta_AA);
						Combobox("Flip Key", key_binds, SETTINGS::settings.flip_int);
						Checkbox("Anti Aim Arrows", SETTINGS::settings.rifk_arrow);
						switch (SETTINGS::settings.aa_mode)
						{
						case 0:
							Slider("Real Additive", -180, 180, SETTINGS::settings.aa_realadditive_val);
							Slider("Fake Additive", -180, 180, SETTINGS::settings.aa_fakeadditive_val);
							Slider("LowerBodyYaw Delta", -119.9, 119.9, SETTINGS::settings.delta_val);
							Checkbox("LBY Flick Up", SETTINGS::settings.lbyflickup);
							break;
						case 1:
							Slider("Real Additive ", -180, 180, SETTINGS::settings.aa_realadditive1_val);
							Slider("Fake Additive", -180, 180, SETTINGS::settings.aa_fakeadditive1_val);
							Slider("LowerBodyYaw Delta", -119.9, 119.9, SETTINGS::settings.delta1_val);
							Checkbox("LBY Flick Up", SETTINGS::settings.lbyflickup1);
							break;
						case 2:
							Slider("Real Additive", -180, 180, SETTINGS::settings.aa_realadditive2_val);
							Slider("Fake Additive", -180, 180, SETTINGS::settings.aa_fakeadditive2_val);
							Slider("LowerBodyYaw Delta", -119.9, 119.9, SETTINGS::settings.delta2_val);
							Checkbox("LBY Flick Up", SETTINGS::settings.lbyflickup2);
							break;
						}

						GroupboxEnd();

						GroupboxBegin("Rotate", AutoCalc(4));

						Slider("Rotate Fake °", 0, 180, SETTINGS::settings.spinanglefake);
						Slider("Rotate Fake %", 0, 100, SETTINGS::settings.spinspeedfake);

						switch (SETTINGS::settings.aa_mode)
						{
						case 0:
							Slider("Rotate Standing °", 0, 180, SETTINGS::settings.spinangle);
							Slider("Rotate Standing %", 0, 100, SETTINGS::settings.spinspeed);
							break;
						case 1:
							Slider("Rotate Moving °", 0, 180, SETTINGS::settings.spinangle1);
							Slider("Rotate Moving %", 0, 100, SETTINGS::settings.spinspeed1);
							break;
						case 2:
							Slider("Rotate Jumping °", 0, 180, SETTINGS::settings.spinangle2);
							Slider("Rotate Jumping %", 0, 100, SETTINGS::settings.spinspeed2);
							break;
						}

						GroupboxEnd();
					}
				}
			} break;
			    case 4: {
				GroupboxBegin("ESP Colours", AutoCalc(6));
				Combobox("ESP Colour Selection", team_select, SETTINGS::settings.espteamcolourselection);
				if (SETTINGS::settings.espteamcolourselection == 0)
				{
					ColorPicker("Enemy Box Colour", SETTINGS::settings.box_col);
					ColorPicker("Enemy Name Colour", SETTINGS::settings.name_col);
					ColorPicker("Enemy Weapon Colour", SETTINGS::settings.weapon_col);
					ColorPicker("Enemy Fov Arrows Colour", SETTINGS::settings.fov_col);
				}
				else if (SETTINGS::settings.espteamcolourselection == 1)
				{
					ColorPicker("Team Box Colour", SETTINGS::settings.boxteam_col);
					ColorPicker("Team Name Colour", SETTINGS::settings.nameteam_col);
					ColorPicker("Team Weapon Colour", SETTINGS::settings.weaponteam_col);
					ColorPicker("Team Fov Arrows Colour", SETTINGS::settings.arrowteam_col);
				}
				ColorPicker("Grenade Prediction Colour", SETTINGS::settings.grenadepredline_col);
				GroupboxEnd();

				GroupboxBegin("Chams Colours", AutoCalc(5));

				ColorPicker("Enemy Visible Colour", SETTINGS::settings.vmodel_col);
				ColorPicker("Enemy Invisible Colour", SETTINGS::settings.imodel_col);

				ColorPicker("Team Visible Colour", SETTINGS::settings.teamvis_color);
				ColorPicker("Team Invisible Colour", SETTINGS::settings.teaminvis_color);

				ColorPicker("Local Colour", SETTINGS::settings.localchams_col);

				GroupboxEnd();

				GroupboxBegin("Glow Colours", AutoCalc(3));

				ColorPicker("Glow Enemy Colour", SETTINGS::settings.glow_col);
				ColorPicker("Glow Team Colour", SETTINGS::settings.teamglow_color);
				ColorPicker("Glow Local Colour", SETTINGS::settings.glowlocal_col);

				GroupboxEnd();

				GroupboxBegin("Bullet Tracer Colours", AutoCalc(3));

				ColorPicker("Local Player", SETTINGS::settings.bulletlocal_col);
				ColorPicker("Enemy Player", SETTINGS::settings.bulletenemy_col);
				ColorPicker("Team Player", SETTINGS::settings.bulletteam_col);

				GroupboxEnd();

			} break;
			    case 5: {
				GroupboxBegin("Configuration", 90);
				switch (Combobox("Config", configs, SETTINGS::settings.config_sel))
				{
				case 0: config = "Default"; break;
				case 1: config = "Legit"; break;
				case 2: config = "Auto"; break;
				case 3: config = "Scout"; break;
				case 4: config = "Pistol"; break;
				case 5: config = "Awp"; break;
				case 6: config = "Nospread"; break;
				}

				if (Button("Load Config"))
				{
					INTERFACES::cvar->ConsoleColorPrintf(CColor(255, 0, 100), "[Config] ");
					GLOBAL::Msg("Configuration Loaded.     \n");

					SETTINGS::settings.Load(config);
				}
				if (Button("Save Config"))
				{
					INTERFACES::cvar->ConsoleColorPrintf(CColor(255, 0, 100), "[Config] ");
					GLOBAL::Msg("Configuration Saved.     \n");

					SETTINGS::settings.Save(config);
				}
				GroupboxEnd();
			} break;
			}
			WindowEnd();
		}
	}
}