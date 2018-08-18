#include "..\includes.h"
#include "../xdxdxd.h"
#include "../AWHitmarkers.h"
#include "hooks.h"
#include "../UTILS/interfaces.h"
#include "../UTILS/offsets.h"
#include "../NetVar.h"
#include "../UTILS/render.h"
#include "../Skinchanger.h"
#include "../Glovechanger.h"
#include "../SDK/CInput.h"
#include "../SDK/IClient.h"
#include "../SDK/CPanel.h"
#include "../SDK/ConVar.h"
#include "../SDK/CGlowObjectManager.h"
#include "../SDK/IEngine.h"
#include "../SDK/CTrace.h"
#include "../SDK/CClientEntityList.h"
#include "../SDK/RecvData.h"
#include "../SDK/CBaseAnimState.h"
#include "../SDK/ModelInfo.h"
#include "../SDK/ModelRender.h"
#include "../SDK/RenderView.h"
#include "../SDK/CTrace.h"
#include "../SDK/CViewSetup.h"
#include "../SDK/CGlobalVars.h"
#include "../SDK/CPrediction.h"

#include "../FEATURES/Movement.h"
#include "../FEATURES/Visuals.h"
#include "../FEATURES/Chams.h"
#include "../FEATURES/AntiAim.h"
#include "../FEATURES/Aimbot.h"
#include "../FEATURES/Resolver.h"
#include "../FEATURES/Backtracking.h"
#include "../FEATURES/FakeWalk.h"
#include "../FEATURES/FakeLag.h"
#include "../FEATURES/EnginePred.h"
#include "../FEATURES/EventListener.h"
#include "../FEATURES/GrenadePrediction.h"
#include "../FEATURES/Legitbot.h"
#include "../FEATURES/Flashlight.h"

#include "../MENU/menu_framework.h"

#include <intrin.h>

static bool tick = false;
static int ground_tick;
Vector vecAimPunch, vecViewPunch;
Vector* pAimPunch = nullptr;
Vector* pViewPunch = nullptr;

static auto CAM_THINK = UTILS::FindSignature("client_panorama.dll", "85 C0 75 30 38 86");
static auto linegoesthrusmoke = UTILS::FindPattern("client_panorama.dll", (PBYTE)"\x55\x8B\xEC\x83\xEC\x08\x8B\x15\x00\x00\x00\x00\x0F\x57\xC0", "xxxxxxxx????xxx");

void ground_ticks()
{
	auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());

	if (!local_player)
		return;

	if (local_player->GetFlags() & FL_ONGROUND)
		ground_tick++;
	else
		ground_tick = 0;
}

namespace HOOKS
{
	CreateMoveFn original_create_move;
	PaintTraverseFn original_paint_traverse;
	PaintFn original_paint;
	FrameStageNotifyFn original_frame_stage_notify;
	DrawModelExecuteFn original_draw_model_execute;
	SceneEndFn original_scene_end;
	TraceRayFn original_trace_ray;
	OverrideViewFn original_override_view;
	RenderViewFn original_render_view;
	SvCheatsGetBoolFn original_get_bool;
	GetViewmodelFOVFn original_viewmodel_fov;

	vfunc_hook client;
	vfunc_hook panel;
	vfunc_hook paint;
	vfunc_hook modelrender;
	vfunc_hook sceneend;
	vfunc_hook renderview;
	vfunc_hook trace;
	vfunc_hook netchannel;
	vfunc_hook overrideview;
	vfunc_hook input;
	vfunc_hook getbool;

	CSX::Hook::VTable SurfaceTable;



	void AutoRevolver(SDK::CUserCmd* cmd) {
		auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());
		auto weapon = reinterpret_cast< SDK::CBaseWeapon* >(INTERFACES::ClientEntityList->GetClientEntity(local_player->GetActiveWeaponIndex()));

		if (!weapon)
			return;

		if (local_player->GetHealth() <= 0)
			return;

		if (!weapon->GetItemDefenitionIndex())
			return;

