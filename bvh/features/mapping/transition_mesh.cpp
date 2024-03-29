#include "bmfile.h"
#include "Ge2Virtools.h"

namespace bvh {
	namespace features {
		namespace mapping {
			namespace bmfile {
				namespace mesh_transition {

					BmTransitionVertex::BmTransitionVertex() :
						m_Vtx(), m_UV(), m_Norm() {
						;
					}
					BmTransitionVertex::BmTransitionVertex(VxVector& vtx, Vx2DVector& uv, VxVector& norm) :
						m_Vtx(vtx.x, vtx.y, vtx.z), m_UV(uv.x, uv.y), m_Norm(norm.x, norm.y, norm.z) {
						;
					}

					BmTransitionFace::BmTransitionFace() :
						ind1(UINT32_C(0)), ind2(UINT32_C(1)), ind3(UINT32_C(2)),
						use_material(UINT8_C(0)), material_index(UINT32_C(0)) {
						;
					}
					BmTransitionFace::BmTransitionFace(uint32_t _i1, uint32_t _i2, uint32_t _i3, uint8_t use_mtl, uint32_t mtl_id) :
						ind1(_i1), ind2(_i2), ind3(_i3),
						use_material(use_mtl), material_index(mtl_id) {
						;
					}
					bool BmTransitionVertexCompare::operator()(const BmTransitionVertex& lhs, const BmTransitionVertex& rhs) const {
						return memcmp(&lhs, &rhs, sizeof(BmTransitionVertex)) < 0;
					}


					MeshTransition::MeshTransition(CKContext* ctx) :
						m_In_Ctx(ctx), m_In_Mesh(NULL),
						m_In_Vtx(NULL), m_In_UV(NULL), m_In_Norm(NULL), 
						m_In_Face(NULL), m_In_FaceAlt(NULL),
						m_In_MaterialList(NULL),
						m_Out_Vertex(), m_Out_FaceIndices(),
						m_DupRemover(), m_IsComponent(FALSE) {
						;
					}
					MeshTransition::~MeshTransition() {
						;
					}

					void MeshTransition::DoMeshParse(
						CKMesh* mesh,
						std::vector<VxVector>* vtx, std::vector<Vx2DVector>* uv, std::vector<VxVector>* norm,
						std::vector<BM_FACE_PROTOTYPE>* face,
						std::vector<FILE_INDEX_HELPER*>* mtl_list
					) {
						m_IsComponent = FALSE;

						// check
						if (!(mesh && vtx && uv && norm && face && mtl_list)) {
							return;
						}

						m_In_Mesh = mesh;
						m_In_Vtx = vtx;
						m_In_UV = uv;
						m_In_Norm = norm;
						m_In_Face = face;
						m_In_FaceAlt = NULL;
						m_In_MaterialList = mtl_list;

						DoRealParse();
						ApplyToMaterial();
					}

					void MeshTransition::DoComponentParse(
						CKMesh* mesh,
						std::vector<VxVector>* vtx, std::vector<VxVector>* norm,
						std::vector<COMPONENT_FACE_PROTOTYPE>* face
					) {
						m_IsComponent = TRUE;

						// check
						if (!(mesh && vtx && norm && face)) {
							return;
						}

						m_In_Mesh = mesh;
						m_In_Vtx = vtx;
						m_In_UV = NULL;
						m_In_Norm = norm;
						m_In_Face = NULL;
						m_In_FaceAlt = face;
						m_In_MaterialList = NULL;

						DoRealParse();
						ApplyToMaterial();
					}

					void MeshTransition::DoRealParse() {
						// clear intermidated val
						m_DupRemover.clear();
						// clear result
						m_Out_Vertex.clear();
						m_Out_FaceIndices.clear();

						// reserve vector to prevent extra mem alloc
						// use the most bad situation to reserve
						uint32_t face_size = m_IsComponent ? m_In_FaceAlt->size() : m_In_Face->size();
						m_Out_Vertex.reserve(face_size * 3);
						m_Out_FaceIndices.reserve(face_size);

						// iterate face
						for (size_t faceid = 0; faceid < face_size; ++faceid) {
							uint32_t idx[3];
							for (int j = 0; j < 3; ++j) {
								// create one first
								PushVertex(faceid, j);

								// try insert it
								auto insert_result = m_DupRemover.try_emplace(m_Out_Vertex.back(), m_Out_Vertex.size() - 1);
								idx[j] = (*(insert_result.first)).second;
								if (!insert_result.second) {
									// have existed key
									// remove prev added
									m_Out_Vertex.pop_back();
								}
							}

							// create face
							PushFace(faceid, idx);
						}
					}

					void MeshTransition::ApplyToMaterial() {
						m_In_Mesh->SetVertexCount(m_Out_Vertex.size());
						int mesh_assignCounter = 0;
						for (auto it = m_Out_Vertex.begin(); it != m_Out_Vertex.end(); (++it), (++mesh_assignCounter)) {
							// set v vn
							m_In_Mesh->SetVertexPosition(mesh_assignCounter, &((*it).m_Vtx));
							m_In_Mesh->SetVertexNormal(mesh_assignCounter, &((*it).m_Norm));
							// only set vt for non-component
							if (!m_IsComponent)
								m_In_Mesh->SetVertexTextureCoordinates(mesh_assignCounter, (*it).m_UV.x, (*it).m_UV.y);
						}

						m_In_Mesh->SetFaceCount(m_Out_FaceIndices.size());
						mesh_assignCounter = 0;
						for (auto it = m_Out_FaceIndices.begin(); it != m_Out_FaceIndices.end(); (++it), (++mesh_assignCounter)) {
							// set indices
							m_In_Mesh->SetFaceVertexIndex(mesh_assignCounter, (*it).ind1, (*it).ind2, (*it).ind3);

							// set material only for non-component
							if (!m_IsComponent && (*it).use_material) {
								m_In_Mesh->SetFaceMaterial(
									mesh_assignCounter,
									(CKMaterial*)m_In_Ctx->GetObjectA((*m_In_MaterialList)[(*it).material_index]->id)
								);
							}
						}
					}

					void MeshTransition::PushVertex(size_t face_index, int indices_index) {
						static Vx2DVector empty_uv(0.0f, 0.0f);
						uint32_t* face_indices_decl;

						if (m_IsComponent) {
							face_indices_decl = &((*m_In_FaceAlt)[face_index].vbin[indices_index * 2]);
							m_Out_Vertex.emplace_back(
								(*m_In_Vtx)[face_indices_decl[0]],
								empty_uv,	// use blank uv to fill it
								(*m_In_Norm)[face_indices_decl[1]]
							);
						} else {
							face_indices_decl = &((*m_In_Face)[face_index].indices.vbin[indices_index * 3]);
							m_Out_Vertex.emplace_back(
								(*m_In_Vtx)[face_indices_decl[0]],
								(*m_In_UV)[face_indices_decl[1]],
								(*m_In_Norm)[face_indices_decl[2]]
							);
						}
					}

					void MeshTransition::PushFace(size_t face_index, uint32_t idx[3]) {
						if (m_IsComponent) {
							m_Out_FaceIndices.emplace_back(
								idx[0], idx[1], idx[2],
								UINT8_C(0), UINT32_C(0)
							);
						} else {
							BM_FACE_PROTOTYPE* proto = &((*m_In_Face)[face_index]);
							m_Out_FaceIndices.emplace_back(
								idx[0], idx[1], idx[2],
								proto->use_material, proto->material_index
							);
						}
					}

				}
			}
		}
	}
}

