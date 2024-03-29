#pragma once

#include "../../../stdafx.h"
#include "../../utils/param_package.h"
#include <fstream>
#include <unordered_set>
#include <map>
#include <vector>
#include <Ge2Virtools.h>

#define BM_FILE_VERSION 14

namespace bvh {
	namespace features {
		namespace mapping {
			namespace bmfile {

				enum FILE_INDEX_TYPE {
					FILE_INDEX_TYPE__OBJECT = 0,
					FILE_INDEX_TYPE__MESH = 1,
					FILE_INDEX_TYPE__MATERIAL = 2,
					FILE_INDEX_TYPE__TEXTURE = 3
				};

				struct FILE_INDEX_HELPER {
					std::string name;
					CK_ID id;
					uint64_t offset;
				};

#pragma pack(1)
				union COMPONENT_FACE_PROTOTYPE {
					struct COMPONENT_FACE_PROTOTYPE_DATA {
						uint32_t v1, vn1;
						uint32_t v2, vn2;
						uint32_t v3, vn3;
					}data;
					uint32_t vbin[sizeof(COMPONENT_FACE_PROTOTYPE_DATA) / sizeof(uint32_t)];
				};
				struct BM_FACE_PROTOTYPE {
					union {
						struct BM_FACE_PROTOTYPE_DATA_V {
							uint32_t v1, vt1, vn1;
							uint32_t v2, vt2, vn2;
							uint32_t v3, vt3, vn3;
						}data;
						uint32_t vbin[sizeof(BM_FACE_PROTOTYPE_DATA_V) / sizeof(uint32_t)];
					}indices;

					uint8_t use_material;
					uint32_t material_index;
				};
#pragma pack()

				namespace mesh_transition {

#pragma pack(1)
					struct BmTransitionVertex {
						BmTransitionVertex();
						BmTransitionVertex(VxVector& vtx, Vx2DVector& uv, VxVector& norm);
						VxVector m_Vtx;
						Vx2DVector m_UV;
						VxVector m_Norm;
					};
#pragma pack()

					struct BmTransitionVertexCompare {
						bool operator()(const BmTransitionVertex& lhs, const BmTransitionVertex& rhs) const;
					};

					struct BmTransitionFace {
						BmTransitionFace();
						BmTransitionFace(uint32_t _i1, uint32_t _i2, uint32_t _i3, uint8_t use_mtl, uint32_t mtl_id);
						uint32_t ind1, ind2, ind3;

						uint8_t use_material;
						uint32_t material_index;
					};

					class MeshTransition {
					public:
						MeshTransition(CKContext* ctx);
						~MeshTransition();

						void DoMeshParse(
							CKMesh* mesh,
							std::vector<VxVector>* vtx, std::vector<Vx2DVector>* uv, std::vector<VxVector>* norm,
							std::vector<BM_FACE_PROTOTYPE>* face,
							std::vector<FILE_INDEX_HELPER*>* mtl_list
						);
						void DoComponentParse(
							CKMesh* mesh,
							std::vector<VxVector>* vtx, std::vector<VxVector>* norm,
							std::vector<COMPONENT_FACE_PROTOTYPE>* face
						);
						
					private:
						void DoRealParse();
						void ApplyToMaterial();
						void PushVertex(size_t face_index, int indices_index);
						void PushFace(size_t face_index, uint32_t idx[3]);

						CKContext* m_In_Ctx;

						BOOL m_IsComponent;
						CKMesh* m_In_Mesh;
						std::vector<VxVector>* m_In_Vtx, * m_In_Norm;
						std::vector<Vx2DVector>* m_In_UV;
						std::vector<BM_FACE_PROTOTYPE>* m_In_Face;
						std::vector<COMPONENT_FACE_PROTOTYPE>* m_In_FaceAlt;
						std::vector<FILE_INDEX_HELPER*>* m_In_MaterialList;

						std::vector<BmTransitionVertex> m_Out_Vertex;
						std::vector<BmTransitionFace> m_Out_FaceIndices;

						// unordered_map have performance problem when dealing with massive data (in this case, big mesh)
						// so we use map to get stable time cost.
						std::map<BmTransitionVertex, uint32_t, BmTransitionVertexCompare> m_DupRemover;
					};

				}

				extern const uint32_t CONST_ExternalTextureList_Length;
				extern const char* CONST_ExternalTextureList[];
				extern const uint32_t CONST_ExternalComponent_Length;
				extern const char* CONST_ExternalComponent[];

				void ImportBM(utils::ParamPackage* pkg);
				// WARNING: all following `Read` func are based on current OS is little-endian.
				template<typename T>
				inline void readData(std::ifstream* fs, T* data) {
					fs->read((char*)data, (std::streamsize)sizeof(T));
				}
				template<typename T>
				inline void readArrayData(std::ifstream* fs, T* arr, uint32_t count) {
					fs->read((char*)arr, ((std::streamsize)sizeof(T)) * ((std::streamsize)count));
				}
				template<typename T>
				inline void readVectorData(std::ifstream* fs, std::vector<T>* vec, uint32_t count) {
					vec->resize(count);
					fs->read((char*)vec->data(), ((std::streamsize)sizeof(T)) * ((std::streamsize)count));
				}
				inline void readBool(std::ifstream* fs, BOOL* boolean) {
					uint8_t num;
					readData<uint8_t>(fs, &num);
					*boolean = num ? TRUE : FALSE;
				}
				void readString(std::ifstream* fs, std::string* str);
				void loadExternalTexture(std::string* name, CKTexture* texture, utils::ParamPackage* pkg);
				void loadComponenetMesh(CKContext* ctx, CKScene* scene, CK3dEntity* obj, mesh_transition::MeshTransition converter, uint32_t model_index);
				BOOL isComponentInStandard(std::string* name);
				CKObject* userCreateCKObject(CKContext* ctx, CK_CLASSID cls, const char* name, BOOL use_rename, BOOL* is_existed);

				void ExportBM(utils::ParamPackage* pkg);
				// WARNING: all following `Write` func are based on current OS is little-endian.
				template<typename T>
				inline void writeData(std::ofstream* fs, T* data) {
					fs->write((char*)data, sizeof(T));
				}
				template<typename T>
				inline void writeArrayData(std::ofstream* fs, T* arr, uint32_t count) {
					fs->write((char*)arr, ((std::streamsize)sizeof(T)) * ((std::streamsize)count));
				}
				template<typename T>
				inline void writeVectorData(std::ofstream* fs, std::vector<T>* vec) {
					fs->write((char*)vec->data(), ((std::streamsize)sizeof(T)) * ((std::streamsize)vec->size()));
				}
				inline void writeBool(std::ofstream* fs, BOOL* boolean) {
					uint8_t num = *boolean ? 1 : 0;
					writeData<uint8_t>(fs, &num);
				}
				void writeString(std::ofstream* fs, std::string* str);
				BOOL isValidObject(CK3dEntity* obj);
				void getComponent(std::unordered_set<CK_ID>* fncgSet, CK3dEntity* obj, BOOL* is_component, uint32_t* gottten_id);
				BOOL isExternalTexture(CKContext* ctx, CKTexture* texture, std::string* name);
				void safelyGetName(CKObject* obj, std::string* name);
				void putIndexHeader(std::ofstream* findex, CKObject* data, FILE_INDEX_TYPE t, uint64_t offset);
				template<typename T>
				uint32_t tryAddWithPtr(std::vector<T*>* list, T* newValue);

				void FixTexture(utils::ParamPackage* pkg);

			}
		}
	}
}