		if (weapon->GetItemDefenitionIndex() == SDK::ItemDefinitionIndex::WEAPON_REVOLVER) {
			static int delay = 0;
			delay++;
			if (delay <= 15)
				cmd->buttons |= IN_ATTACK;
			else
				delay = 0;
		}
	}

	bool __stdcall HookedCreateMove(float sample_input_frametime, SDK::CUserCmd* cmd)
	{ 
		static auto ofunc = overrideview.get_original<CreateMoveFn>(24);
		ofunc(sample_input_frametime, cmd);
		if (!cmd || cmd->command_number == 0)
			return false;

		uintptr_t* FPointer; __asm { MOV FPointer, EBP }
		byte* SendPacket = (byte*)(*FPointer - 0x1C);
		if (!SendPacket) return false;

		auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());
		if (!local_player) return false;

		GLOBAL::should_send_packet = *SendPacket;
		GLOBAL::originalCMD = *cmd;
		if (INTERFACES::Engine->IsConnected() && INTERFACES::Engine->IsInGame())
		{
			GrenadePrediction::instance().Tick(cmd->buttons);
			if (SETTINGS::settings.aim_type == 0)
				slidebitch->do_fakewalk(cmd);

			if (SETTINGS::settings.fakewalk)
			{
				if (GetAsyncKeyState(VK_SHIFT))
				{
					static int choked = 0;
					choked = choked > 7 ? 0 : choked + 1;
					GLOBAL::originalCMD.move.x = choked < 2 || choked > 5 ? 0 : GLOBAL::originalCMD.move.x;
					GLOBAL::originalCMD.move.y = choked < 2 || choked > 5 ? 0 : GLOBAL::originalCMD.move.y;
					GLOBAL::should_send_packet = choked < 1;
				}
			}

			if (SETTINGS::settings.novis_bool)
			{
				pAimPunch = (Vector*)((DWORD)local_player + OFFSETS::m_aimPunchAngle);
				pViewPunch = (Vector*)((DWORD)local_player + OFFSETS::m_viewPunchAngle);

				vecAimPunch = *pAimPunch;
				vecViewPunch = *pViewPunch;

				*pAimPunch = Vector(0, 0, 0);
				*pViewPunch = Vector(0, 0, 0);
			}

			for (int i = 1; i <= INTERFACES::Globals->maxclients; i++)
			{
				auto entity = INTERFACES::ClientEntityList->GetClientEntity(i);

				if (!entity)
					continue;

				if (entity == local_player)
					continue;

				*(int*)((uintptr_t)entity + 0xA30) = INTERFACES::Globals->framecount;
				*(int*)((uintptr_t)entity + 0xA28) = 0;
			}

			if (!GetAsyncKeyState(0x56))
				fakelag->do_fakelag(cmd);

			if (SETTINGS::settings.astro)
			{
				if (GLOBAL::originalCMD.move.x > 0)
				{
					cmd->buttons |= IN_BACK;
					cmd->buttons &= ~IN_FORWARD;
				}

				if (GLOBAL::originalCMD.move.x < 0)
				{
					cmd->buttons |= IN_FORWARD;
					cmd->buttons &= ~IN_BACK;
				}

				if (GLOBAL::originalCMD.move.y < 0)
				{
					cmd->buttons |= IN_MOVERIGHT;
					cmd->buttons &= ~IN_MOVELEFT;
				}

				if (GLOBAL::originalCMD.move.y > 0)
				{
					cmd->buttons |= IN_MOVELEFT;
					cmd->buttons &= ~IN_MOVERIGHT;
				}
			}

			static SDK::ConVar* ragdoll = INTERFACES::cvar->FindVar("cl_ragdoll_gravity");
			if (SETTINGS::settings.ragdoll_bool)
			{
				ragdoll->SetValue(400);
			}
			else {
				ragdoll->SetValue(800);
			}

			static SDK::ConVar* impacts = INTERFACES::cvar->FindVar("sv_showimpacts");
			if (SETTINGS::settings.impacts)
			{
				impacts->SetValue(1);
			}
			else {
				impacts->SetValue(0);
			}


			if (SETTINGS::settings.bhop_bool) movement->bunnyhop(cmd);
			if (SETTINGS::settings.duck_bool) movement->duckinair(cmd);

			//if (SETTINGS::settings.misc_clantag) visuals->Clantag();
			if (SETTINGS::settings.GameSenseClan) visuals->GameSense();
			if (SETTINGS::settings.Beta_AA) antiaim->BetaAA(cmd);
			if (SETTINGS::settings.BacktrackChams) visuals->DrawBacktrack();
			if (SETTINGS::settings.autozeus) aimbot->autozeus(cmd);
			prediction->run_prediction(cmd);
			{
				if (SETTINGS::settings.auto_revolver) AutoRevolver(cmd);

				if (SETTINGS::settings.strafe_bool) movement->autostrafer(cmd);

				if (SETTINGS::settings.aim_type == 0 && SETTINGS::settings.aim_bool)
				{
					aimbot->run_aimbot(cmd);
					backtracking->run_legit(cmd);
				}

				if (SETTINGS::settings.aim_type == 1 && SETTINGS::settings.aim_bool)
				{
					if (GetAsyncKeyState(UTILS::INPUT::input_handler.keyBindings(SETTINGS::settings.legittrigger_key)) && SETTINGS::settings.legittrigger_bool)
						legitbot->triggerbot(cmd);
					backtracking->run_legit(cmd);
				}

				if (SETTINGS::settings.aa_bool)
				{
					antiaim->do_antiaim(cmd);
					antiaim->fix_movement(cmd);
					//ground_ticks();
				}
			}
			prediction->end_prediction(cmd);

			if (!GLOBAL::should_send_packet)
				GLOBAL::real_angles = cmd->viewangles;
			else
			{
				GLOBAL::FakePosition = local_player->GetAbsOrigin();
				GLOBAL::fake_angles = cmd->viewangles;
			}

		}
		*SendPacket = GLOBAL::should_send_packet;
		cmd->move = antiaim->fix_movement(cmd, GLOBAL::originalCMD);
		if (SETTINGS::settings.aa_pitch < 2 || SETTINGS::settings.aa_pitch1_type < 2 || SETTINGS::settings.aa_pitch2_type < 2)
			UTILS::ClampLemon(cmd->viewangles);
		return false;
	}

	


	void __stdcall HookedPaintTraverse(int VGUIPanel, bool ForceRepaint, bool AllowForce)
	{
		static auto ofunc = panel.get_original<PaintTraverseFn>(41);
		std::string panel_name = INTERFACES::Panel->GetName(VGUIPanel);
		if (panel_name == "HudZoom" && SETTINGS::settings.scope_bool) return;
		if (panel_name == "FocusOverlayPanel")
		{
			if (FONTS::ShouldReloadFonts())
				FONTS::InitFonts();

			if (INTERFACES::Engine->IsConnected() && INTERFACES::Engine->IsInGame())
			{
				GrenadePrediction::instance().Paint();
				visuals->DrawDamageIndicator();
				visuals->AsusProps();


				auto matpostprocess = INTERFACES::cvar->FindVar("mat_postprocess_enable");
				matpostprocess->fnChangeCallback = 0;
				matpostprocess->SetValue(SETTINGS::settings.matpostprocessenable);

				visuals->ModulateWorld();

				if (SETTINGS::settings.esp_bool)
				{
					visuals->Draw();
					visuals->ClientDraw();
				}
				if (SETTINGS::settings.hitmarker)
				{
					pHitmarker->Paint();
				}
				Flashlight.RunFrame();
			}

			MENU::PPGUI_PP_GUI::Begin();
			MENU::Do();
			MENU::PPGUI_PP_GUI::End();

			UTILS::INPUT::input_handler.Update();

			visuals->LogEvents();
		}
		ofunc(INTERFACES::Panel, VGUIPanel, ForceRepaint, AllowForce);
	}
	void __fastcall HookedFrameStageNotify(void* ecx, void* edx, int stage)
	{
		static auto ofunc = client.get_original<FrameStageNotifyFn>(37);
		auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());
		if (!local_player) return;
		Vector vecAimPunch, vecViewPunch;
		Vector* pAimPunch = nullptr; Vector* pViewPunch = nullptr;
		visuals->ModulateSky();
		switch (stage)
		{
			case FRAME_NET_UPDATE_POSTDATAUPDATE_START:

				skinchanger();
				GloveChanger();

			    if (INTERFACES::Engine->IsConnected() && INTERFACES::Engine->IsInGame())
				{
					for (int i = 1; i <= 65; i++)
					{
						auto entity = INTERFACES::ClientEntityList->GetClientEntity(i);
						if (!entity) continue;

						bool is_local_player = entity == local_player;
						bool is_teammate = local_player->GetTeam() == entity->GetTeam() && !is_local_player;

						if (is_local_player) continue;
						if (is_teammate) continue;
						if (entity->GetHealth() <= 0) continue;
						if (entity->GetIsDormant()) continue;

						if (SETTINGS::settings.ResolverEnable == 0) return;
						if (SETTINGS::settings.ResolverEnable == 1) {
							resolver->resolve(entity);
						//}
						//if (SETTINGS::settings.ResolverEnable == 2) {
						//	resolver2->resolve2(entity);
						//}
						//if (SETTINGS::settings.ResolverEnable == 3) {
						//	resolver3->resolve3(entity);
						}
					}
				} break;
			case FRAME_NET_UPDATE_POSTDATAUPDATE_END:
				break;
			case FRAME_RENDER_START:
				if (INTERFACES::Engine->IsConnected() && INTERFACES::Engine->IsInGame())
				{
					if (in_tp)
					{
						*(Vector*)((DWORD)local_player + 0x31C8) = Vector(GLOBAL::real_angles.x, GLOBAL::real_angles.y, 0.f);

						INTERFACES::pPrediction->SetLocalViewAngles(GLOBAL::real_angles);
						local_player->UpdateClientSideAnimation();
						INTERFACES::pPrediction->SetLocalViewAngles(GLOBAL::fake_angles);

					}
					for (int i = 1; i <= 65; i++)
					{
						auto entity = INTERFACES::ClientEntityList->GetClientEntity(i);
						if (!entity) continue;
						if (entity == local_player) continue;

						*(int*)((uintptr_t)entity + 0xA30) = INTERFACES::Globals->framecount;
						*(int*)((uintptr_t)entity + 0xA28) = 0;
					}
				} break;

			case FRAME_NET_UPDATE_START:
				if (INTERFACES::Engine->IsConnected() && INTERFACES::Engine->IsInGame())
				{
					if (SETTINGS::settings.beam_bool)
						visuals->DrawBulletBeams();
				} break;
			case FRAME_NET_UPDATE_END:
				if (INTERFACES::Engine->IsConnected() && INTERFACES::Engine->IsInGame())
				{
					for (int i = 1; i < 65; i++)
					{
						auto entity = INTERFACES::ClientEntityList->GetClientEntity(i);

						if (!entity)
							continue;

						if (!local_player)
							continue;

						bool is_local_player = entity == local_player;
						bool is_teammate = local_player->GetTeam() == entity->GetTeam() && !is_local_player;

						if (is_local_player)
							continue;

						if (is_teammate)
							continue;

						if (entity->GetHealth() <= 0)
							continue;
					}
				}
				break;
		}
		ofunc(ecx, stage);
	}
	void __fastcall HookedDrawModelExecute(void* ecx, void* edx, SDK::IMatRenderContext* context, const SDK::DrawModelState_t& state, const SDK::ModelRenderInfo_t& render_info, matrix3x4_t* matrix)
	{
		static auto ofunc = modelrender.get_original<DrawModelExecuteFn>(21);
		if (INTERFACES::Engine->IsConnected() && INTERFACES::Engine->IsInGame())
		{
			std::string ModelName = INTERFACES::ModelInfo->GetModelName(render_info.pModel);

			if (ModelName.find("v_sleeve") != std::string::npos)
			{
				SDK::IMaterial* material = INTERFACES::MaterialSystem->FindMaterial(ModelName.c_str(), TEXTURE_GROUP_MODEL);
				if (!material) return;
				material->SetMaterialVarFlag(SDK::MATERIAL_VAR_NO_DRAW, true);
				INTERFACES::ModelRender->ForcedMaterialOverride(material);
			}
			
		}
		std::string strModelName = INTERFACES::ModelInfo->GetModelName(render_info.pModel);
		if (SETTINGS::settings.wirehand_bool)
		{
			if (strModelName.find("arms") != std::string::npos && SETTINGS::settings.wirehand_bool)
			{
				SDK::IMaterial* WireHands = INTERFACES::MaterialSystem->FindMaterial(strModelName.c_str(), TEXTURE_GROUP_MODEL);
				WireHands->SetMaterialVarFlag(SDK::MATERIAL_VAR_WIREFRAME, true);
				INTERFACES::ModelRender->ForcedMaterialOverride(WireHands);
			}
		}
		ofunc(ecx, context, state, render_info, matrix);
	}

	void __fastcall HookedSceneEnd(void* ecx, void* edx)
	{
		static auto ofunc = sceneend.get_original<SceneEndFn>(9);
		ofunc(ecx);
		static SDK::IMaterial* ignorez = chams->CreateMaterialBasic(true, true, false);
		static SDK::IMaterial* notignorez = chams->CreateMaterialBasic(false, true, false);
		static SDK::IMaterial* ignorez_metallic = chams->CreateMaterialMetallic(true, true, false);
		static SDK::IMaterial* notignorez_metallic = chams->CreateMaterialMetallic(false, true, false);

		if (INTERFACES::Engine->IsConnected() && INTERFACES::Engine->IsInGame())
		{
			auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());
			if (!local_player) return;
			CColor color = SETTINGS::settings.glow_col, colorTeam = SETTINGS::settings.teamglow_color, colorlocal = SETTINGS::settings.glowlocal_col;
			for (int i = 1; i < 65; i++)
			{
				auto entity = INTERFACES::ClientEntityList->GetClientEntity(i);

				if (!entity) continue;
				if (!local_player) continue;

				bool is_local_player = entity == local_player;
				bool is_teammate = local_player->GetTeam() == entity->GetTeam() && !is_local_player;
				auto ignorezmaterial = SETTINGS::settings.chamstype == 0 ? ignorez_metallic : ignorez;
				auto notignorezmaterial = SETTINGS::settings.chamstype == 0 ? notignorez_metallic : notignorez;
				if (is_local_player)
				{
					switch (SETTINGS::settings.localchams)
					{
						case 0: continue; break;
						case 1:
							local_player->SetAbsOrigin(GLOBAL::FakePosition);
							local_player->DrawModel(0x1, 255);
							local_player->SetAbsOrigin(local_player->GetAbsOrigin());
							continue; break;
						case 2:
							notignorezmaterial->ColorModulate(SETTINGS::settings.localchams_col);
							INTERFACES::ModelRender->ForcedMaterialOverride(notignorezmaterial);
							local_player->DrawModel(0x1, 255);
							INTERFACES::ModelRender->ForcedMaterialOverride(nullptr);
							continue;  break;
						case 3:
							notignorezmaterial->ColorModulate(SETTINGS::settings.localchams_col);
							INTERFACES::ModelRender->ForcedMaterialOverride(notignorezmaterial);
							local_player->SetAbsOrigin(GLOBAL::FakePosition);
							local_player->DrawModel(0x1, 255);
							local_player->SetAbsOrigin(local_player->GetAbsOrigin());
							INTERFACES::ModelRender->ForcedMaterialOverride(nullptr);
							continue; break;
					}
				}

				if (entity->GetHealth() <= 0) continue;
				if (entity->GetIsDormant())	continue;
				if (entity->GetClientClass()->m_ClassID != 35) continue;

				if (SETTINGS::settings.draw_fake)
				{
					if (local_player)
					{
						if (notignorez)
						{
							Vector OrigAng;
							OrigAng = local_player->GetEyeAngles();
							local_player->SetAngle2(Vector(0, local_player->GetLowerBodyYaw(), 0));
							bool LbyColor = false;
							float NormalColor[3] = { 1,1,1 };
							float lbyupdatecolor[3] = { 0,1,0 };
							INTERFACES::RenderView->SetColorModulation(SETTINGS::settings.fake_darw_col);
							INTERFACES::ModelRender->ForcedMaterialOverride(notignorez);
							local_player->DrawModel(0x1, 255);
							INTERFACES::ModelRender->ForcedMaterialOverride(nullptr);
							local_player->SetAngle2(OrigAng);
						}
					}
				}

				if (is_teammate)
				{
					if (entity && SETTINGS::settings.chamsteam == 2)
					{
						ignorezmaterial->ColorModulate(SETTINGS::settings.teaminvis_color);
						INTERFACES::ModelRender->ForcedMaterialOverride(ignorezmaterial);
						entity->DrawModel(0x1, 255);

						notignorezmaterial->ColorModulate(SETTINGS::settings.teamvis_color);
						INTERFACES::ModelRender->ForcedMaterialOverride(notignorezmaterial);
						entity->DrawModel(0x1, 255);

						INTERFACES::ModelRender->ForcedMaterialOverride(nullptr);
					}
					else if (entity && SETTINGS::settings.chamsteam == 1)
					{
						notignorezmaterial->ColorModulate(SETTINGS::settings.teamvis_color);
						INTERFACES::ModelRender->ForcedMaterialOverride(notignorezmaterial);
						entity->DrawModel(0x1, 255);

						INTERFACES::ModelRender->ForcedMaterialOverride(nullptr);
					} continue;
				}
				else if (is_teammate && SETTINGS::settings.chamsteam)
					continue;

				if (entity && SETTINGS::settings.chams_type == 2)
				{
					ignorezmaterial->ColorModulate(SETTINGS::settings.imodel_col);
					INTERFACES::ModelRender->ForcedMaterialOverride(ignorezmaterial);
					entity->DrawModel(0x1, 255);

					notignorezmaterial->ColorModulate(SETTINGS::settings.vmodel_col);
					INTERFACES::ModelRender->ForcedMaterialOverride(notignorezmaterial);
					entity->DrawModel(0x1, 255);

					INTERFACES::ModelRender->ForcedMaterialOverride(nullptr);
				}
				else if (entity && SETTINGS::settings.chams_type == 1)
				{
					notignorezmaterial->ColorModulate(SETTINGS::settings.vmodel_col);
					INTERFACES::ModelRender->ForcedMaterialOverride(notignorezmaterial);
					entity->DrawModel(0x1, 255);

					INTERFACES::ModelRender->ForcedMaterialOverride(nullptr);
				}
			}

			for (auto i = 0; i < INTERFACES::GlowObjManager->GetSize(); i++)
			{
				auto &glowObject = INTERFACES::GlowObjManager->m_GlowObjectDefinitions[i];
				auto entity = reinterpret_cast<SDK::CBaseEntity*>(glowObject.m_pEntity);

				if (!entity) continue;
				if (!local_player) continue;

				if (glowObject.IsUnused()) continue;

				bool is_local_player = entity == local_player;
				bool is_teammate = local_player->GetTeam() == entity->GetTeam() && !is_local_player;

				if (is_local_player && in_tp && SETTINGS::settings.glowlocal)
				{
					glowObject.m_nGlowStyle = SETTINGS::settings.glowstylelocal;
					glowObject.m_flRed = colorlocal.RGBA[0] / 255.0f;
					glowObject.m_flGreen = colorlocal.RGBA[1] / 255.0f;
					glowObject.m_flBlue = colorlocal.RGBA[2] / 255.0f;
					glowObject.m_flAlpha = colorlocal.RGBA[3] / 255.0f;
					glowObject.m_bRenderWhenOccluded = true;
					glowObject.m_bRenderWhenUnoccluded = false;
					continue;
				}
				else if (!SETTINGS::settings.glowlocal && is_local_player)
					continue;

				if (entity->GetHealth() <= 0) continue;
				if (entity->GetIsDormant())	continue;
				if (entity->GetClientClass()->m_ClassID != 35) continue;

				if (is_teammate && SETTINGS::settings.glowteam)
				{
					glowObject.m_nGlowStyle = SETTINGS::settings.glowstyle;
					glowObject.m_flRed = colorTeam.RGBA[0] / 255.0f;
					glowObject.m_flGreen = colorTeam.RGBA[1] / 255.0f;
					glowObject.m_flBlue = colorTeam.RGBA[2] / 255.0f;
					glowObject.m_flAlpha = colorTeam.RGBA[3] / 255.0f;
					glowObject.m_bRenderWhenOccluded = true;
					glowObject.m_bRenderWhenUnoccluded = false;
					continue;
				}
				else if (is_teammate && !SETTINGS::settings.glowteam)
					continue;

				if (SETTINGS::settings.glowenable)
				{
					glowObject.m_nGlowStyle = SETTINGS::settings.glowstyle;
					glowObject.m_flRed = color.RGBA[0] / 255.0f;
					glowObject.m_flGreen = color.RGBA[1] / 255.0f;
					glowObject.m_flBlue = color.RGBA[2] / 255.0f;
					glowObject.m_flAlpha = color.RGBA[3] / 255.0f;
					glowObject.m_bRenderWhenOccluded = true;
					glowObject.m_bRenderWhenUnoccluded = false;
				}
			}

			if (SETTINGS::settings.smoke_bool)
			{
				std::vector<const char*> vistasmoke_wireframe =  { "particle/vistasmokev1/vistasmokev1_smokegrenade" };

				std::vector<const char*> vistasmoke_nodraw =
				{
					"particle/vistasmokev1/vistasmokev1_fire",
					"particle/vistasmokev1/vistasmokev1_emods",
					"particle/vistasmokev1/vistasmokev1_emods_impactdust",
				};

				for (auto mat_s : vistasmoke_wireframe)
				{
					SDK::IMaterial* mat = INTERFACES::MaterialSystem->FindMaterial(mat_s, TEXTURE_GROUP_OTHER);
					mat->SetMaterialVarFlag(SDK::MATERIAL_VAR_WIREFRAME, true);
				}

				for (auto mat_n : vistasmoke_nodraw)
				{
					SDK::IMaterial* mat = INTERFACES::MaterialSystem->FindMaterial(mat_n, TEXTURE_GROUP_OTHER);
					mat->SetMaterialVarFlag(SDK::MATERIAL_VAR_NO_DRAW, true);
				}

				static auto smokecout = *(DWORD*)(linegoesthrusmoke + 0x8);
				*(int*)(smokecout) = 0;
			}
		}
	}


	void __fastcall HookedOverrideView(void* ecx, void* edx, SDK::CViewSetup* pSetup)
	{
		static auto ofunc = overrideview.get_original<OverrideViewFn>(18);
		auto local_player = INTERFACES::ClientEntityList->GetClientEntity(INTERFACES::Engine->GetLocalPlayer());
		if (!local_player) return;

		auto animstate = local_player->GetAnimState();
		if (!animstate) return;

		if (GetAsyncKeyState(UTILS::INPUT::input_handler.keyBindings(SETTINGS::settings.thirdperson_int)) & 1)
			in_tp = !in_tp;

		if (SETTINGS::settings.novisualrecoil_bool)
		{
			pSetup->angles -= *local_player->GetAimPunchAngle() * 0.9f + *local_player->GetViewPunchAngle();
		}

		if (INTERFACES::Engine->IsConnected() && INTERFACES::Engine->IsInGame())
		{
			GrenadePrediction::instance().View(pSetup);
			auto GetCorrectDistance = [&local_player](float ideal_distance) -> float
			{
				Vector inverse_angles;
				INTERFACES::Engine->GetViewAngles(inverse_angles);

				inverse_angles.x *= -1.f, inverse_angles.y += 180.f;

				Vector direction;
				MATH::AngleVectors(inverse_angles, &direction);

				SDK::CTraceWorldOnly filter;
				SDK::trace_t trace;
				SDK::Ray_t ray;

				ray.Init(local_player->GetVecOrigin() + local_player->GetViewOffset(), (local_player->GetVecOrigin() + local_player->GetViewOffset()) + (direction * (ideal_distance + 5.f)));
				INTERFACES::Trace->TraceRay(ray, MASK_ALL, &filter, &trace);

				return ideal_distance * trace.flFraction;
			};

			if (SETTINGS::settings.tp_bool && in_tp)
			{
				if (local_player->GetHealth() <= 0)
					local_player->SetObserverMode(5);

				if (!INTERFACES::Input->m_fCameraInThirdPerson)
				{
					INTERFACES::Input->m_fCameraInThirdPerson = true;
					if (animstate->m_bInHitGroundAnimation && ground_tick > 1)
						INTERFACES::Input->m_vecCameraOffset = Vector(0.0f, GLOBAL::real_angles.y, GetCorrectDistance(100));
					else
						INTERFACES::Input->m_vecCameraOffset = Vector(GLOBAL::real_angles.x, GLOBAL::real_angles.y, GetCorrectDistance(100));
					Vector camForward;

					MATH::AngleVectors(Vector(INTERFACES::Input->m_vecCameraOffset.x, INTERFACES::Input->m_vecCameraOffset.y, 0), &camForward);
				}
			}
			else
			{
				INTERFACES::Input->m_fCameraInThirdPerson = false;
				INTERFACES::Input->m_vecCameraOffset = Vector(GLOBAL::real_angles.x, GLOBAL::real_angles.y, 0);
			}
			auto zoomsensration = INTERFACES::cvar->FindVar("zoom_sensitivity_ratio_mouse");
			if (SETTINGS::settings.fixscopesens)
				zoomsensration->SetValue("0");
			else
				zoomsensration->SetValue("1");

			if (SETTINGS::settings.aim_type == 0)
			{
				if (!local_player->GetIsScoped())
					pSetup->fov = SETTINGS::settings.fov_val;
				else if (local_player->GetIsScoped() && SETTINGS::settings.removescoping)
					pSetup->fov = SETTINGS::settings.fov_val;
			}
			else if (!(SETTINGS::settings.aim_type == 0) && !local_player->GetIsScoped())
				pSetup->fov = 90;
		}
		ofunc(ecx, pSetup);
	}
	void __fastcall HookedTraceRay(void *thisptr, void*, const SDK::Ray_t &ray, unsigned int fMask, SDK::ITraceFilter *pTraceFilter, SDK::trace_t *pTrace)
	{
		static auto ofunc = trace.get_original<TraceRayFn>(5);
		ofunc(thisptr, ray, fMask, pTraceFilter, pTrace);
		if (INTERFACES::Engine->IsConnected() && INTERFACES::Engine->IsInGame())
			pTrace->surface.flags |= SURF_SKY;
	}
	bool __fastcall HookedGetBool(void* pConVar, void* edx)
	{
		static auto ofunc = getbool.get_original<SvCheatsGetBoolFn>(13);
		if ((uintptr_t)_ReturnAddress() == CAM_THINK)
			return true;

		return ofunc(pConVar);
	}
	float __fastcall GetViewmodelFOV()
	{
		static auto ofunc = overrideview.get_original<GetViewmodelFOVFn>(35);
		ofunc();
		if (INTERFACES::Engine->IsConnected() && INTERFACES::Engine->IsInGame())
		{
			float player_fov = original_viewmodel_fov();

			if (SETTINGS::settings.esp_bool)
				player_fov = SETTINGS::settings.viewfov_val;

			return player_fov;
		}
	}

	void Hook_LockCursor(void* xd)
	{
		SurfaceTable.UnHook();
		INTERFACES::Surface->lockcursor();
		SurfaceTable.ReHook();
		if (menu_open)
			INTERFACES::Surface->unlockcursor();
	}

	void InitHooks()
	{
		client.setup(INTERFACES::Client);
		client.hook_index(37, HookedFrameStageNotify);

		panel.setup(INTERFACES::Panel);
		panel.hook_index(41, HookedPaintTraverse);

		modelrender.setup(INTERFACES::ModelRender);
		modelrender.hook_index(21, HookedDrawModelExecute);
		
		sceneend.setup(INTERFACES::RenderView);
		sceneend.hook_index(9, HookedSceneEnd);

		trace.setup(INTERFACES::Trace);
		trace.hook_index(5, HookedTraceRay);

		overrideview.setup(INTERFACES::ClientMode);
		overrideview.hook_index(18, HookedOverrideView);
		overrideview.hook_index(24, HookedCreateMove);

		getbool.setup(reinterpret_cast<DWORD**>(INTERFACES::cvar->FindVar("sv_cheats")));
		getbool.hook_index(13, HookedGetBool);



		SurfaceTable.InitTable(INTERFACES::Surface);
		SurfaceTable.HookIndex(67, Hook_LockCursor);

	}
	void EyeAnglesPitchHook(const SDK::CRecvProxyData *pData, void *pStruct, void *pOut)
	{
		*reinterpret_cast<float*>(pOut) = pData->m_Value.m_Float;

		auto entity = reinterpret_cast<SDK::CBaseEntity*>(pStruct);
		if (!entity)
			return;

	}
	void EyeAnglesYawHook(const SDK::CRecvProxyData *pData, void *pStruct, void *pOut)
	{
		*reinterpret_cast<float*>(pOut) = pData->m_Value.m_Float;

		auto entity = reinterpret_cast<SDK::CBaseEntity*>(pStruct);
		if (!entity)
			return;
	}
	void InitNetvarHooks()
	{
		UTILS::netvar_hook_manager.Hook("DT_CSPlayer", "m_angEyeAngles[0]", EyeAnglesPitchHook);
		UTILS::netvar_hook_manager.Hook("DT_CSPlayer", "m_angEyeAngles[1]", EyeAnglesYawHook);
	}
}



