#include "stdafx.h"
#include "gep/modelloader.h"
#include "gep/file.h"
#include "gep/exception.h"
#include "gep/chunkfile.h"
#include <sstream>

namespace {
    float readCompressedFloat(gep::Chunkfile& file)
    {
        gep::int16 data = 0;
        file.read(data);
        return (float)data / (float)std::numeric_limits<gep::int16>::max();
    }
}

gep::ModelLoader::ModelLoader(IAllocator* pAllocator) :
    m_pMeshDataAllocator(nullptr),
    m_pStartMarker(nullptr)
{
    if(pAllocator == nullptr)
        pAllocator = &g_stdAllocator;
    m_pAllocator = pAllocator;
}

gep::ModelLoader::~ModelLoader()
{
    if(m_pMeshDataAllocator != nullptr)
    {
        m_pMeshDataAllocator->freeToMarker(m_pStartMarker);
        GEP_DELETE(m_pAllocator, m_pMeshDataAllocator);
    }
}

void gep::ModelLoader::loadFile(const char* pFilename, uint32 loadWhat)
{
    GEP_ASSERT(!m_modelData.hasData,"LoadFile can only be called once");
    m_filename = pFilename;

    if(!fileExists(pFilename))
    {
        std::ostringstream msg;
        msg << "File '" << pFilename << "' does not exist";
        throw LoadingError(msg.str());
    }

    Chunkfile file(pFilename, Chunkfile::Operation::read);

    if(file.startReading("thModel") != SUCCESS)
    {
        std::ostringstream msg;
        msg << "File '" << pFilename << "' is not a thModel format";
        throw LoadingError(msg.str());
    }

    if(file.getFileVersion() > ModelFormatVersion::Version3)
    {
        std::ostringstream msg;
        msg << "File '" << pFilename << "' does have a old format, please reexport";
        throw LoadingError(msg.str());
    }

    MemoryStatistics memstat;

    uint32 nodeNameMemory;

    Load::Enum loadTexCoords[] = {Load::TexCoords0, Load::TexCoords1, Load::TexCoords2, Load::TexCoords3};

    uint32 texturePathMemory = 0;
    uint32 materialNameMemory = 0;
    uint32 boneNameMemory = 0;
    //Read the size info
    {
        const size_t alignmentOverhead = AlignmentHelper::__ALIGNMENT - 1;
        file.startReadChunk();
        if(file.getCurrentChunkName() != "sizeinfo")
        {
            std::ostringstream msg;
            msg << "Expected sizeinfo chunk, got '" << file.getCurrentChunkName() << "' chunk in file '" << pFilename << "'";
            throw LoadingError(msg.str());
        }

        size_t meshDataSize=0;

        uint32 numTextures=0;
        file.read(numTextures);
        if(loadWhat & Load::Materials)
        {
            meshDataSize += sizeof(const char*) * numTextures;
            memstat.texturePathReferencesMemory = MemoryPool(sizeof(const char*) * numTextures);
        }

        file.read(texturePathMemory);
        texturePathMemory += numTextures; // trailing \0 bytes
        texturePathMemory = (uint32)AlignmentHelper::__alignedSize(texturePathMemory);
        if(loadWhat & Load::Materials)
        {
            meshDataSize += texturePathMemory;
            memstat.texturePathMemory = MemoryPool(texturePathMemory);
        }

        if(file.getFileVersion() >= ModelFormatVersion::Version2)
        {
            file.read(materialNameMemory);
        }

        uint32 numBones = 0;
        uint32 numBoneInfos = 0;
        if (file.getFileVersion() >= ModelFormatVersion::Version3)
        {
            file.read(boneNameMemory);
            file.read(numBones);
            file.read(numBoneInfos);
        }

        uint32 numMaterials=0;
        file.read(numMaterials);
        if(loadWhat & Load::Materials)
        {
            memstat.materialData = MemoryPool(numMaterials * sizeof(MaterialData));
            meshDataSize += memstat.materialData.size;
        }

        if(file.getFileVersion() >= ModelFormatVersion::Version2)
        {
            materialNameMemory += numMaterials; /// trailing \0
            if(materialNameMemory % AlignmentHelper::__ALIGNMENT != 0)
                materialNameMemory += AlignmentHelper::__ALIGNMENT - (materialNameMemory % AlignmentHelper::__ALIGNMENT);
            if(loadWhat & Load::Materials)
            {
                meshDataSize += materialNameMemory;
                memstat.materialNameMemory = MemoryPool(materialNameMemory);
            }
        }

        if (file.getFileVersion() >= ModelFormatVersion::Version3)
        {
            boneNameMemory += numBones; /// because of the trailing \0

            if(boneNameMemory % AlignmentHelper::__ALIGNMENT != 0)
                boneNameMemory += AlignmentHelper::__ALIGNMENT - (boneNameMemory % AlignmentHelper::__ALIGNMENT);
            if(loadWhat & Load::Bones)
            {
                meshDataSize += boneNameMemory;
                memstat.boneNameMemory = MemoryPool(boneNameMemory);

                meshDataSize += sizeof(BoneData) * numBones;
                memstat.boneDataArray = MemoryPool(sizeof(BoneData) * numBones);

                meshDataSize += sizeof(BoneInfo) * numBoneInfos;
                memstat.boneDataArray.size += sizeof(BoneInfo) * numBoneInfos; // extend the existing pool
            }
        }

        uint32 numMeshes=0;
        file.read(numMeshes);
        if(loadWhat & Load::Meshes)
        {
            meshDataSize += sizeof(MeshData) * numMeshes;
            memstat.meshDataArray = MemoryPool(sizeof(MeshData) * numMeshes);
        }
        for(uint32 i=0; i<numMeshes; i++)
        {
            uint32 numVertices=0, PerVertexFlags=0, numComponents=0, numTexcoords=0;
            file.read(numVertices);
            file.read(PerVertexFlags);

            if(PerVertexFlags & PerVertexData::Position)
                numComponents++;
            if((PerVertexFlags & PerVertexData::Normal) && (loadWhat & Load::Normals))
                numComponents++;
            if((PerVertexFlags & PerVertexData::Tangent) && (loadWhat & Load::Tangents))
                numComponents++;
            if((PerVertexFlags & PerVertexData::Bitangent) && (loadWhat & Load::Bitangents))
                numComponents++;
            if(PerVertexFlags & PerVertexData::TexCoord0)
                numTexcoords++;
            if(PerVertexFlags & PerVertexData::TexCoord1)
                numTexcoords++;
            if(PerVertexFlags & PerVertexData::TexCoord2)
                numTexcoords++;
            if(PerVertexFlags & PerVertexData::TexCoord3)
                numTexcoords++;

            meshDataSize += allocationSize<float>(numVertices * 3) * numComponents;
            memstat.vertexData.size += allocationSize<float>(numVertices * 3) * numComponents;

            for(uint32 j=0; j<numTexcoords; j++)
            {
                uint8 numUVComponents;
                file.read(numUVComponents);
                if(loadWhat & loadTexCoords[j])
                {
                    meshDataSize += allocationSize<float>(numVertices * 3) * numUVComponents;
                    memstat.vertexData.size += allocationSize<float>(numVertices * 3) * numUVComponents;
                }
            }

            uint32 numFaces;
            file.read(numFaces);
            if(loadWhat & Load::Meshes)
            {
                meshDataSize += numFaces * sizeof(FaceData);
                memstat.faceDataArray.size += numFaces * sizeof(FaceData);
            }
        }

        uint32 numNodes = 0, numNodeReferences = 0, numMeshReferences = 0, numTextureReferences = 0;
        file.read(numNodes);
        file.read(numNodeReferences);
        file.read(nodeNameMemory);
        nodeNameMemory += numNodes; // trailing \0 bytes
        file.read(numMeshReferences);
        file.read(numTextureReferences);

        if(nodeNameMemory % AlignmentHelper::__ALIGNMENT != 0)
            nodeNameMemory += AlignmentHelper::__ALIGNMENT - (nodeNameMemory % AlignmentHelper::__ALIGNMENT);

        if(loadWhat & Load::Nodes)
        {
            meshDataSize += numNodes * sizeof(NodeData);
            meshDataSize += numNodes * sizeof(NodeDrawData);
            memstat.nodeData = MemoryPool(numNodes * sizeof(NodeData) + numNodes * sizeof(NodeDrawData));

            meshDataSize += allocationSize<uint32>(numMeshReferences);
            memstat.meshReferenceMemory = MemoryPool(allocationSize<uint32>(1) * numMeshReferences);

            meshDataSize += numNodeReferences * sizeof(NodeData*);
            memstat.nodeReferenceMemory = MemoryPool(numNodeReferences * sizeof(NodeDrawData*));

            meshDataSize += nodeNameMemory;
            memstat.nodeNameMemory = MemoryPool(nodeNameMemory);
        }
        if(loadWhat & Load::Materials)
        {
            meshDataSize += numTextureReferences * sizeof(TextureReference);
            memstat.textureReferenceMemory = MemoryPool(numTextureReferences * sizeof(TextureReference));
        }

        file.endReadChunk();

        m_pMeshDataAllocator = GEP_NEW(m_pAllocator, StackAllocator)(true, meshDataSize, m_pAllocator);
        m_pStartMarker = m_pMeshDataAllocator->getMarker();
    }

    // Load textures
    {
        file.startReadChunk();
        if(file.getCurrentChunkName() != "textures")
        {
            std::ostringstream msg;
            msg << "Expected 'textures' chunk but got '" << file.getCurrentChunkName() << "' chunk in file '" << pFilename << "'";
            throw LoadingError(msg.str());
        }
        if(loadWhat & Load::Materials)
        {
            uint32 numTextures = 0;
            file.read(numTextures);

            if(numTextures > 0)
            {
                memstat.texturePathReferencesMemory += allocationSize<const char*>(numTextures);
                m_modelData.textures = GEP_NEW_ARRAY(m_pMeshDataAllocator, const char*, numTextures);

                memstat.texturePathMemory += allocationSize<char>(texturePathMemory);
                char* textureNames = (char*)m_pMeshDataAllocator->allocateMemory(texturePathMemory);
                size_t curNamePos = 0;
                for(auto& texture : m_modelData.textures)
                {
                    uint32 len;
                    file.read(len);
                    file.readArray(ArrayPtr<char>(textureNames + curNamePos, len));
                    texture = textureNames + curNamePos;
                    curNamePos += len;
                    textureNames[curNamePos++] = '\0';
                }
            }

            file.endReadChunk();
        }
        else
        {
            file.skipCurrentChunk();
        }
    }

    // Read Materials
    {
        file.startReadChunk();
        if(file.getCurrentChunkName() != "materials")
        {
            std::ostringstream msg;
            msg << "Expected 'materials' chunk but got '" << file.getCurrentChunkName() << "' chunk in file '" << pFilename << "'";
            throw LoadingError(msg.str());
        }

        if(loadWhat & Load::Materials)
        {
            uint32 numMaterials = 0;
            file.read(numMaterials);

            if(numMaterials > 0)
            {
                memstat.materialData += allocationSize<MaterialData>(numMaterials);
                m_modelData.materials = GEP_NEW_ARRAY(m_pMeshDataAllocator, MaterialData, numMaterials);

                ArrayPtr<char> materialNames;
                size_t curNamePos = 0;
                if(file.getFileVersion() >= ModelFormatVersion::Version2)
                {
                    memstat.materialNameMemory += allocationSize<char>(materialNameMemory);
                    materialNames = GEP_NEW_ARRAY(m_pMeshDataAllocator, char, materialNameMemory);
                }

                for(auto& material : m_modelData.materials)
                {
                    file.startReadChunk();
                    if(file.getCurrentChunkName() != "mat")
                    {
                        std::ostringstream msg;
                        msg << "Expected 'mat' chunk but got '" << file.getCurrentChunkName() << "' chunk in file '" << pFilename << "'";
                        throw LoadingError(msg.str());
                    }

                    //read material name
                    if(file.getFileVersion() >= ModelFormatVersion::Version2)
                    {
                        uint32 len = 0;
                        file.read(len);
                        auto data = materialNames(curNamePos, curNamePos+len);
                        curNamePos += len;
                        file.readArray(data);
                        materialNames[curNamePos++] = '\0';
                        material.name = data.getPtr();
                    }

                    uint32 numTextures = 0;
                    file.read(numTextures);
                    memstat.textureReferenceMemory += allocationSize<TextureReference>(numTextures);
                    material.textures = GEP_NEW_ARRAY(m_pMeshDataAllocator, TextureReference, numTextures);
                    for(auto& texture : material.textures)
                    {
                        uint32 textureIndex;
                        file.read(textureIndex);
                        texture.file = m_modelData.textures[textureIndex];

                        uint8 semantic = (uint8)TextureType::UNKNOWN;
                        file.read<uint8>(semantic);
                        texture.semantic = (TextureType)semantic;
                    }

                    file.endReadChunk();
                }
            }

            file.endReadChunk();
        }
        else
        {
            file.skipCurrentChunk();
        }
    }


    // Read Meshes
    {
        file.startReadChunk();
        if(file.getCurrentChunkName() != "meshes")
        {
            std::ostringstream msg;
            msg << "Expected 'meshes' chunk but got '" << file.getCurrentChunkName() << "' chunk in file '" << pFilename << "'";
            throw LoadingError(msg.str());
        }

        if(loadWhat & Load::Meshes)
        {
            uint32 numMeshes;
            file.read(numMeshes);
            memstat.meshDataArray += allocationSize<MeshData>(numMeshes);
            m_modelData.meshes = GEP_NEW_ARRAY(m_pMeshDataAllocator, MeshData, numMeshes);

            for(auto& mesh : m_modelData.meshes)
            {
                file.startReadChunk();
                if(file.getCurrentChunkName() != "mesh")
                {
                    std::ostringstream msg;
                    msg << "Expected 'mesh' chunk but got '" << file.getCurrentChunkName() << "' chunk in file '" << pFilename << "'";
                }

                uint32 materialIndex = 0;
                file.read(mesh.materialIndex);

                vec3 minBounds, maxBounds;
                file.readArray<float>(minBounds.data);
                file.readArray<float>(maxBounds.data);
                maxBounds += vec3(0.01f, 0.01f, 0.01f);
                mesh.bbox = AABB(minBounds, maxBounds);

                uint32 numVertices = 0;
                file.read(numVertices);

                file.startReadChunk();
                if(file.getCurrentChunkName() != "vertices")
                {
                    std::ostringstream msg;
                    msg << "Expected 'vertices' chunk but got '" << file.getCurrentChunkName() << "' chunk in file '" << pFilename << "'";
                    throw LoadingError(msg.str());
                }
                memstat.vertexData += allocationSize<vec3>(numVertices);
                mesh.vertices = GEP_NEW_ARRAY(m_pMeshDataAllocator, vec3, numVertices);

                static_assert(sizeof(vec3) == 3 * sizeof(float), "The following read call assumes that a vec3 is 3 floats big");
                file.readArray(ArrayPtr<float>(mesh.vertices[0].data, numVertices * 3));
                file.endReadChunk();

                {
                    file.startReadChunk();
                    if(file.getCurrentChunkName() == "normals")
                    {
                        if(loadWhat & Load::Normals)
                        {
                            memstat.vertexData += allocationSize<vec3>(numVertices);
                            mesh.normals = GEP_NEW_ARRAY(m_pMeshDataAllocator, vec3, numVertices);
                            for(auto& normal : mesh.normals)
                            {
                                normal.x = readCompressedFloat(file);
                                normal.y = readCompressedFloat(file);
                                normal.z = readCompressedFloat(file);
                            }
                            file.endReadChunk();
                        }
                        else
                        {
                            file.skipCurrentChunk();
                        }
                        file.startReadChunk();
                    }
                    if(file.getCurrentChunkName() == "tangents")
                    {
                        if(loadWhat & Load::Tangents)
                        {
                            memstat.vertexData += allocationSize<vec3>(numVertices);
                            mesh.tangents = GEP_NEW_ARRAY(m_pMeshDataAllocator, vec3, numVertices);
                            for(auto& tangent : mesh.tangents)
                            {
                                tangent.x = readCompressedFloat(file);
                                tangent.y = readCompressedFloat(file);
                                tangent.z = readCompressedFloat(file);
                            }
                            file.endReadChunk();
                        }
                        else
                        {
                            file.skipCurrentChunk();
                        }
                        file.startReadChunk();
                    }
                    if(file.getCurrentChunkName() == "bitangents")
                    {
                        if(loadWhat & Load::Bitangents)
                        {
                            memstat.vertexData += allocationSize<vec3>(numVertices);
                            mesh.bitangents = GEP_NEW_ARRAY(m_pMeshDataAllocator, vec3, numVertices);
                            for(auto& bitangent : mesh.bitangents)
                            {
                                bitangent.x = readCompressedFloat(file);
                                bitangent.y = readCompressedFloat(file);
                                bitangent.z = readCompressedFloat(file);
                            }
                            file.endReadChunk();
                        }
                        else
                        {
                            file.skipCurrentChunk();
                        }
                        file.startReadChunk();
                    }
                    if(file.getCurrentChunkName() == "texcoords")
                    {
                        if((loadWhat & Load::TexCoords0) || (loadWhat & Load::TexCoords1) ||
                            (loadWhat & Load::TexCoords2) || (loadWhat & Load::TexCoords3))
                        {
                            uint8 numTexCoords;
                            file.read(numTexCoords);
                            for(uint8 i=0; i<numTexCoords; i++)
                            {
                                uint8 numUVComponents;
                                file.read(numUVComponents);
                                if(numUVComponents != 2)
                                {
                                    std::ostringstream msg;
                                    msg << "Currently only 2 component texture coordinates are supported got " << numUVComponents << " components";
                                    throw LoadingError(msg.str());
                                }
                                if(loadWhat & loadTexCoords[i])
                                {
                                    memstat.vertexData += allocationSize<vec2>(numVertices);
                                    mesh.texcoords[i] = GEP_NEW_ARRAY(m_pMeshDataAllocator, vec2, numVertices);
                                    static_assert(sizeof(vec2) == 2 * sizeof(float), "the following read call assumes that a vec2 is twice the size of a float");
                                    file.readArray(ArrayPtr<float>((float*)mesh.texcoords[i].getPtr(), numVertices * numUVComponents));
                                }
                                else
                                {
                                    //skip the texcoord data
                                    file.skipRead(sizeof(float) * numUVComponents * numVertices);
                                }
                            }
                            file.endReadChunk();
                        }
                        else
                        {
                            file.skipCurrentChunk();
                        }
                        file.startReadChunk();
                    }
                    if (file.getFileVersion() >= ModelFormatVersion::Version3 && file.getCurrentChunkName() == "bones")
                    {
                        if (loadWhat & Load::Bones)
                        {
                            // read bone data
                            {
                                uint32 numBones = 0;
                                file.read(numBones);

                                memstat.boneDataArray += allocationSize<BoneData>(numBones);
                                mesh.bones = GEP_NEW_ARRAY(m_pMeshDataAllocator, BoneData, numBones);

                                memstat.boneNameMemory += allocationSize<char>(boneNameMemory);
                                auto boneNames = GEP_NEW_ARRAY(m_pMeshDataAllocator, char, boneNameMemory);

                                size_t currentNamePos = 0;
                                for (auto& bone : mesh.bones)
                                {
                                    // Get name
                                    uint32 nameLen = 0;
                                    file.read(nameLen);
                                    auto data = boneNames(currentNamePos, currentNamePos + nameLen);
                                    currentNamePos += nameLen;
                                    file.readArray(data);
                                    boneNames[currentNamePos++] = '\0';
                                    bone.name = data.getPtr();

                                    // Get the offset matrix
                                    file.read(bone.offsetMatrix);
                                }
                            }
                            // Read bone infos
                            {
                                uint32 numBoneInfos = 0;
                                file.read(numBoneInfos);

                                if (numVertices != numBoneInfos)
                                {
                                    throw LoadingError("Wrong number of bone infos! There must be exactly as many bone infos as there are vertices.");
                                }
                            
                                memstat.boneDataArray += allocationSize<BoneInfo>(numBoneInfos);
                                mesh.boneInfos = GEP_NEW_ARRAY(m_pMeshDataAllocator, BoneInfo, numBoneInfos);

                                static_assert(sizeof(BoneInfo) == sizeof(uint32) * 4 + sizeof(float) * 4,
                                    "The size of the internal BoneInfo struct has changed."
                                    "You should check that the code here is still correct.");

                                // Needed because the BoneInfo used by the engine is not the same size as the BoneInfo in the file.
                                struct BoneInfoInFile
                                {
                                    uint16 boneIndices[4];
                                    float weights[4];
                                };

                                static_assert(sizeof(BoneInfoInFile) < sizeof(BoneInfo), "You should also check that the allocation thing below is correct.");

                                auto marker = m_pMeshDataAllocator->getMarker();
                                SCOPE_EXIT{ m_pMeshDataAllocator->freeToMarker(marker); });

                                #pragma message ("warning: Probably allocating too much memory here! In some cases, this could fail.")
                                auto boneInfosInFile = GEP_NEW_ARRAY(m_pMeshDataAllocator, BoneInfoInFile, numBoneInfos);

                                file.readArray(boneInfosInFile);

                                for (size_t i = 0; i < numBoneInfos; i++)
                                {
                                    auto& boneInfoInFile = boneInfosInFile[i];
                                    auto& boneInfo = mesh.boneInfos[i];
                                    for (int j = 0; j < BoneInfo::NUM_SUPPORTED_BONES; j++)
                                    {
                                        boneInfo.boneIndices[j] = uint32(boneInfoInFile.boneIndices[j]);
                                        boneInfo.weights[j] = float(boneInfoInFile.weights[j]);
                                    }
                                }
                            }

                            file.endReadChunk();
                        }
                        else
                        {
                            file.skipCurrentChunk();
                        }
                        file.startReadChunk();
                    }
                    if(file.getCurrentChunkName() == "faces")
                    {
                        uint32 numFaces = 0;
                        file.read(numFaces);
                        memstat.faceDataArray += allocationSize<FaceData>(numFaces);
                        mesh.faces = GEP_NEW_ARRAY(m_pMeshDataAllocator, FaceData, numFaces);
                        GEP_ASSERT(mesh.faces.getPtr() != nullptr);
                        if(numVertices > std::numeric_limits<uint16>::max())
                        {
                            static_assert(sizeof(FaceData) == sizeof(uint32) * 3, "the following read call assumes that FaceData is 3 uint32s wide");
                            file.readArray(ArrayPtr<uint32>((uint32*)mesh.faces.getPtr(), numFaces*3));
                        }
                        else
                        {
                            uint16 data = 0;
                            for(auto& face : mesh.faces)
                            {
                                file.read(data); face.indices[0] = (uint32)data;
                                file.read(data); face.indices[1] = (uint32)data;
                                file.read(data); face.indices[2] = (uint32)data;
                            }
                        }
                    }
                    else
                    {
                        std::ostringstream msg;
                        msg << "Unexepcted chunk '" << file.getCurrentChunkName() << "' in file '" << pFilename << "'";
                        throw LoadingError(msg.str());
                    }
                    file.endReadChunk();
                }

                file.endReadChunk();
            }

            file.endReadChunk();
        }
        else
        {
            file.skipCurrentChunk();
        }
    }

    // Read Nodes
    {
        file.startReadChunk();
        if(loadWhat & Load::Nodes)
        {
            if(file.getCurrentChunkName() != "nodes")
            {
                std::ostringstream msg;
                msg << "Expected 'nodes' chunk but got '" << file.getCurrentChunkName() << "' in file '" << pFilename << "'";
                throw LoadingError(msg.str());
            }
            uint32 numNodes;
            file.read(numNodes);

            memstat.nodeNameMemory += allocationSize<char>(nodeNameMemory);
            auto nodeNames = GEP_NEW_ARRAY(m_pMeshDataAllocator, char, nodeNameMemory);

            size_t curNodeNamePos = 0;

            memstat.nodeData += allocationSize<NodeData>(numNodes);
            auto nodesData = GEP_NEW_ARRAY(m_pMeshDataAllocator, NodeData, numNodes);

            memstat.nodeData += allocationSize<NodeDrawData>(numNodes);
            auto nodes = GEP_NEW_ARRAY(m_pMeshDataAllocator, NodeDrawData, numNodes);

            size_t i = 0;
            for(auto& node : nodes)
            {
                node.data = &nodesData[i];
                uint32 nameLength = 0;
                file.read(nameLength);
                auto name = nodeNames(curNodeNamePos, curNodeNamePos + nameLength);
                file.readArray(name);
                curNodeNamePos += nameLength;
                nodeNames[curNodeNamePos++] = '\0';
                node.data->name = name.getPtr();

                file.readArray<float>(node.transform.data);
                uint32 nodeParentIndex = 0;
                file.read(nodeParentIndex);
                if(nodeParentIndex == std::numeric_limits<uint32>::max())
                    node.data->parent = nullptr;
                else
                    node.data->parent = &nodes[nodeParentIndex];

                node.meshes = file.readAndAllocateArray<uint32, uint32>(m_pMeshDataAllocator);
                memstat.meshReferenceMemory += allocationSize<uint32>((uint32)(node.meshes.length()));
                uint32 numChildren = 0;
                file.read(numChildren);
                if(numChildren > 0)
                {
                    memstat.nodeReferenceMemory += allocationSize<NodeDrawData*>(numChildren);
                    node.children = GEP_NEW_ARRAY(m_pMeshDataAllocator, NodeDrawData*, numChildren);
                    for(auto& child : node.children)
                    {
                        uint32 childIndex = 0;
                        file.read(childIndex);
                        child = &nodes[childIndex];
                    }
                }

                i++;
            }
            m_modelData.rootNode = &nodes[0];

            file.endReadChunk();
        }
        else
        {
            file.skipCurrentChunk();
        }
    }

    file.endReading();
    m_modelData.hasData = true;
}
void gep::ModelLoader::loadFromData(SmartPtr<ReferenceCounted> pDataHolder, ArrayPtr<vec4> vertices, ArrayPtr<uint32> indices)
{
    m_filename = "<from data>";
    uint32 meshDataSize = 0;
    meshDataSize += allocationSize<MaterialData>(1);
    meshDataSize += allocationSize<MeshData>(1);
    meshDataSize += allocationSize<FaceData>(indices.length() / 3);
    meshDataSize += allocationSize<float>(vertices.length() * 3);
    meshDataSize += allocationSize<NodeData>(1);
    meshDataSize += allocationSize<NodeDrawData>(1);
    meshDataSize += allocationSize<uint32>(1);
    meshDataSize += allocationSize<MeshData*>(1);

    m_pMeshDataAllocator = GEP_NEW(m_pAllocator, StackAllocator)(true, meshDataSize, m_pAllocator);
    m_pStartMarker = m_pMeshDataAllocator->getMarker();

    m_modelData.rootNode = GEP_NEW(m_pMeshDataAllocator, NodeDrawData)();
    m_modelData.rootNode->meshes = GEP_NEW_ARRAY(m_pMeshDataAllocator, uint32, 1);
    m_modelData.rootNode->meshes[0] = 0;
    m_modelData.rootNode->transform = mat4::identity().right2Left();
    m_modelData.rootNode->data = GEP_NEW(m_pMeshDataAllocator, NodeData);
    m_modelData.rootNode->data->name = "root node";
    m_modelData.rootNode->data->parent = nullptr;
    auto mesh = GEP_NEW(m_pMeshDataAllocator, MeshData)();
    m_modelData.meshes = ArrayPtr<MeshData>(mesh, 1);
    m_modelData.rootNode->data->meshData = GEP_NEW_ARRAY(m_pMeshDataAllocator, MeshData*, 1);
    m_modelData.rootNode->data->meshData[0] = mesh;
    
    vec3 vmin(std::numeric_limits<float>::max());
    vec3 vmax(std::numeric_limits<float>::lowest());
    for(auto& v : vertices)
    {
        if(v.x < vmin.x) vmin.x = v.x;
        if(v.y < vmin.y) vmin.y = v.y;
        if(v.z < vmin.z) vmin.z = v.z;
        if(v.x > vmax.x) vmax.x = v.x;
        if(v.y > vmax.y) vmax.y = v.y;
        if(v.z > vmax.z) vmax.z = v.z;
    }
    mesh->bbox = AABB(vmin, vmax);
    mesh->faces = GEP_NEW_ARRAY(m_pMeshDataAllocator, FaceData, indices.length() / 3);

    size_t i=0;
    for(auto& face : mesh->faces)
    {
        face.indices[0] = indices[i];
        face.indices[1] = indices[i + 1];
        face.indices[2] = indices[i + 2];
        i += 3;
    }

    mesh->materialIndex = 0;
    mesh->numFaces = indices.length() / 3;
    mesh->vertices = GEP_NEW_ARRAY(m_pMeshDataAllocator, vec3, vertices.length());
    
    i=0;
    for(auto& v : mesh->vertices)
    {
        v = vec3(vertices[i].x, vertices[i].y, vertices[i].z);
        i++;
    }

    m_modelData.materials = GEP_NEW_ARRAY(m_pMeshDataAllocator, MaterialData, 1);
    m_modelData.materials[0].name = "dummy material";
    m_modelData.hasData = true;
    
}
