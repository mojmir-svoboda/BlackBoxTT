#pragma once
#include "WorkSpacesConfig.h"
#include "WorkSpacesGraph.h"

namespace bb {

  struct WorkSpaces
  {
    WorkSpacesConfig m_config;
		WorkSpacesGraph m_graph;

		WorkSpaces ();
		~WorkSpaces ();

		bool Init (WorkSpacesConfig & config);
		void InitClusterAndVertex ();
		bool Done ();
		bool CreateGraph ();
		void ClearGraph ();

		void OnWindowCreated ();
		void OnWindowDestroyed ();

		void SetCurrentClusterId (bbstring const & id);
		bool SetCurrentVertexId (bbstring const & id);
		bool CanSetCurrentVertexId (bbstring const & id) const;
		bool CanSwitchVertexViaEdge (bbstring const & edge_id, bbstring & target_vertex_id) const;

		bbstring const & GetCurrentClusterId () const { return m_config.m_currentClusterId; }
		bbstring const * GetCurrentVertexId () const;
		WorkGraphConfig const * FindCluster(bbstring const & cluster_id) const;
		WorkGraphConfig const * FindClusterForVertex (bbstring const & vertex_id) const;
		WorkGraphConfig * FindCluster (bbstring const & cluster_id);
		WorkGraphConfig * FindClusterForVertex(bbstring const & vertex_id);

		bool SwitchVertexViaEdge (bbstring const & edge_property, bbstring & target_vertex_id);

		void OnSwitchFromVertex (bbstring const & old_vertex_id, bbstring const & new_vertex_id);
	};
}
