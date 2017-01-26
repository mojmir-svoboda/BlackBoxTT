#pragma once
#include "WorkSpacesConfig.h"
#include "WorkSpacesGraph.h"
#include "VirtualDesktopManager.h"
#include <platform_win.h>

namespace bb {

	struct NotifWindow
	{
		uint32_t m_counter { 0 };
		uint32_t m_counterMax { 32 };
		uint32_t m_timeout1_ms { 16 };
		uint32_t m_timeout2_ms { 1 };
		HWND m_window { nullptr };

		static LRESULT CALLBACK notifWndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		//LRESULT WndProcHandler (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	};

	struct WorkSpaces
	{
		WorkSpacesConfig m_config;
		WorkSpacesGraph m_graph;
		std::unique_ptr<VirtualDesktopManager> m_vdm;
		NotifWindow m_notif;

		WorkSpaces ();
		~WorkSpaces ();

		bool Init (WorkSpacesConfig & config);
		void InitClusterAndVertex ();
		void InitNotifWindow ();
		void DoneNotifWindow ();
		bool Done ();
		bool CreateGraph ();
		void ClearGraph ();
		bool RecreateGraph ();

		void OnWindowCreated ();
		void OnWindowDestroyed ();
		void OnGraphConfigurationChanged ();
		void OnSwitchedDesktop ();

		void SetCurrentClusterId (bbstring const & id);
		bool SetCurrentVertexId (bbstring const & id);
		bool CanSetCurrentVertexId (bbstring const & id) const;
		bool CanSwitchVertexViaEdge (bbstring const & edge_id, bbstring & target_vertex_id) const;
		bool SwitchDesktop (bbstring const & vertex_id);
		bool IsVertexVDM (bbstring const & vertex_id) const;
		bool IsVertexVDM (bbstring const & vertex_id, size_t & idx) const;
		GUID GetVertexGUID (size_t idx) const;

		bbstring const & GetCurrentClusterId () const { return m_config.m_currentClusterId; }
		bbstring const * GetCurrentVertexId () const;
		WorkGraphConfig const * FindCluster(bbstring const & cluster_id) const;
		WorkGraphConfig const * FindClusterForVertex (bbstring const & vertex_id) const;
		WorkGraphConfig * FindCluster (bbstring const & cluster_id);
		WorkGraphConfig * FindClusterForVertex(bbstring const & vertex_id);

		bool AssignWorkSpace (HWND hwnd, bbstring & vertex_id);
	};
}
