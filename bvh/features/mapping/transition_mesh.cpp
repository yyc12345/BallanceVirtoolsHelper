#include "bmfile.h"

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
					BmTransitionFace::BmTransitionFace(uint32_t _i1, uint32_t _i2, uint32_t _i3, BM_FACE_PROTOTYPE& parent) :
						ind1(_i1), ind2(_i2), ind3(_i3),
						use_material(parent.use_material), material_index(parent.material_index) {
						;
					}

					// Generate from Visual Studio with stuppid adaption.
					size_t BmTransitionVertexHash::operator()(const PBmTransitionVertex& cla) const {
#define DO_HASH(hashcode, const_value, fvalue) hashcode = hashcode * const_value + (*((int32_t*)(&(fvalue))))
						int32_t hashcode_pos = 373119288;
						DO_HASH(hashcode_pos, -1521134295, cla->m_Vtx.x);
						DO_HASH(hashcode_pos, -1521134295, cla->m_Vtx.y);
						DO_HASH(hashcode_pos, -1521134295, cla->m_Vtx.z);

						int32_t hashcode_uv = -1237110951;
						DO_HASH(hashcode_pos, -1521134295, cla->m_UV.x);
						DO_HASH(hashcode_pos, -1521134295, cla->m_UV.y);

						int32_t hashcode_normal = 373119288;
						DO_HASH(hashcode_pos, -1521134295, cla->m_Norm.x);
						DO_HASH(hashcode_pos, -1521134295, cla->m_Norm.y);
						DO_HASH(hashcode_pos, -1521134295, cla->m_Norm.z);

						int32_t hashcode = -436449171;
						DO_HASH(hashcode_pos, -1521134295, hashcode_pos);
						DO_HASH(hashcode_pos, -1521134295, hashcode_uv);
						DO_HASH(hashcode_pos, -1521134295, hashcode_normal);

						// force convert it into unsigned
						uint32_t uhashcode = *(uint32_t*)(&hashcode);
						// return converted
						return (size_t)uhashcode;
#undef DO_HASH
					}

					bool BmTransitionVertexEqual::operator()(const PBmTransitionVertex& c1, const PBmTransitionVertex& c2) const {
						return c1->m_Vtx == c2->m_Vtx &&
							c1->m_UV == c2->m_UV &&
							c1->m_Norm == c2->m_Norm;
					}


					MeshTransition::MeshTransition() :
						m_In_Vtx(NULL), m_In_UV(NULL), m_In_Norm(NULL), m_In_Face(NULL),
						m_Out_Vertex(), m_Out_FaceIndices(),
						m_DupRemover() {
						;
					}
					MeshTransition::~MeshTransition() {
						;
					}

					void MeshTransition::DoParse() {
						// clear inermidated val
						m_DupRemover.clear();
						// clear result
						m_Out_Vertex.clear();
						m_Out_FaceIndices.clear();

						// check input requirements
						if (m_In_Vtx == NULL || m_In_UV == NULL || m_In_Norm == NULL ||
							m_In_Face == NULL) {
							return;
						}





					}


				}
			}
		}
	}
}

